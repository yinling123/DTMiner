#ifndef ManualStack_H
#define ManualStack_H

#include <iostream>
#include <stdexcept>
#include <algorithm>

template <typename T>
class ManualStack
{
private:
    T *data;      // 栈数据指针
    int capacity; // 栈容量
    int topIndex; // 栈顶索引

public:
    // 构造函数：初始化栈容量（默认10）
    explicit ManualStack(int initialCapacity = 6)
        : capacity(initialCapacity), topIndex(-1)
    {
        if (initialCapacity <= 0)
        {
            capacity = 10; // 默认最小容量
        }
        data = new T[capacity];
    }

    // 拷贝构造函数（深拷贝）
    ManualStack(const ManualStack &other)
        : capacity(other.capacity), topIndex(other.topIndex)
    {
        data = new T[capacity];
        for (int i = 0; i <= topIndex; ++i)
        {
            data[i] = other.data[i];
        }
    }

    /**
     * 删除的地方不可动,这部分是为了避免重复指针指向,不然会重复释放
     * 就让只维护自己的指针
     */
    ManualStack &operator=(const ManualStack &other)
    {
        if (this != &other)
        {
            // delete[] data;
            capacity = other.capacity;
            topIndex = other.topIndex;
            // data = new T[capacity];
            for (int i = 0; i <= topIndex; ++i)
            {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    /**
     * 移动赋值运算符
     */
    ManualStack &operator=(ManualStack &&other)
    {
        if (this != &other)
        {
            delete[] data;
            capacity = other.capacity;
            topIndex = other.topIndex;
            data = other.data;
            other.data = nullptr;
        }
        return *this;
    }

    /**
     * 移动拷贝构造函数
    */
    ManualStack(ManualStack &&other)
    {
        capacity = other.capacity;
        topIndex = other.topIndex;
        data = other.data;
        other.data = nullptr;
    }

    // 析构函数：释放内存
    ~ManualStack()
    {
        if (data != nullptr)
        {
            delete[] data;
        }
    }

    // 入栈（自动扩容）
    void push(const T &value)
    {
        if (isFull())
        {
            // 扩容策略：容量翻倍 +1（避免初始容量为0）
            int newCapacity = (capacity == 0) ? 1 : capacity * 2 + 1;
            T *newData = new T[newCapacity];
            for (int i = 0; i <= topIndex; ++i)
            {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
            capacity = newCapacity;
        }
        data[++topIndex] = value;
    }

    // 出栈
    void pop()
    {
        if (empty())
        {
            throw std::out_of_range("Stack underflow");
        }
        --topIndex;
    }

    // 获取栈顶元素
    T &top()
    {
        if (empty())
        {
            throw std::out_of_range("Stack is empty");
        }
        return data[topIndex];
    }

    // 清空栈（逻辑清空，保留内存）
    void clear()
    {
        topIndex = -1;
    }

    // 调整栈容量
    void resize(int newCapacity)
    {
        if (newCapacity <= 0)
        {
            throw std::invalid_argument("Capacity must be positive");
        }
        if (newCapacity == capacity)
            return;

        // 计算需要保留的元素数量
        int elementsToKeep = std::min(topIndex + 1, newCapacity);

        T *newData = new T[newCapacity];
        for (int i = 0; i < elementsToKeep; ++i)
        {
            newData[i] = data[i];
        }

        delete[] data;
        data = newData;
        capacity = newCapacity;
        topIndex = elementsToKeep - 1; // 调整栈顶指针
    }

    // 判断栈是否为空
    bool empty() const
    {
        return topIndex == -1;
    }

    // 判断栈是否已满
    bool isFull() const
    {
        return topIndex == capacity - 1;
    }

    // 获取当前容量
    int getCapacity() const
    {
        return capacity;
    }

    // 获取元素数量
    int size() const
    {
        return topIndex + 1;
    }
};

#endif