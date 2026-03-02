// #include <iostream>   // 标准输入输出
// #include <thread>     // C++线程库
// #include <vector>     // 动态数组
// #include <numa.h>     // NUMA API库
// #include <sched.h>    // 调度器相关定义
// #include <unistd.h>   // POSIX操作系统API

// // 根据线程ID获取应该绑定的CPU核心
// // thread_id: 线程标识符
// // threads_per_node: 每个NUMA节点分配的线程数
// int get_cpu_for_thread(int thread_id, int threads_per_node) {
//     // 计算线程属于哪个NUMA节点
//     int numa_node = thread_id / threads_per_node;
    
//     // 获取该NUMA节点可用的CPU列表
//     struct bitmask* cpus = numa_allocate_cpumask();  // 分配CPU掩码结构
//     numa_node_to_cpus(numa_node, cpus);              // 获取该节点所有可用CPU
    
//     // 计算线程在该节点内的局部ID
//     int local_id = thread_id % threads_per_node;
    
//     // 找到第local_id个可用的CPU
//     int cpu_idx = -1;
//     int count = 0;
//     for (unsigned int i = 0; i < cpus->size; i++) {
//         if (numa_bitmask_isbitset(cpus, i)) {  // 检查该位是否被设置(即该CPU是否可用)，判断当前cpu是否在当前NUMA节点内
//             if (count++ == local_id) {         // 找到对应位置的CPU
//                 cpu_idx = i;
//                 break;
//             }
//         }
//     }
    
//     numa_free_cpumask(cpus);  // 释放CPU掩码结构
//     return cpu_idx;
// }

// // 线程工作函数
// void worker(int thread_id, int numa_node) {
//     // === 1. 绑定线程到指定核心 ===
//     // 每个NUMA节点32物理核心，根据线程ID找到对应的CPU核心
//     int cpu = get_cpu_for_thread(thread_id, 32);
    
//     // 初始化CPU亲和性掩码
//     cpu_set_t cpuset;
//     CPU_ZERO(&cpuset);          // 清空集合
//     CPU_SET(cpu, &cpuset);      // 将指定CPU加入集合
    
//     // 将当前线程绑定到指定CPU
//     if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset)) {
//         perror("pthread_setaffinity_np");
//         exit(EXIT_FAILURE);
//     }

//     // === 2. 在本地NUMA节点分配数据 ===
//     // 分配并初始化NUMA节点掩码
//     struct bitmask* nodemask = numa_allocate_nodemask();
//     numa_bitmask_setbit(nodemask, numa_node);  // 设置关注的NUMA节点
    
//     // 设置当前线程的内存分配策略：仅从指定的NUMA节点分配内存
//     numa_set_membind(nodemask);
    
//     // 分配大块本地内存
//     const size_t size = 1024 * 1024 * 100; // 100MB
//     // 在指定的NUMA节点上分配内存
//     void* local_data = numa_alloc_onnode(size, numa_node);
//     if (!local_data) {
//         std::cerr << "Failed to allocate on node " << numa_node << std::endl;
//         exit(EXIT_FAILURE);
//     }
    
//     // === 3. 报告绑定情况 ===
//     char name[16];
//     pthread_getname_np(pthread_self(), name, sizeof(name));  // 获取线程名称
//     std::cout << "Thread " << thread_id 
//               << " (" << name << ") bound to CPU: " << cpu
//               << ", NUMA node: " << numa_node
//               << ", Data addr: " << local_data << std::endl;
    
//     // === 4. 实际工作负载 ===
//     // 这里执行具体任务，保证内存访问在本地
    
//     // === 5. 清理 ===
//     numa_free(local_data, size);           // 释放之前分配的内存
//     numa_free_nodemask(nodemask);          // 释放节点掩码
// }

// int main() {
//     // 检查系统是否支持NUMA功能
//     if (numa_available() < 0) {
//         std::cerr << "NUMA not available!" << std::endl;
//         return EXIT_FAILURE;
//     }
    
//     const int THREAD_COUNT = 64; // 64物理核心（非超线程）
//     std::vector<std::thread> threads;
    
//     for (int i = 0; i < THREAD_COUNT; i++) {
//         // 计算线程所属的NUMA节点
//         // 假设每个节点有32个线程/核心，节点0处理0-31号线程，节点1处理32-63号线程
//         int numa_node = i / 32; // 节点0: 0-31, 节点1: 32-63
        
//         threads.emplace_back(worker, i, numa_node);  // 创建线程并运行worker函数
//     }
    
//     for (auto& t : threads) {
//         t.join();  // 等待所有线程完成
//     }
    
//     return EXIT_SUCCESS;
// }