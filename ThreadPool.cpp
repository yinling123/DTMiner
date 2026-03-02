#include "ThreadPool.h"
#include "GraphSearch.h"
#include <iostream>
#include <xmmintrin.h>
#include <random>
#include "HighResClock.h"

// 现在准备转移过去

// 为每个线程设置私有变量

// 【优化】极速随机数生成器
struct FastRNG
{
    uint32_t state;
    explicit FastRNG(uint32_t seed) : state(seed == 0 ? 1 : seed) {}
    inline uint32_t next()
    {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return state = x;
    }
    inline uint32_t next_u32(uint32_t range)
    {
        return (uint32_t)(((uint64_t)next() * range) >> 32);
    }
};
thread_local bool ThreadPool::is_worker = false;
thread_local int ThreadPool::thread_id = -1;

// 1. 声明一个默认构造的、线程本地的随机数生成器。
thread_local std::mt19937 thread_rand_generator;
thread_local std::unique_ptr<FastRNG> tls_rng;

ThreadPool::ThreadPool(size_t threads, int numa_node) : stop(false)
{
    // 当最大队列有超过100个任务，就进行任务窃取
    threshold = 20;
    thread_num = threads;

    /**
     * 创建graphSearch对象，用于循环利用
     */
    for (size_t i = 0; i < threads; ++i)
    {
        vec_graph.push_back(GraphSearch());
    }

    // std::cout << "开始调用线程" << std::endl;
    for (size_t i = 0; i < threads; ++i)
    {
        workers.emplace_back([this, i, threads, numa_node]
                             {
            // 初始化大小后，性能下降，故不处理
            // thread_data_vector[i]->queue.resize(50);
            // thread_data_vector[i]->queue.clear();
            bind_to_core(i % threads + numa_node * threads);
            numa_set_localalloc();

            is_worker = true;
            thread_id = i;
            
            // 2. 在线程ID赋值后，为本线程的生成器进行一次性的、唯一的播种。
            // thread_rand_generator.seed(std::random_device{}() + thread_id);
            tls_rng = std::make_unique<FastRNG>(std::random_device{}() + i);

            // 4个线程分一组
            group_size = threads / 4;

            // ThreadData& td = *thread_data_vector[i]; // 直接引用
            while (true) {
                /**
                 * 线程等待主进程修改信号，确定执行什么函数
                 * 阻塞完成，立马释放，防止一直抢占锁
                 * notify_all()函数是让所有等待的线程唤醒，然后抢占锁一次，判断条件一次，要是条件失败，则重新阻塞；
                 * 要是未获取锁，则再次进行获取
                 * 前几轮循环正常；最后一轮直接退出
                */
                {
                    std::unique_lock<std::mutex> lock(flag_mutex);
                    // std::cout << "thread " << i << " is waiting" << std::endl;
                    flag_condition.wait(lock, [this] { return flag != 0 || stop;});
                }
                // std::cout << "thread " << i << " is running" << std::endl;
                if(stop){
                    break;
                }

                /**
                 * 先执行第一阶段，再执行第二阶段
                */
                for(int idx = start_part; idx < end_part; idx++){
                    /**
                     * 初始化范围和变量，考虑转化为连续分配
                    */ 
                    vec_graph[i].p = &(p_s -> at(idx));
                    vec_graph[i].part_idx = idx;

                    // std::cout << "开始执行新任务" << idx << "线程" << thread_id << std::endl;

                    {
                        vec_graph[i].findNewOrderedSubgraphs(&edge_idx_new[idx], &edge_mutex_new, thread_id, threads, phase_tasks_beg_first, phase_tasks_end_first);
                    }

                    /**
                     * 执行完自己的任务，开始进行工作窃取
                     * 如果当前窃取失败，直接out;否则处理完成接着窃取
                     * 可以考虑优化为当前NUMA节点无法窃取就窃取其他NUMA节点
                     * 先关闭下工作窃取
                    */
                    {
                        int fail_cnt = 0;
                        while(true)
                        {
                            if(tryToStealFirst(idx))
                            {
                                /**
                                 * 窃取后先执行窃取的任务，再修改当前可窃取
                                */
                                vec_graph[i].findNewOrderedSubgraphs(&edge_idx_new[idx], &edge_mutex_new, thread_id, threads, phase_tasks_beg_first, phase_tasks_end_first);
                            }
                            else
                            {
                                fail_cnt++;
                                if(fail_cnt > 3)
                                {
                                    break;
                                }
                            }
                        }
                    }

                    // std::cout << "结束执行新任务" << idx << "线程" << thread_id << std::endl;

                    int spin = 0;
                    if(idx >= 1)
                    {
                        while(thread_count_first[idx - 1].load() != threads + 1)
                        {
                            if(spin < 1000)
                            {
                                ++spin;
                                _mm_pause();
                            }
                            else
                            {
                                std::this_thread::sleep_for(std::chrono::microseconds(20));
                            }
                        }
                    }
                    
                    {
                        // std::cout << "开始执行旧任务" << idx << "线程" << thread_id << std::endl;
                        vec_graph[i].findOrderedSubgraphs(&q -> at(idx), &edge_idx_old[idx], &edge_mutex_old, thread_id, threads, &otherPool -> q -> at(idx));
                    }

                    /**
                     * 任务队列内任务窃取负载均衡
                     * 如果对方NUMA核心没有执行完成，旧任务的初始化，必须等待，否则会出现错误
                    */
                    spin = 0;

                    if(idx >= 1)
                    {   
                        while(otherPool -> thread_count_first[idx - 1] != threads + 1)
                        {
                            if(spin < 1000)
                            {
                                ++spin;
                                _mm_pause();
                            }
                            else
                            {
                                std::this_thread::sleep_for(std::chrono::microseconds(20));
                            }
                        }

                        {
                            int fail_cnt = 0;
                            while(true)
                            {
                                /**
                                 * 每次都只尝试窃取一次，窃取完成后立马执行，防止重复窃取导致漏解
                                 * 逻辑没啥问题，但是不知道哪里出了问题，就是这个窃取其他任务队列的问题
                                 * numa核心值为0、1；所以应该根据这个进行调整
                                */
                                int flag = tryToStealOld(idx);
                                if(flag == numa_node)
                                {
                                    /**
                                     * 窃取后先执行窃取的任务，再修改当前可窃取
                                    */
                                    // fail_cnt = 0;
                                    vec_graph[i].findOrderedSubgraphs(&q -> at(idx), &edge_idx_old[idx], &edge_mutex_old, thread_id, threads, &otherPool -> q -> at(idx));
                                }
                                else if(flag == 1 - numa_node)
                                {
                                    // fail_cnt = 0;
                                    vec_graph[i].findOthgerOrderedSubgraphs(&q -> at(idx), &edge_idx_old[idx], &edge_mutex_old, thread_id, threads, &otherPool -> q -> at(idx));
                                }
                                else
                                {
                                    fail_cnt++;
                                    if(fail_cnt >= 3)
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // std::cout << "开始执行新任务" << idx << "线程" << thread_id << std::endl;
                    // if(idx < new_end)

                    /**
                     * p_s -> size() - 1
                     * 当前块处理完成,就释放当前块队列的内存
                     * 并且销毁当前队列的值，减少不必要的空间开销
                     * 如果直接fetch_add会导致其他立刻执行下一个块，对应的数据可能不为最新，所以还是应该修改下
                     * 每次都释放前面一个块的任务队列
                     * 可以当前只阻塞一个线程来等待对方的旧任务执行完成，这样可以确保
                    */
                    if(thread_count_first[idx].fetch_add(1) == threads - 1)
                    {
                        /**
                         * 最后一个线程要确保任务执行完成才可以执行下一个块
                        */
                        while(otherPool -> thread_count_first[idx].load() < threads)
                        {
                            /**
                             * 休眠一段时间再判断
                            */
                            std::this_thread::sleep_for(std::chrono::microseconds(20));
                        }

                        if(idx >= 1)
                        {
                            pool_memory -> add_segments(q -> at(idx).segments);
                            // q -> at(idx - 1).destroy_temp();
                            q -> at(idx).clear_queue();
                        }

                        // pool_memory -> add_segments(q -> at(idx).segments);
                        // // q -> at(idx).~LockFreeQueue();
                        // q -> at(idx).destroy_temp();
                        // std::cout << "当前执行时间: " << q -> at(idx).total_times / 1000 << "ms" << std::endl;
                        flag = 0; 
                        // std::cout << "当前执行到块" << idx << "完成" << std::endl;
                        if(idx < end_part - 1)
                        {
                            // old_edge_idx.store(0);
                            int size = q -> at(idx + 1).size();

                            for(int i = 0; i < thread_num; i++)
                            {
                                int bg = size / threads * i;
                                // if(idx < new_end)
                                {
                                    int ed = bg + size / threads;
                                    if (size % thread_num > i)
                                    {
                                        bg += i;
                                        ed += i + 1;
                                    }
                                    else
                                    {
                                        bg += size % threads;
                                        ed += size % threads;
                                    }

                                    old_flag[i + (idx + 1) * thread_num].store(numa_id, std::memory_order_relaxed);
                                    old_task_beg[i + (idx + 1) * thread_num].store(bg, std::memory_order_relaxed);
                                    old_task_end[i + (idx + 1) * thread_num].store(ed, std::memory_order_relaxed);
                                    steal_flag[i].store(false, std::memory_order_release);
                                }
                            }
                        }
                        thread_count_first[idx].fetch_add(1);
                    }
                    
                    // int spin = 0;
                    // while(thread_count[idx] != threads + 1 || otherPool->thread_count[idx] != threads + 1) {
                    //     if (spin < 1000)
                    //         ++spin, _mm_pause();  // 更长时间主动轮询
                    //     else
                    //         std::this_thread::sleep_for(std::chrono::microseconds(20));  // 不进入内核态
                    // }

                }

                /**
                 * 执行第二阶段，就是第二次扫描开始了
                */
                // std::cout << "开始执行第二阶段" << std::endl;

                for(int idx = start_part; idx < end_part; idx++){
                    /**
                     * 初始化范围和变量，考虑转化为连续分配
                    */ 
                    vec_graph[i].p = &(p_s -> at(idx));
                    vec_graph[i].part_idx = idx;

                    {
                        vec_graph[i].findNewOrderedSubgraphs(&edge_idx_new[idx], &edge_mutex_new, thread_id, threads, phase_tasks_beg_second, phase_tasks_end_second);
                    }

                    /**
                     * 执行完自己的任务，开始进行工作窃取
                     * 如果当前窃取失败，直接out;否则处理完成接着窃取
                     * 可以考虑优化为当前NUMA节点无法窃取就窃取其他NUMA节点
                     * 先关闭下工作窃取
                    */
                    {
                        int fail_cnt = 0;
                        while(true)
                        {
                            if(tryToStealSecond(idx))
                            {
                                /**
                                 * 窃取后先执行窃取的任务，再修改当前可窃取
                                */
                                vec_graph[i].findNewOrderedSubgraphs(&edge_idx_new[idx], &edge_mutex_new, thread_id, threads, phase_tasks_beg_second, phase_tasks_end_second);
                            }
                            else
                            {
                                fail_cnt++;
                                if(fail_cnt > 3)
                                {
                                    break;
                                }
                            }
                        }
                    }

                    int spin = 0;
                    if(idx >= 1)
                    {
                        while(thread_count_second[idx - 1] != threads + 1)
                        {
                            if(spin < 1000)
                            {
                                ++spin;
                                _mm_pause();
                            }
                            else
                            {
                                std::this_thread::sleep_for(std::chrono::microseconds(20));
                            }
                        }
                    }
                    
                    {
                        // std::cout << "开始执行旧任务" << idx << "线程" << thread_id << std::endl;
                        vec_graph[i].findOrderedSubgraphs(&q -> at(idx), &edge_idx_old[idx], &edge_mutex_old, thread_id, threads, &otherPool -> q -> at(idx));
                    }

                    /**
                     * 任务队列内任务窃取负载均衡
                     * 如果对方NUMA核心没有执行完成，旧任务的初始化，必须等待，否则会出现错误
                    */
                    spin = 0;

                    if(idx >= 1)
                    {   
                        while(otherPool -> thread_count_second[idx - 1] != threads + 1)
                        {
                            if(spin < 1000)
                            {
                                ++spin;
                                _mm_pause();
                            }
                            else
                            {
                                std::this_thread::sleep_for(std::chrono::microseconds(20));
                            }
                        }

                        {
                            int fail_cnt = 0;
                            while(true)
                            {
                                /**
                                 * 每次都只尝试窃取一次，窃取完成后立马执行，防止重复窃取导致漏解
                                 * 逻辑没啥问题，但是不知道哪里出了问题，就是这个窃取其他任务队列的问题
                                 * numa核心值为0、1；所以应该根据这个进行调整
                                */
                                int flag = tryToStealOld(idx);
                                if(flag == numa_node)
                                {
                                    /**
                                     * 窃取后先执行窃取的任务，再修改当前可窃取
                                    */
                                    // fail_cnt = 0;
                                    vec_graph[i].findOrderedSubgraphs(&q -> at(idx), &edge_idx_old[idx], &edge_mutex_old, thread_id, threads, &otherPool -> q -> at(idx));
                                }
                                else if(flag == 1 - numa_node)
                                {
                                    // fail_cnt = 0;
                                    vec_graph[i].findOthgerOrderedSubgraphs(&q -> at(idx), &edge_idx_old[idx], &edge_mutex_old, thread_id, threads, &otherPool -> q -> at(idx));
                                }
                                else
                                {
                                    fail_cnt++;
                                    if(fail_cnt >= 3)
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // std::cout << "开始执行新任务" << idx << "线程" << thread_id << std::endl;
                    // if(idx < new_end)

                    /**
                     * p_s -> size() - 1
                     * 当前块处理完成,就释放当前块队列的内存
                     * 并且销毁当前队列的值，减少不必要的空间开销
                     * 如果直接fetch_add会导致其他立刻执行下一个块，对应的数据可能不为最新，所以还是应该修改下
                     * 每次都释放前面一个块的任务队列
                     * 可以当前只阻塞一个线程来等待对方的旧任务执行完成，这样可以确保
                    */
                    if(thread_count_second[idx].fetch_add(1) == threads - 1)
                    {
                        /**
                         * 最后一个线程要确保任务执行完成才可以执行下一个块
                        */
                        while(otherPool -> thread_count_second[idx].load() < threads)
                        {
                            /**
                             * 休眠一段时间再判断
                            */
                            std::this_thread::sleep_for(std::chrono::microseconds(20));
                        }

                        if(idx >= 1)
                        {
                            // 清理上一个块的资源
                            q -> at(idx).clear_queue();
                            pool_memory -> add_segments(q -> at(idx).segments);
                            q -> at(idx).destroy_temp();
                        }

                        // pool_memory -> add_segments(q -> at(idx).segments);
                        // // q -> at(idx).~LockFreeQueue();
                        // q -> at(idx).destroy_temp();
                        // std::cout << "当前执行时间: " << q -> at(idx).total_times / 1000 << "ms" << std::endl;
                        flag = 0; 
                        // std::cout << "当前执行到块" << idx << "完成" << std::endl;
                        if(idx < end_part - 1)
                        {
                            // old_edge_idx.store(0);
                            int size = q -> at(idx + 1).size();

                            for(int i = 0; i < thread_num; i++)
                            {
                                int bg = size / threads * i;
                                // if(idx < new_end)
                                {
                                    int ed = bg + size / threads;
                                    if (size % thread_num > i)
                                    {
                                        bg += i;
                                        ed += i + 1;
                                    }
                                    else
                                    {
                                        bg += size % threads;
                                        ed += size % threads;
                                    }

                                    old_flag[i + (idx + 1) * thread_num].store(numa_id, std::memory_order_relaxed);
                                    old_task_beg[i + (idx + 1) * thread_num].store(bg, std::memory_order_relaxed);
                                    old_task_end[i + (idx + 1) * thread_num].store(ed, std::memory_order_relaxed);
                                    steal_flag[i].store(false, std::memory_order_release);
                                }
                            }

                            // uint64_t start = HighResClock::now_ns();

                            q->at(idx + 1).flush_buffers();

                            // uint64_t end = HighResClock::now_ns();

                            // vec_graph[i].trans_time += end - start;
                        }
                        thread_count_second[idx].fetch_add(1, std::memory_order_release);
                    }
                    
                    // int spin = 0;
                    // while(thread_count[idx] != threads + 1 || otherPool->thread_count[idx] != threads + 1) {
                    //     if (spin < 1000)
                    //         ++spin, _mm_pause();  // 更长时间主动轮询
                    //     else
                    //         std::this_thread::sleep_for(std::chrono::microseconds(20));  // 不进入内核态
                    // }
                }


                finished.fetch_add(1);

            
                {
                    // std::lock_guard<std::mutex> lock(count_mtx);
                    /**
                     * 最后执行完成才统计结果
                    */
                    pool -> res_num += vec_graph[i].res_num;
                    // pool -> binary_prepare += vec_graph[i].binary_prepare;
                    // pool -> binary_search_time += vec_graph[i].binary_search_time;
                    // pool -> trans_time += vec_graph[i].trans_time;
                    // pool -> check_time += vec_graph[i].check_time;
                    // std::cout << finished << " " << numa_node << std::endl;
                }

                // std::cout << "线程" << thread_id << "处理完成" << std::endl;

                if(stop){
                    return;
                }
                
            } });
    }
}

/**
 * 仅窃取起始边任务，并且每次窃取完成后依次进行操作
 * 这部分改动不影响最初的运行
 */
bool ThreadPool::tryToSteal(int part_idx)
{
    /**
     * 从自身索引进行遍历，找到可以窃取的线程
     * 对于一个可窃取线程，一次只允许一个线程窃取
     * 当前阶段的end必须是原子变量，否则会出现异常
     * 当当前线程窃取完成，其他线程也会来进行窃取，所以必须一致同步，不然可能存在问题
     * 开始窃取的时候，直接先给自己加锁
     */

    bool flag = false;
    int i = 0;
    std::uniform_int_distribution<int> dist(0, thread_num - 1);
    int offset = dist(thread_rand_generator);
    for (int j = 0; j < thread_num; j++)
    {
        i = (j + offset) % thread_num;
        if (i == thread_id)
        {
            continue;
        }
        /**
         * 这部分是和全局压入任务范围同步
         */
        if (phase_tasks_end[i + part_idx * thread_num].load(std::memory_order_acquire) - phase_tasks_beg[i + part_idx * thread_num].load(std::memory_order_acquire) >= threshold)
        {
            // std::cout << "thread " << thread_id << " steal from thread " << i << std::endl;
            bool expected = false;
            if (steal_flag[i].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
            {
                int bg = phase_tasks_beg[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int ed = phase_tasks_end[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int total = ed - bg;
                if (total < threshold)
                {
                    // 当前已经被抢占获取了，恢复状态，并且立马去获取其他任务
                    steal_flag[i].store(false, std::memory_order_release);
                    continue;
                }

                // 确保是整除的
                // thread_num / 2 * thread_num
                int steal_cnt = std::min((ed - bg) / 2, 32);
                int steal_beg = bg;
                int steal_end = bg + steal_cnt;
                /**
                 * 一定要是release确保对于其他线程，这个修改可见；并且后续通过 读取这个变量时，必须使用acquire
                 * 在线程更新自己窃取到的任务的时候，应该确保自身的更新完成，即不会更新到一半然后被其他线程窃取，这样可能导致错误
                 */
                phase_tasks_beg[i + part_idx * thread_num].store(steal_end, std::memory_order_release);

                if (steal_flag[thread_id].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    vec_graph[thread_id].start_idx = steal_beg;
                    // vec_graph[thread_id].end_idx = std::min(steal_end, steal_beg + steal_cnt);
                    vec_graph[thread_id].end_idx = steal_end;
                    // phase_tasks_beg[thread_id].store(vec_graph[thread_id].end_idx, std::memory_order_relaxed);
                    // phase_tasks_end[thread_id].store(steal_end, std::memory_order_relaxed);
                }

                flag = true;
                /**
                 * 当前线程窃取完成后，不允许其他线程窃取，避免频繁原子访问
                 */
                steal_flag[i].store(false, std::memory_order_release);
                steal_flag[thread_id].store(false, std::memory_order_release);
                // break;
                return flag;
            }
        }
    }

    /**
     * 当前不存在可窃取任务，转而窃取另一个NUMA节点任务
     */
    for (int j = 0; j < thread_num; j++)
    {
        i = (j + offset) % thread_num;
        if (i == thread_id)
        {
            continue;
        }
        /**
         * 这部分是和全局压入任务范围同步
         */
        if (otherPool->phase_tasks_end[i + part_idx * thread_num].load(std::memory_order_acquire) - otherPool->phase_tasks_beg[i + part_idx * thread_num].load(std::memory_order_acquire) >= threshold)
        {
            // std::cout << "thread " << thread_id << " steal from thread " << i << std::endl;
            bool expected = false;
            if (otherPool->steal_flag[i].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
            {
                int bg = otherPool->phase_tasks_beg[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int ed = otherPool->phase_tasks_end[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int total = ed - bg;
                if (total < threshold)
                {
                    // 当前已经被抢占获取了，恢复状态，并且立马去获取其他任务
                    otherPool->steal_flag[i].store(false, std::memory_order_release);
                    continue;
                }

                // 确保是整除的
                // thread_num / 2 * thread_num
                int steal_cnt = std::min((ed - bg) / 2, 32);
                int steal_beg = bg;
                int steal_end = bg + steal_cnt;
                /**
                 * 一定要是release确保对于其他线程，这个修改可见；并且后续通过 读取这个变量时，必须使用acquire
                 * 在线程更新自己窃取到的任务的时候，应该确保自身的更新完成，即不会更新到一半然后被其他线程窃取，这样可能导致错误
                 */
                otherPool->phase_tasks_beg[i + part_idx * thread_num].store(steal_end, std::memory_order_release);

                if (steal_flag[thread_id].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    vec_graph[thread_id].start_idx = steal_beg;
                    // vec_graph[thread_id].end_idx = std::min(steal_end, steal_beg + steal_cnt);
                    vec_graph[thread_id].end_idx = steal_end;
                    // phase_tasks_beg[thread_id].store(vec_graph[thread_id].end_idx, std::memory_order_relaxed);
                    // phase_tasks_end[thread_id].store(steal_end, std::memory_order_relaxed);
                }

                flag = true;
                /**
                 * 当前线程窃取完成后，不允许其他线程窃取，避免频繁原子访问
                 */
                otherPool->steal_flag[i].store(false, std::memory_order_release);
                steal_flag[thread_id].store(false, std::memory_order_release);
                // break;
                return flag;
            }
        }
    }

    return flag;
}

bool ThreadPool::tryToStealFirst(int part_idx)
{

    bool flag = false;
    int i = 0;
    // std::uniform_int_distribution<int> dist(0, thread_num - 1);
    // int offset = dist(thread_rand_generator);
    int offset = tls_rng->next_u32(thread_num);
    for (int j = 0; j < thread_num; j++)
    {
        i = (j + offset) % thread_num;
        if (i == thread_id)
        {
            continue;
        }
        /**
         * 这部分是和全局压入任务范围同步
         */
        if (phase_tasks_end_first[i + part_idx * thread_num].load(std::memory_order_acquire) - phase_tasks_beg_first[i + part_idx * thread_num].load(std::memory_order_acquire) >= threshold)
        {
            // std::cout << "thread " << thread_id << " steal from thread " << i << std::endl;
            bool expected = false;
            if (steal_flag[i].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
            {
                int bg = phase_tasks_beg_first[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int ed = phase_tasks_end_first[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int total = ed - bg;
                if (total < threshold)
                {
                    // 当前已经被抢占获取了，恢复状态，并且立马去获取其他任务
                    steal_flag[i].store(false, std::memory_order_release);
                    continue;
                }

                // 确保是整除的
                // thread_num / 2 * thread_num
                int steal_cnt = std::min((ed - bg) / 2, 32);
                int steal_beg = bg;
                int steal_end = bg + steal_cnt;
                /**
                 * 一定要是release确保对于其他线程，这个修改可见；并且后续通过 读取这个变量时，必须使用acquire
                 * 在线程更新自己窃取到的任务的时候，应该确保自身的更新完成，即不会更新到一半然后被其他线程窃取，这样可能导致错误
                 */
                phase_tasks_beg_first[i + part_idx * thread_num].store(steal_end, std::memory_order_release);

                if (steal_flag[thread_id].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    vec_graph[thread_id].start_idx = steal_beg;
                    // vec_graph[thread_id].end_idx = std::min(steal_end, steal_beg + steal_cnt);
                    vec_graph[thread_id].end_idx = steal_end;
                    // phase_tasks_beg[thread_id].store(vec_graph[thread_id].end_idx, std::memory_order_relaxed);
                    // phase_tasks_end[thread_id].store(steal_end, std::memory_order_relaxed);
                }

                flag = true;
                /**
                 * 当前线程窃取完成后，不允许其他线程窃取，避免频繁原子访问
                 */
                steal_flag[i].store(false, std::memory_order_release);
                steal_flag[thread_id].store(false, std::memory_order_release);
                // break;
                return flag;
            }
        }
    }

    /**
     * 当前不存在可窃取任务，转而窃取另一个NUMA节点任务
     */
    for (int j = 0; j < thread_num; j++)
    {
        i = (j + offset) % thread_num;
        if (i == thread_id)
        {
            continue;
        }
        /**
         * 这部分是和全局压入任务范围同步
         */
        if (otherPool->phase_tasks_end_first[i + part_idx * thread_num].load(std::memory_order_acquire) - otherPool->phase_tasks_beg_first[i + part_idx * thread_num].load(std::memory_order_acquire) >= threshold)
        {
            // std::cout << "thread " << thread_id << " steal from thread " << i << std::endl;
            bool expected = false;
            if (otherPool->steal_flag[i].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
            {
                int bg = otherPool->phase_tasks_beg_first[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int ed = otherPool->phase_tasks_end_first[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int total = ed - bg;
                if (total < threshold)
                {
                    // 当前已经被抢占获取了，恢复状态，并且立马去获取其他任务
                    otherPool->steal_flag[i].store(false, std::memory_order_release);
                    continue;
                }

                // 确保是整除的
                // thread_num / 2 * thread_num
                int steal_cnt = std::min((ed - bg) / 2, 32);
                int steal_beg = bg;
                int steal_end = bg + steal_cnt;
                /**
                 * 一定要是release确保对于其他线程，这个修改可见；并且后续通过 读取这个变量时，必须使用acquire
                 * 在线程更新自己窃取到的任务的时候，应该确保自身的更新完成，即不会更新到一半然后被其他线程窃取，这样可能导致错误
                 */
                otherPool->phase_tasks_beg_first[i + part_idx * thread_num].store(steal_end, std::memory_order_release);

                if (steal_flag[thread_id].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    vec_graph[thread_id].start_idx = steal_beg;
                    // vec_graph[thread_id].end_idx = std::min(steal_end, steal_beg + steal_cnt);
                    vec_graph[thread_id].end_idx = steal_end;
                    // phase_tasks_beg[thread_id].store(vec_graph[thread_id].end_idx, std::memory_order_relaxed);
                    // phase_tasks_end[thread_id].store(steal_end, std::memory_order_relaxed);
                }

                flag = true;
                /**
                 * 当前线程窃取完成后，不允许其他线程窃取，避免频繁原子访问
                 */
                otherPool->steal_flag[i].store(false, std::memory_order_release);
                steal_flag[thread_id].store(false, std::memory_order_release);
                // break;
                return flag;
            }
        }
    }

    return flag;
}

bool ThreadPool::tryToStealSecond(int part_idx)
{
    bool flag = false;
    int i = 0;
    // std::uniform_int_distribution<int> dist(0, thread_num - 1);
    // int offset = dist(thread_rand_generator);
    int offset = tls_rng->next_u32(thread_num);
    for (int j = 0; j < thread_num; j++)
    {
        i = (j + offset) % thread_num;
        if (i == thread_id)
        {
            continue;
        }
        /**
         * 这部分是和全局压入任务范围同步
         */
        if (phase_tasks_end_second[i + part_idx * thread_num].load(std::memory_order_acquire) - phase_tasks_beg_second[i + part_idx * thread_num].load(std::memory_order_acquire) >= threshold)
        {
            // std::cout << "thread " << thread_id << " steal from thread " << i << std::endl;
            bool expected = false;
            if (steal_flag[i].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
            {
                int bg = phase_tasks_beg_second[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int ed = phase_tasks_end_second[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int total = ed - bg;
                if (total < threshold)
                {
                    // 当前已经被抢占获取了，恢复状态，并且立马去获取其他任务
                    steal_flag[i].store(false, std::memory_order_release);
                    continue;
                }

                // 确保是整除的
                // thread_num / 2 * thread_num
                int steal_cnt = std::min((ed - bg) / 2, 32);
                int steal_beg = bg;
                int steal_end = bg + steal_cnt;
                /**
                 * 一定要是release确保对于其他线程，这个修改可见；并且后续通过 读取这个变量时，必须使用acquire
                 * 在线程更新自己窃取到的任务的时候，应该确保自身的更新完成，即不会更新到一半然后被其他线程窃取，这样可能导致错误
                 */
                phase_tasks_beg_second[i + part_idx * thread_num].store(steal_end, std::memory_order_release);

                if (steal_flag[thread_id].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    vec_graph[thread_id].start_idx = steal_beg;
                    // vec_graph[thread_id].end_idx = std::min(steal_end, steal_beg + steal_cnt);
                    vec_graph[thread_id].end_idx = steal_end;
                    // phase_tasks_beg[thread_id].store(vec_graph[thread_id].end_idx, std::memory_order_relaxed);
                    // phase_tasks_end[thread_id].store(steal_end, std::memory_order_relaxed);
                }

                flag = true;
                /**
                 * 当前线程窃取完成后，不允许其他线程窃取，避免频繁原子访问
                 */
                steal_flag[i].store(false, std::memory_order_release);
                steal_flag[thread_id].store(false, std::memory_order_release);
                // break;
                return flag;
            }
        }
    }

    /**
     * 当前不存在可窃取任务，转而窃取另一个NUMA节点任务
     */
    for (int j = 0; j < thread_num; j++)
    {
        i = (j + offset) % thread_num;
        if (i == thread_id)
        {
            continue;
        }
        /**
         * 这部分是和全局压入任务范围同步
         */
        if (otherPool->phase_tasks_end_second[i + part_idx * thread_num].load(std::memory_order_acquire) - otherPool->phase_tasks_beg_second[i + part_idx * thread_num].load(std::memory_order_acquire) >= threshold)
        {
            // std::cout << "thread " << thread_id << " steal from thread " << i << std::endl;
            bool expected = false;
            if (otherPool->steal_flag[i].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
            {
                int bg = otherPool->phase_tasks_beg_second[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int ed = otherPool->phase_tasks_end_second[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int total = ed - bg;
                if (total < threshold)
                {
                    // 当前已经被抢占获取了，恢复状态，并且立马去获取其他任务
                    otherPool->steal_flag[i].store(false, std::memory_order_release);
                    continue;
                }

                // 确保是整除的
                // thread_num / 2 * thread_num
                int steal_cnt = std::min((ed - bg) / 2, 32);
                int steal_beg = bg;
                int steal_end = bg + steal_cnt;
                /**
                 * 一定要是release确保对于其他线程，这个修改可见；并且后续通过 读取这个变量时，必须使用acquire
                 * 在线程更新自己窃取到的任务的时候，应该确保自身的更新完成，即不会更新到一半然后被其他线程窃取，这样可能导致错误
                 */
                otherPool->phase_tasks_beg_second[i + part_idx * thread_num].store(steal_end, std::memory_order_release);

                if (steal_flag[thread_id].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    vec_graph[thread_id].start_idx = steal_beg;
                    // vec_graph[thread_id].end_idx = std::min(steal_end, steal_beg + steal_cnt);
                    vec_graph[thread_id].end_idx = steal_end;
                    // phase_tasks_beg[thread_id].store(vec_graph[thread_id].end_idx, std::memory_order_relaxed);
                    // phase_tasks_end[thread_id].store(steal_end, std::memory_order_relaxed);
                }

                flag = true;
                /**
                 * 当前线程窃取完成后，不允许其他线程窃取，避免频繁原子访问
                 */
                otherPool->steal_flag[i].store(false, std::memory_order_release);
                steal_flag[thread_id].store(false, std::memory_order_release);
                // break;
                return flag;
            }
        }
    }

    return flag;
}

void ThreadPool::wait()
{
    // std::unique_lock<std::mutex> lock(queue_mutex);
    // task_done_condition.wait(lock, [this]
    //                          { return tasks_pending == 0; });
    for (auto &worker : workers)
    {
        if (worker.joinable())
            worker.join();
    }
}

void ThreadPool::start()
{
    // edge_idx_old = 0;
    // thread_count = 0;
    // count_condition.notify_all();
    {
        std::unique_lock<std::mutex> lock(flag_mutex);
        flag = 2;
    }
    flag_condition.notify_all();
}

int ThreadPool::getCount()
{
    return finished;
}

/**
 * 这个可以设置一下，直接根据标识去判断当前窃取的任务是什么队列的
 * 然后根据标识去判断应该执行什么函数;
 */
int ThreadPool::tryToStealOld(int part_idx)
{
    int flag = -1;
    // std::uniform_int_distribution<int> dist(0, thread_num - 1);
    // int offset = dist(thread_rand_generator);
    int offset = tls_rng->next_u32(thread_num);
    int i = 0;
    for (int j = 0; j < thread_num; j++)
    {
        i = (j + offset) % thread_num;
        if (i == thread_id)
        {
            continue;
        }
        /**
         * 这部分是和全局压入任务范围同步
         */
        if (old_task_end[i + part_idx * thread_num].load(std::memory_order_acquire) - old_task_beg[i + part_idx * thread_num].load(std::memory_order_acquire) >= threshold)
        {
            // std::cout << "thread " << thread_id << " steal from thread " << i << std::endl;
            bool expected = false;
            if (steal_flag[i].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
            {
                int bg = old_task_beg[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int ed = old_task_end[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int total = ed - bg;
                if (total < threshold)
                {
                    // 当前已经被抢占获取了，恢复状态，并且立马去获取其他任务
                    steal_flag[i].store(false, std::memory_order_release);
                    continue;
                }

                // 确保是整除的
                // thread_num / 2 * thread_num
                int steal_cnt = std::min((ed - bg) / 2, 32);
                int steal_beg = bg;
                int steal_end = bg + steal_cnt;
                /**
                 * 一定要是release确保对于其他线程，这个修改可见；并且后续通过 读取这个变量时，必须使用acquire
                 * 在线程更新自己窃取到的任务的时候，应该确保自身的更新完成，即不会更新到一半然后被其他线程窃取，这样可能导致错误
                 */
                old_task_beg[i + part_idx * thread_num].store(steal_end, std::memory_order_release);

                int temp = old_flag[i + part_idx * thread_num].load(std::memory_order_acquire);
                if (steal_flag[thread_id].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    vec_graph[thread_id].start_idx = steal_beg;
                    // vec_graph[thread_id].end_idx = std::min(steal_end, steal_beg + steal_cnt);
                    vec_graph[thread_id].end_idx = steal_end;
                    // old_task_beg[thread_id].store(vec_graph[thread_id].end_idx, std::memory_order_relaxed);
                    // old_task_end[thread_id].store(steal_end, std::memory_order_relaxed);

                    old_flag[thread_id + part_idx * thread_num].store(temp, std::memory_order_release);
                }

                flag = temp;
                /**
                 * 当前线程窃取完成后，不允许其他线程窃取，避免频繁原子访问
                 */
                steal_flag[i].store(false, std::memory_order_release);
                steal_flag[thread_id].store(false, std::memory_order_release);
                // break;
                return flag;
            }
        }
    }

    /**
     * 当前不存在可窃取任务，转而窃取另一个NUMA节点任务
     */
    for (int j = 0; j < thread_num; j++)
    {
        i = (j + offset) % thread_num;
        if (i == thread_id)
        {
            continue;
        }
        /**
         * 这部分是和全局压入任务范围同步
         */
        if (otherPool->old_task_end[i + part_idx * thread_num].load(std::memory_order_acquire) - otherPool->old_task_beg[i + part_idx * thread_num].load(std::memory_order_acquire) >= threshold)
        {
            // std::cout << "thread " << thread_id << " steal from thread " << i << std::endl;
            bool expected = false;
            if (otherPool->steal_flag[i].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
            {
                int bg = otherPool->old_task_beg[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int ed = otherPool->old_task_end[i + part_idx * thread_num].load(std::memory_order_relaxed);
                int total = ed - bg;
                if (total < threshold)
                {
                    // 当前已经被抢占获取了，恢复状态，并且立马去获取其他任务
                    otherPool->steal_flag[i].store(false, std::memory_order_release);
                    continue;
                }

                // 确保是整除的
                // thread_num / 2 * thread_num
                int steal_cnt = std::min((ed - bg) / 2, 64);
                int steal_beg = bg;
                int steal_end = bg + steal_cnt;
                /**
                 * 一定要是release确保对于其他线程，这个修改可见；并且后续通过 读取这个变量时，必须使用acquire
                 * 在线程更新自己窃取到的任务的时候，应该确保自身的更新完成，即不会更新到一半然后被其他线程窃取，这样可能导致错误
                 */
                otherPool->old_task_beg[i + part_idx * thread_num].store(steal_end, std::memory_order_release);

                int temp = otherPool->old_flag[i + part_idx * thread_num].load(std::memory_order_acquire);
                if (steal_flag[thread_id].compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    vec_graph[thread_id].start_idx = steal_beg;
                    // vec_graph[thread_id].end_idx = std::min(steal_end, steal_beg + steal_cnt);
                    vec_graph[thread_id].end_idx = steal_end;
                    // old_task_beg[thread_id].store(vec_graph[thread_id].end_idx, std::memory_order_relaxed);
                    // old_task_end[thread_id].store(steal_end, std::memory_order_relaxed);
                    old_flag[thread_id + part_idx * thread_num].store(temp, std::memory_order_release);
                    // old_steal.fetch_add(steal_end - steal_beg);
                }

                /**
                 * 拉取远程numa的数据到本地
                 */
                int temp_index = steal_beg;
                for (int localIdx = steal_beg; localIdx < steal_end; localIdx++)
                {
                    vec_graph[thread_id].other_tasks[localIdx - temp_index] = std::move(otherPool->q->at(part_idx).at(localIdx));
                }

                flag = temp;
                /**
                 * 当前线程窃取完成后，不允许其他线程窃取，避免频繁原子访问
                 */
                otherPool->steal_flag[i].store(false, std::memory_order_release);
                steal_flag[thread_id].store(false, std::memory_order_release);
                // break;
                return flag;
            }
        }
    }

    return flag;
}

/**
 * 初始化，将部分全局变量进行构造
 */
void ThreadPool::init(std::vector<std::vector<int>> *temp_res, std::vector<std::pair<int, int>> *p_s, int block_size, int j, DataGraph &g, DataGraph &h, MatchCriteria &_criteria, int limit, int delta, std::pair<int, int> *p, std::vector<std::mutex> *mtx, std::mutex *res_mutex, LockFreeQueue<GraphMatch> *results, std::vector<LockFreeQueue<SearchData>> *q, std::vector<int> *all_edge_idxs, ThreadPool *pool)
{
    this->temp_res = temp_res;
    this->p_s = p_s;
    this->block_size = block_size;
    this->g_i = j;
    this->_g = &g;
    this->_h = &h;
    this->_criteria = &_criteria;
    this->limit = limit;
    this->_delta = delta;
    this->p = p;
    this->mtx = mtx;
    this->res_mutex = res_mutex;
    this->res = results;
    this->q = q;
    this->all_edge_idxs = all_edge_idxs;
    this->pool = pool;

    for (int i = 0; i < vec_graph.size(); i++)
    {
        vec_graph[i] = GraphSearch(temp_res, p_s, block_size, i, *_g, *_h, _criteria, limit, _delta, p, mtx, res_mutex, res, q, all_edge_idxs, this);
        vec_graph[i].vec_idx = i;
        vec_graph[i].minfos = minfos;
        vec_graph[i].in_part_col_idx = in_part_col_idx;
        vec_graph[i].in_part_values = in_part_values;
        vec_graph[i].in_part_row_ptr = in_part_row_ptr;
        vec_graph[i].out_part_col_idx = out_part_col_idx;
        vec_graph[i].out_part_values = out_part_values;
        vec_graph[i].out_part_row_ptr = out_part_row_ptr;
        vec_graph[i].in_range = in_range;
        vec_graph[i].out_range = out_range;
        vec_graph[i].in_block_map = in_block_map;
        vec_graph[i].out_block_map = out_block_map;
        for (int j = 0; j < 5; j++)
        {
            vec_graph[i].nodes[j] = nodes[j];
        }
        vec_graph[i].csr = csr;
    }

    edge_idx_old = new int[p_s->size()];
    edge_idx_new = new int[p_s->size()];

    for (int i = 0; i < p_s->size(); i++)
    {
        edge_idx_old[i] = 0;
        edge_idx_new[i] = (p_s->at(i).first);
    }

    thread_count = new AlignedAtomic<int>[p_s->size()];
    thread_count_first = new AlignedAtomic<int>[p_s->size()];
    thread_count_second = new AlignedAtomic<int>[p_s->size()];

    for (int i = 0; i < p_s->size(); i++)
    {
        thread_count[i].store(0);
        thread_count_first[i].store(0);
        thread_count_second[i].store(0);
    }

    old_task_beg = new AlignedAtomic<int>[thread_num * p_s->size()];
    old_task_end = new AlignedAtomic<int>[thread_num * p_s->size()];
    old_flag = new AlignedAtomic<int>[thread_num * p_s->size()];

    for (int i = 0; i < thread_num * p_s->size(); i++)
    {
        old_task_beg[i].store(0);
        old_task_end[i].store(0);
        old_flag[i].store(numa_id);
    }

    std::cout << "线程数量" << thread_num << std::endl;
    phase_tasks_beg = new AlignedAtomic<int>[thread_num * p_s->size()];
    phase_tasks_end = new AlignedAtomic<int>[thread_num * p_s->size()];
    phase_tasks_beg_first = new AlignedAtomic<int>[thread_num * p_s->size()];
    phase_tasks_beg_second = new AlignedAtomic<int>[thread_num * p_s->size()];
    phase_tasks_end_first = new AlignedAtomic<int>[thread_num * p_s->size()];
    phase_tasks_end_second = new AlignedAtomic<int>[thread_num * p_s->size()];

    for (int i = 0; i < p_s->size(); i++)
    {
        int start = p_s->at(i).first;
        int end = p_s->at(i).second;
        int total = end - start;
        int total_threads = thread_num * 2;
        int base_chunk = total / total_threads;
        int remainder = total % total_threads;

        std::vector<std::pair<int, int>> ranges(total_threads);
        int curr = start;
        for (int t = 0; t < total_threads; ++t)
        {
            int sz = base_chunk + (t < remainder ? 1 : 0);
            ranges[t] = {curr, curr + sz};
            curr += sz;
        }

        for (int j = 0; j < thread_num; ++j)
        {
            int k = i * thread_num + j;
            if (numa_id == 0)
            {
                int t = j * 2;
                phase_tasks_beg[k].store(ranges[t].first);
                phase_tasks_end[k].store(ranges[t].second);
            }
            else
            {
                int t = j * 2 + 1;
                phase_tasks_beg[k].store(ranges[t].first);
                phase_tasks_end[k].store(ranges[t].second);
            }
        }
    }

    for (int i = 0; i < p_s->size() * thread_num; i++)
    {
        phase_tasks_beg_first[i].store(phase_tasks_beg[i].load());
        phase_tasks_end_first[i].store(phase_tasks_beg[i].load() + (phase_tasks_end[i].load() - phase_tasks_beg[i].load()) / 2);
        phase_tasks_beg_second[i].store(phase_tasks_end_first[i].load());
        phase_tasks_end_second[i].store(phase_tasks_end[i].load());
    }

    steal_flag = new AlignedAtomic<bool>[thread_num];
    for (int i = 0; i < thread_num; i++)
    {
        steal_flag[i].store(false);
    }

    group_finish = new AlignedAtomic<int>[group_size * p_s->size()];
    for (int i = 0; i < group_size * p_s->size(); i++)
    {
        group_finish[i].store(0);
    }

    re_distribute = new AlignedAtomic<bool>[thread_num];
    distribute = new AlignedAtomic<bool>[thread_num];
    for (int i = 0; i < thread_num; i++)
    {
        re_distribute[i].store(false);
        distribute[i].store(false);
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop = true;
    }
    // for (auto &td_ptr : thread_data_vector)
    // {
    //     td_ptr->cond.notify_all();
    // }
    // wait();
    stop = true;
    count_condition.notify_all();
    flag = 1;
    flag_condition.notify_all();
    for (auto &worker : workers)
    {
        if (worker.joinable())
            worker.join();
    }

    delete[] group_finish;
    delete[] edge_idx_new;
    delete[] edge_idx_old;
    delete[] thread_count;
    delete[] phase_tasks_beg;
    delete[] phase_tasks_end;
    delete[] steal_flag;
    delete[] re_distribute;
    delete[] distribute;
    delete[] phase_tasks_beg_first;
    delete[] phase_tasks_beg_second;
    delete[] phase_tasks_end_first;
    delete[] phase_tasks_end_second;
    delete[] thread_count_first;
    delete[] thread_count_second;
    delete[] old_flag;
    delete[] old_task_beg;
    delete[] old_task_end;
}