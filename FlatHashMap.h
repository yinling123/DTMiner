#ifndef FlatHashMap_h
#define FlatHashMap_h

#include <cstdint>
#include <cstring>
#include <vector>
#include <algorithm>
#include <utility>
#include "SparseIntMap.h"

class FlatHashMap
{
public:
    using K = std::pair<int, int>; // <vertex, block_id>
    using V = std::pair<int, int>; // <start_index, end_index>

    // 仅在构建阶段使用的临时结构
    struct TempEntry
    {
        int block_id;
        V value;
        // 排序只看 block_id
        bool operator<(const TempEntry &other) const { return block_id < other.block_id; }
    };

    using InternalBlockMap = SparseIntMap<int, V>;

    FlatHashMap(size_t cap = 16) : size(0), is_finalized(false)
    {
        internal_block_maps.resize(cap);
        temp_build_buffer.resize(cap);
    }

    ~FlatHashMap() {}

    void insert(const K &key, const V &val)
    {
        int vertex_id = key.first;
        int block_id = key.second;

        // 动态扩容
        if (vertex_id >= internal_block_maps.size())
        {
            size_t new_size = std::max((size_t)vertex_id + 1, internal_block_maps.size() * 2);
            internal_block_maps.resize(new_size);
            temp_build_buffer.resize(new_size);
        }

        // 1. 存入哈希表 (用于精确查找 Fast Path)
        internal_block_maps[vertex_id][block_id] = val;

        // 2. 存入临时 Buffer (用于构建 CSR Fallback Path)
        temp_build_buffer[vertex_id].push_back({block_id, val});

        ++size;
    }

    // 查询逻辑：混合查找 (Map + SoA CSR)
    bool get(const K &key, V &out) const
    {
        int vertex_id = key.first;
        int block_id = key.second;

        // 越界检查
        if (vertex_id >= internal_block_maps.size())
            return false;

        // ============================================================
        // Path 1: 精确查找 (哈希表 - 最快)
        // ============================================================
        const auto &block_map = internal_block_maps[vertex_id];
        auto it_map = block_map.find(block_id);
        if (it_map != block_map.end())
        {
            out = (*it_map).value;
            return true;
        }

        // ============================================================
        // Path 2: 范围查找 (SoA CSR - 二分查找)
        // ============================================================
        // if (!is_finalized)
        //     return false;

        uint32_t start_idx = csr_row_ptr[vertex_id];
        uint32_t end_idx = csr_row_ptr[vertex_id + 1];

        // 该顶点无数据，直接返回
        if (start_idx == end_idx)
            return false;

        // 【优化】: 利用有序数组性质进行 O(1) 剪枝，替代 vertex_bounds 数组
        // 直接读取该行最后一个元素的 block_id (即最大值)
        int max_block_id = csr_keys[end_idx - 1];
        if (block_id > max_block_id)
            return false;

        // 【优化】: 软件预取 CSR Key 数组
        // 告诉 CPU 提前加载二分查找所需的内存页，减少流水线停顿
        __builtin_prefetch(&csr_keys[start_idx], 0, 1);

        // 【优化】: 在 SoA 布局的 Key 数组上二分
        // 相比 AoS，Cache Line 能容纳更多 Key，减少 Cache Miss
        const int *base_ptr = csr_keys.data();
        const int *it = std::lower_bound(base_ptr + start_idx, base_ptr + end_idx, block_id);

        // 如果找到了有效的下一个块
        if (it != base_ptr + end_idx)
        {
            // 计算偏移量，去 Values 数组取值
            uint32_t offset = std::distance(base_ptr, it);
            out = csr_values[offset];
            return true;
        }

        return false;
    }

    size_t getSize() const { return size; }

    // 单线程构建 CSR (SoA 布局)
    void finalize_preprocessing()
    {
        if (temp_build_buffer.empty())
            return;

        size_t num_vertices = temp_build_buffer.size();
        csr_row_ptr.assign(num_vertices + 1, 0);

        // 1. 排序并计算总数
        size_t total_entries = 0;
        for (size_t i = 0; i < num_vertices; ++i)
        {
            csr_row_ptr[i] = total_entries;

            if (!temp_build_buffer[i].empty())
            {
                // 必须排序以支持二分查找
                std::sort(temp_build_buffer[i].begin(), temp_build_buffer[i].end());
                total_entries += temp_build_buffer[i].size();
            }
        }
        csr_row_ptr[num_vertices] = total_entries; // 哨兵

        // 2. 分配紧凑内存 (SoA: Key 和 Value 分离)
        csr_keys.resize(total_entries);
        csr_values.resize(total_entries);

        // 3. 填充数据 (将 struct 拆分到两个数组)
        size_t current_idx = 0;
        for (size_t i = 0; i < num_vertices; ++i)
        {
            const auto &buffer = temp_build_buffer[i];
            size_t count = buffer.size();

            if (count > 0)
            {
                for (size_t j = 0; j < count; ++j)
                {
                    csr_keys[current_idx + j] = buffer[j].block_id;
                    csr_values[current_idx + j] = buffer[j].value;
                }
                current_idx += count;

                // 及时释放临时内存
                std::vector<TempEntry>().swap(temp_build_buffer[i]);
            }
        }

        // 4. 彻底释放构建缓冲区
        temp_build_buffer.clear();
        temp_build_buffer.shrink_to_fit();

        is_finalized = true;
    }

private:
    // 哈希表数组 (Fast Path)
    std::vector<InternalBlockMap> internal_block_maps;

    // CSR 结构 (Fallback Path, SoA 布局)
    std::vector<uint32_t> csr_row_ptr;
    std::vector<int> csr_keys; // 只存 Block ID，极大利于二分
    std::vector<V> csr_values; // 存 Range

    // 构建缓冲区 (Temp)
    std::vector<std::vector<TempEntry>> temp_build_buffer;

    size_t size;
    bool is_finalized;
};

#endif