#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>
#include <limits>
#include <deque>
#include <deque>
#include "GraphSearch.h"
#include "LockFreeQueue.h"
#include "MemoryPool.h"
#include <chrono>
#include "BinaryRange.h"
#include "Minfo.h"
#include "NumaTool.h"
#include "AlignedAtomic.h"
#include "FlatHashMap.h"
#include "Csr.h"

// 放到 ThreadPool.h 顶部

class ThreadPool
{
    struct ThreadData
    {
        std::deque<GraphSearch> queue;
        std::mutex mutex;
        std::condition_variable cond;
        std::atomic<size_t> task_count{0};
    };

public:
    ThreadPool(size_t threads, int numa_node);
    // void enqueueGraphSearch(GraphSearch &task);
    // template <class F, class... Args>
    // auto enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>;
    bool tryToSteal(int part_idx);
    bool tryToStealFirst(int part_idx);
    bool tryToStealSecond(int part_idx);
    void wait();
    void setOld(int id);
    void setNew(int id);
    void start();
    int getCount();
    int tryToStealOld(int part_idx);
    void init(std::vector<std::vector<int>> *temp_res, std::vector<std::pair<int, int>> *p_s, int block_size, int j, DataGraph &g, DataGraph &h, MatchCriteria &_criteria, int limit = INT_MAX, int delta = INT_MAX, std::pair<int, int> *p = nullptr, std::vector<std::mutex> *mtx = nullptr, std::mutex *res_mutex = nullptr, LockFreeQueue<GraphMatch> *results = nullptr, std::vector<LockFreeQueue<SearchData>> *q = nullptr, std::vector<int> *all_edge_idxs = nullptr, ThreadPool *pool = nullptr);
    ~ThreadPool();
    static thread_local bool is_worker;
    static thread_local int thread_id;

public:
    std::vector<std::unique_ptr<ThreadData>> thread_data_vector;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable task_done_condition;
    std::atomic<size_t> tasks_pending{0};
    std::atomic<bool> stop{false};
    int threshold;
    int threshold_new;
    std::mutex count_mtx;

    AlignedAtomic<int> *thread_count = nullptr;

    std::condition_variable count_condition;
    std::mutex flag_mutex;
    std::condition_variable flag_condition;
    int flag{0};
    std::atomic<int> finished{0};
    int thread_num{0};
    std::atomic<long long> res_num{0};
    long long int max_time{0};
    long long int binary_t{0};

    AlignedAtomic<int> *phase_tasks = nullptr;
    AlignedAtomic<int> *phase_tasks_beg = nullptr;
    AlignedAtomic<int> *phase_tasks_end = nullptr;
    AlignedAtomic<bool> *steal_flag = nullptr;
    AlignedAtomic<int> *group_finish = nullptr;

    int group_size;

    AlignedAtomic<bool> *re_distribute = nullptr;
    AlignedAtomic<bool> *distribute = nullptr;

    BlockPool<SearchData> *pool_memory;
    int temp_idx;
    BinaryRange *in_binary_range;
    BinaryRange *out_binary_range;
    std::vector<Minfo> *minfos;
    int start_part = 0;
    int end_part = 0;
    int new_end = 0;
    ThreadPool *otherPool = nullptr;
    std::vector<int> *in_part_values;
    std::vector<int> *in_part_col_idx;
    std::vector<int> *in_part_row_ptr;
    std::pair<int, int> *in_range;
    std::vector<int> *out_part_values;
    std::vector<int> *out_part_col_idx;
    std::pair<int, int> *out_range;
    std::vector<int> *out_part_row_ptr;
    FlatHashMap *in_block_map;
    FlatHashMap *out_block_map;
    int *nodes = nullptr;
    void *handle = nullptr;

    AlignedAtomic<int> *phase_tasks_beg_first = nullptr;
    AlignedAtomic<int> *phase_tasks_end_first = nullptr;
    AlignedAtomic<int> *phase_tasks_beg_second = nullptr;
    AlignedAtomic<int> *phase_tasks_end_second = nullptr;

    AlignedAtomic<int> *thread_count_first = nullptr;
    AlignedAtomic<int> *thread_count_second = nullptr;

    std::vector<GraphSearch> vec_graph;

    DataGraph *_g, *_h;
    MatchCriteria *_criteria;
    int _delta;
    time_t _firstEdgeTime;
    std::vector<int> _h2gNodes;
    std::vector<int> _numSearchEdgesForNode;
    std::vector<std::pair<int, int>> map;
    std::stack<int> _sg_edgeStack;
    int h_i = 0;
    int g_i = -1;
    int beg = -1;
    int end = -1;
    std::vector<int> temp_e;
    int level = 0;

    std::pair<int, int> *p = nullptr;
    int limit = INT_MAX;
    std::vector<std::mutex> *mtx = nullptr;
    std::mutex *res_mutex = nullptr;
    LockFreeQueue<GraphMatch> *res = nullptr;
    std::vector<LockFreeQueue<SearchData>> *q = nullptr;
    std::vector<int> *all_edge_idxs = nullptr;
    ThreadPool *pool = nullptr;
    int block_size = 1;
    bool binary_search = true;
    bool binary_left = true;

    std::vector<std::pair<int, int>> *p_s = nullptr;
    std::vector<std::vector<int>> *temp_res = nullptr;

    int *edge_idx_old = nullptr;
    std::mutex edge_mutex_old;

    int *edge_idx_new = nullptr;
    std::mutex edge_mutex_new;

    int part_idx;
    std::atomic<int> old_edge_idx{0};

    AlignedAtomic<int> *old_task_beg;
    AlignedAtomic<int> *old_task_end;
    AlignedAtomic<int> *old_flag;

    std::atomic<int> old_steal{0};
    std::atomic<int> old_process{0};
    int numa_id;
    int task_nums = 1;
    int all_thread_nums = 64;
    std::atomic<int> now_idx{0};

    Csr *csr;

    std::atomic<uint64_t> binary_prepare{0};
    std::atomic<uint64_t> binary_search_time{0};
    std::atomic<uint64_t> check_time{0};
    std::atomic<uint64_t> trans_time{0};
};

#endif // THREAD_POOL_H