#ifndef SparseIntMap_h
#define SparseIntMap_h

#include <utility>
#include <cstddef>
#include <limits>
#include <cstring>
#include <algorithm>
#include <cstdint>

template <typename Key = int, typename Value = std::pair<int, int>>
class SparseIntMap
{
public:
    struct Entry
    {
        Key key;
        Value value;
    };

    // 迭代器定义
    class iterator
    {
    public:
        iterator(const SparseIntMap *map, size_t pos) : map_(map), pos_(pos) { advance_to_valid(); }
        const Entry operator*() const { return Entry{map_->keys[pos_], map_->values[pos_]}; }
        bool operator==(const iterator &other) const { return pos_ == other.pos_; }
        bool operator!=(const iterator &other) const { return pos_ != other.pos_; }
        iterator &operator++()
        {
            ++pos_;
            advance_to_valid();
            return *this;
        }
        Value &value() const { return map_->values[pos_]; }

    private:
        void advance_to_valid()
        {
            while (pos_ < map_->capacity_ && map_->keys[pos_] == map_->EMPTY)
                ++pos_;
        }
        const SparseIntMap *map_;
        size_t pos_;
    };

    SparseIntMap(size_t initial_cap = 16) : size_(0)
    {
        EMPTY = std::numeric_limits<Key>::min();
        // 【修复】使用自定义的 next_power_of_two 替代 std::bit_ceil
        capacity_ = initial_cap < 16 ? 16 : next_power_of_two(initial_cap);
        mask_ = capacity_ - 1;

        keys = new Key[capacity_];
        values = new Value[capacity_];
        std::fill(keys, keys + capacity_, EMPTY);
    }

    // 深拷贝构造
    SparseIntMap(const SparseIntMap &other)
        : size_(other.size_), capacity_(other.capacity_), mask_(other.mask_), EMPTY(other.EMPTY)
    {
        keys = new Key[capacity_];
        values = new Value[capacity_];
        std::memcpy(keys, other.keys, capacity_ * sizeof(Key));
        std::copy(other.values, other.values + capacity_, values);
    }

    // 赋值运算符
    SparseIntMap &operator=(const SparseIntMap &other)
    {
        if (this != &other)
        {
            delete[] keys;
            delete[] values;
            size_ = other.size_;
            capacity_ = other.capacity_;
            mask_ = other.mask_;
            EMPTY = other.EMPTY;
            keys = new Key[capacity_];
            values = new Value[capacity_];
            std::memcpy(keys, other.keys, capacity_ * sizeof(Key));
            std::copy(other.values, other.values + capacity_, values);
        }
        return *this;
    }

    ~SparseIntMap()
    {
        delete[] keys;
        delete[] values;
    }

    // 插入或查找
    inline Value &operator[](const Key &k)
    {
        // 负载因子 0.75
        if (size_ > (capacity_ >> 1) + (capacity_ >> 2))
            rehash(capacity_ << 1);

        size_t idx = fibonacci_hash(k) & mask_;

        while (keys[idx] != EMPTY)
        {
            if (keys[idx] == k)
                return values[idx];
            idx = (idx + 1) & mask_;
        }

        keys[idx] = k;
        values[idx] = Value();
        ++size_;
        return values[idx];
    }

    // 查找
    inline iterator find(const Key &k) const
    {
        size_t idx = fibonacci_hash(k) & mask_;

        while (keys[idx] != EMPTY)
        {
            if (keys[idx] == k)
                return iterator(this, idx);
            idx = (idx + 1) & mask_;
        }
        return end();
    }

    iterator end() const { return iterator(this, capacity_); }

    void clear()
    {
        std::fill(keys, keys + capacity_, EMPTY);
        size_ = 0;
    }

    size_t capacity() const { return capacity_; }

private:
    Key *keys;
    Value *values;
    size_t capacity_;
    size_t mask_;
    size_t size_;
    Key EMPTY;

    static inline size_t fibonacci_hash(Key k)
    {
        static constexpr uint64_t GOLDEN_RATIO = 0x9E3779B97F4A7C15ULL;
        return static_cast<size_t>((static_cast<uint64_t>(k) * GOLDEN_RATIO));
    }

    // 【核心修复】手写 bit_ceil 的替代实现 (兼容 C++11/14/17)
    static size_t next_power_of_two(size_t x)
    {
        if (x <= 1)
            return 1;
        // 使用 GCC/Clang 内置指令计算前导零个数
        // 对于 64 位系统使用 __builtin_clzll
        return 1ULL << (64 - __builtin_clzll(x - 1));
    }

    void rehash(size_t new_cap)
    {
        Key *old_keys = keys;
        Value *old_vals = values;
        size_t old_cap = capacity_;

        capacity_ = new_cap;
        mask_ = new_cap - 1;
        keys = new Key[capacity_];
        values = new Value[capacity_];
        std::fill(keys, keys + capacity_, EMPTY);
        size_ = 0;

        for (size_t i = 0; i < old_cap; ++i)
        {
            if (old_keys[i] != EMPTY)
            {
                size_t idx = fibonacci_hash(old_keys[i]) & mask_;
                while (keys[idx] != EMPTY)
                    idx = (idx + 1) & mask_;
                keys[idx] = old_keys[i];
                values[idx] = old_vals[i];
                ++size_;
            }
        }

        delete[] old_keys;
        delete[] old_vals;
    }
};

#endif