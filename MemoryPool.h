#ifndef BLOCK_POOL_H
#define BLOCK_POOL_H

#include <cstddef>
#include <vector>
#include <atomic>
#include <stdexcept>
#include <new>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <sys/mman.h>
#include <errno.h>
#include <numa.h>
#include <numaif.h> // 必须包含: 提供 mbind 和 MPOL_PREFERRED

template <typename T>
class BlockPool
{
    // 【关键修复 1】必须 >= LockFreeQueue 的 Segment Size (2^19 = 524288)
    // 之前 Crash 就是因为这里小了，导致队列写到了分配的内存块之外
    static constexpr size_t SEGMENT_POWER = 19;
    static constexpr size_t BLOCK_SIZE = 1 << SEGMENT_POWER;

    static constexpr size_t ALIGNMENT = 64;

    struct BlockHeader
    {
        BlockHeader *next;
        void *raw_memory;
        size_t alloc_size;
    };

    std::atomic<BlockHeader *> free_list;
    std::vector<BlockHeader *> allocated_blocks;
    int numa_node;

    static size_t calc_data_offset()
    {
        size_t base = sizeof(BlockHeader);
        return (base + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }

    void allocate_block()
    {
        size_t data_offset = calc_data_offset();
        size_t data_size = BLOCK_SIZE * sizeof(T);
        size_t total_size = data_offset + data_size;

        // 1. 标准 mmap，只分配虚拟地址，不立即消耗物理内存 (Lazy Allocation)
        void *raw_mem = mmap(nullptr, total_size,
                             PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS,
                             -1, 0);

        if (raw_mem == MAP_FAILED)
        {
            std::cerr << "BlockPool: mmap failed: " << strerror(errno) << std::endl;
            throw std::bad_alloc();
        }

        // 2. 【关键修复 2】使用 MPOL_PREFERRED (软亲和)
        // 告诉内核：尽量用 numa_node 的内存，如果没了，就去别的节点借，不要崩。
        if (numa_node >= 0)
        {
            unsigned long nodemask = (1UL << numa_node);

            long ret = mbind(raw_mem, total_size,
                             MPOL_PREFERRED, // <--- 软限制，安全
                             &nodemask, sizeof(nodemask) * 8,
                             0);

            // if (ret < 0) { std::cerr << "mbind warning" << std::endl; }
        }

        auto *header = reinterpret_cast<BlockHeader *>(raw_mem);
        header->raw_memory = raw_mem;
        header->alloc_size = total_size;

        // 将新块加入 free_list
        free(block_data(header));
        allocated_blocks.push_back(header);
    }

    T *block_data(BlockHeader *blk)
    {
        size_t offset = calc_data_offset();
        return reinterpret_cast<T *>(reinterpret_cast<char *>(blk) + offset);
    }

    BlockHeader *data_block(T *ptr)
    {
        size_t offset = calc_data_offset();
        return reinterpret_cast<BlockHeader *>(reinterpret_cast<char *>(ptr) - offset);
    }

public:
    // 【优化】减少默认预分配数量到 8，避免启动时虚拟内存占用过大
    explicit BlockPool(int preallocate_blocks = 8, int numa_node_ = -1)
        : free_list(nullptr), numa_node(numa_node_)
    {
        if (numa_available() < 0)
        {
            this->numa_node = -1;
        }
        else if (numa_node >= 0 && numa_node > numa_max_node())
        {
            throw std::runtime_error("Invalid NUMA node index");
        }

        for (int i = 0; i < preallocate_blocks; ++i)
        {
            allocate_block();
        }
    }

    ~BlockPool()
    {
        for (BlockHeader *header : allocated_blocks)
        {
            munmap(header->raw_memory, header->alloc_size);
        }
    }

    T *alloc()
    {
        BlockHeader *old_head = free_list.load(std::memory_order_relaxed);
        while (true)
        {
            if (old_head == nullptr)
            {
                // 只有当内存不够时才分配新块 (Lazy)
                allocate_block();
                old_head = free_list.load(std::memory_order_relaxed);
                if (old_head == nullptr)
                    throw std::bad_alloc();
                continue;
            }
            BlockHeader *new_head = old_head->next;
            if (free_list.compare_exchange_weak(old_head, new_head,
                                                std::memory_order_acquire,
                                                std::memory_order_relaxed))
            {
                return block_data(old_head);
            }
        }
    }

    void free(T *ptr)
    {
        if (!ptr)
            return;
        BlockHeader *blk = data_block(ptr);
        BlockHeader *old_head = free_list.load(std::memory_order_relaxed);
        do
        {
            blk->next = old_head;
        } while (!free_list.compare_exchange_weak(old_head, blk,
                                                  std::memory_order_release,
                                                  std::memory_order_relaxed));
    }

    void add_segments(std::vector<std::atomic<T *>> &segments)
    {
        for (auto &atomic_ptr : segments)
        {
            T *seg = atomic_ptr.exchange(nullptr, std::memory_order_acquire);
            if (seg && seg != reinterpret_cast<T *>(-1))
            {
                free(seg);
            }
        }
    }

    int free_block_count() const { return 0; }
    int total_chunks() const { return allocated_blocks.size(); }
};

#endif // BLOCK_POOL_H