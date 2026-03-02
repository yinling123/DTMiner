#ifndef SEGMENTED_QUEUE_H
#define SEGMENTED_QUEUE_H

#include <sys/mman.h>
#include <unistd.h>
#include <atomic>
#include <vector>
#include <mutex>
#include <stdexcept>
#include <xmmintrin.h>
#include "MemoryPool.h"
#include <chrono>

// 【修复】定义分支预测宏，兼容旧版本编译器 (C++17及以下)
#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

/**
 * 这个是每个target chunk一个队列
 */
template <typename T>
class LockFreeQueue
{
    // 2^19 = 524288
    static constexpr size_t SEGMENT_POWER = 19;
    static constexpr size_t SEGMENT_SIZE = 1 << SEGMENT_POWER;
    static constexpr size_t SEGMENT_MASK = SEGMENT_SIZE - 1;

    static constexpr int BATCH_SIZE = 128;

    /**
     * 每个队列最多59GB内存
     */
    static constexpr size_t expected_max_items = 2000000000;

public:
    std::atomic<size_t> tail;
    int max_segs;
    std::vector<std::atomic<T *>> segments;
    BlockPool<T> *pool;
    bool own_pool;
    static std::atomic<long long> total_times;

    // 解决伪共享 (False Sharing)
    struct alignas(64) ThreadBuffer
    {
        T buffer[BATCH_SIZE];
        std::atomic<int> count = 0;
    };
    ThreadBuffer thread_buffers[32];

    T *get_or_create_segment(size_t idx)
    {
        T *seg = segments[idx].load(std::memory_order_acquire);
        if (seg && seg != reinterpret_cast<T *>(-1))
            return seg;

        T *expected = nullptr;
        if (segments[idx].compare_exchange_strong(expected, (T *)-1,
                                                  std::memory_order_acq_rel,
                                                  std::memory_order_acquire))
        {
            // 第一个线程负责初始化
            T *newSeg = static_cast<T *>(pool->alloc());
            segments[idx].store(newSeg, std::memory_order_release);
            return newSeg;
        }

        // 等待其他线程完成创建
        while (true)
        {
            seg = segments[idx].load(std::memory_order_acquire);
            if (seg && seg != reinterpret_cast<T *>(-1))
                return seg;
            _mm_pause();
        }
    }

public:
    LockFreeQueue(BlockPool<T> *external_pool)
        : tail(0),
          max_segs((expected_max_items + SEGMENT_SIZE - 1) / SEGMENT_SIZE),
          segments(max_segs),
          pool(external_pool),
          own_pool(external_pool == nullptr)
    {
        for (size_t i = 0; i < 16 && i < max_segs; i++)
        {
            T *seg = static_cast<T *>(pool->alloc());
            segments[i].store(seg, std::memory_order_release);
        }
    }

    LockFreeQueue()
        : max_segs((expected_max_items + SEGMENT_SIZE - 1) / SEGMENT_SIZE),
          segments(max_segs)
    {
    }

    void init(BlockPool<T> *external_pool = nullptr)
    {
        tail = 0;
        pool = external_pool;
        own_pool = external_pool == nullptr;
        for (size_t i = 0; i < 4 && i < max_segs; i++)
        {
            T *seg = static_cast<T *>(pool->alloc());
            segments[i].store(seg, std::memory_order_release);
        }
        // 重置所有线程的 buffer
        for (int i = 0; i < 32; ++i)
            thread_buffers[i].count = 0;
    }

    void destroy_temp()
    {
        std::vector<std::atomic<T *>> temp;
        temp.swap(segments);
    }

    void clear_queue()
    {
        tail.store(0);
    }

    ~LockFreeQueue()
    {
        if (own_pool)
        {
            delete pool;
        }
    }

    // inline 减少调用开销
    inline void flush_batch_logic(ThreadBuffer &tb)
    {
        int count = tb.count.load(std::memory_order_acquire);
        if (count == 0)
            return;

        size_t start_pos = tail.fetch_add(count, std::memory_order_relaxed);

        size_t idx = start_pos >> SEGMENT_POWER;
        size_t off = start_pos & SEGMENT_MASK;

        size_t end_pos = start_pos + count;
        size_t end_idx = (end_pos - 1) >> SEGMENT_POWER;

        // 【修复】使用 LIKELY 宏替代 C++20 属性
        if (LIKELY(idx == end_idx))
        {
            T *seg = get_or_create_segment(idx);
            for (int i = 0; i < count; ++i)
            {
                new (&seg[off + i]) T(std::move(tb.buffer[i]));
            }
        }
        else
        {
            T *seg1 = get_or_create_segment(idx);
            size_t first_part = SEGMENT_SIZE - off;
            for (size_t i = 0; i < first_part; ++i)
            {
                new (&seg1[off + i]) T(std::move(tb.buffer[i]));
            }

            T *seg2 = get_or_create_segment(idx + 1);
            for (size_t i = 0; i < count - first_part; ++i)
            {
                new (&seg2[i]) T(std::move(tb.buffer[first_part + i]));
            }
        }
        tb.count.store(0, std::memory_order_relaxed);
    }

    void enqueue(T &&item, int threadIdx)
    {
        ThreadBuffer &tb = thread_buffers[threadIdx];

        // 1. 获取当前下标 (Relaxed即可，因为只有当前线程写)
        int cnt = tb.count.load(std::memory_order_relaxed);

        // 2. 写入数据 (此时数据可能还在CPU缓存/Store Buffer中，未同步到内存)
        tb.buffer[cnt] = std::move(item);

        // 3. 【关键】Release Store
        // 语义：我把 count 改为 cnt+1。
        // 保证：在这个操作之前的所有内存写入（即上面的 buffer 写入），
        //       对于任何能看到这个新 count 的线程来说，都是可见的。
        tb.count.store(cnt + 1, std::memory_order_release);

        // 检查是否已满
        if (UNLIKELY(cnt + 1 >= BATCH_SIZE))
        {
            flush_batch_logic(tb);
        }
    }

    void flush_buffers()
    {
        for (int threadIdx = 0; threadIdx < 32; ++threadIdx)
        {
            flush_batch_logic(thread_buffers[threadIdx]);
        }
    }

    T &at(size_t i) const
    {
        size_t idx = i >> SEGMENT_POWER;
        size_t off = i & SEGMENT_MASK;
        T *seg = segments[idx].load(std::memory_order_acquire);

        return seg[off];
    }

    size_t size() const
    {
        return tail.load(std::memory_order_acquire);
    }
};

#endif // SEGMENTED_QUEUE_H