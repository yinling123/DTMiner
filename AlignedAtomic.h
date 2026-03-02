#ifndef ALIGNED_ATOMIC_H
#define ALIGNED_ATOMIC_H

#include <atomic>

template <typename T>
struct alignas(64) AlignedAtomic
{
    std::atomic<T> val;
    char padding[64 - sizeof(std::atomic<T>)]; // 填充至64字节

    // 1. 构造函数
    AlignedAtomic(T v = T()) : val(v) {}

    // 禁止拷贝 (原子变量不能拷贝)
    AlignedAtomic(const AlignedAtomic &) = delete;
    AlignedAtomic &operator=(const AlignedAtomic &) = delete;

    // ============================================================
    // 核心魔法 1：隐式转换 (读)
    // 允许: int x = s[i];  或者  if (s[i] == 10)
    // ============================================================
    operator T() const
    {
        return val.load(std::memory_order_seq_cst);
    }

    // ============================================================
    // 核心魔法 2：赋值重载 (写)
    // 允许: s[i] = 10;
    // ============================================================
    T operator=(T v)
    {
        val.store(v, std::memory_order_seq_cst);
        return v;
    }

    // ============================================================
    // 核心魔法 3：算术运算重载 (针对 ++, --, +=, -=)
    // 允许: s[i]++;  或者  s[i] += 5;
    // ============================================================
    T operator++() { return val.fetch_add(1) + 1; } // ++Prefix
    T operator++(int) { return val.fetch_add(1); }  // Postfix++
    T operator--() { return val.fetch_sub(1) - 1; } // --Prefix
    T operator--(int) { return val.fetch_sub(1); }  // Postfix--
    T operator+=(T v) { return val.fetch_add(v) + v; }
    T operator-=(T v) { return val.fetch_sub(v) - v; }

    // ------------------------------------------------------------
    // 保留标准 atomic 函数 (以备不时之需，比如需要指定 memory_order 时)
    // ------------------------------------------------------------
    T load(std::memory_order order = std::memory_order_seq_cst) const { return val.load(order); }
    void store(T v, std::memory_order order = std::memory_order_seq_cst) { val.store(v, order); }
    T fetch_add(T v, std::memory_order order = std::memory_order_seq_cst) { return val.fetch_add(v, order); }
    bool compare_exchange_strong(T &expected, T desired, std::memory_order s, std::memory_order f) { return val.compare_exchange_strong(expected, desired, s, f); }
    // 简化版 CAS
    bool compare_exchange_strong(T &expected, T desired) { return val.compare_exchange_strong(expected, desired); }
};

#endif