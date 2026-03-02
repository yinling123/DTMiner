#include "NumaTool.h"

std::mutex mtx;

void *migrate_to_node(void *src, size_t size, int dst_node)
{
    // 1. 在目标 NUMA 节点上分配等大小内存
    void *dst = numa_alloc_onnode(size, dst_node);
    if (!dst)
        return nullptr;

    // 2. 拷贝原始数据
    memcpy(dst, src, size);

    // （可选）释放原来的
    // numa_free(src, size);

    return dst;
}

void bind_to_core(int core_id)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    // {
    //     std::lock_guard<std::mutex> lock(mtx);
    //     std::cout << "Binding to core " << core_id << "..." << std::endl;
    // }

    int ret = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (ret != 0)
    {
        std::cerr << "Failed to bind to core " << core_id << std::endl;
    }
}