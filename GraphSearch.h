/*
 * 文件: GraphSearch.h
 * 作者: D3M430
 *
 * 创建时间: 2017年1月13日 下午3:45
 */

#ifndef GRAPHSEARCH_H
#define GRAPHSEARCH_H

#include "Graph.h"
#include "MatchCriteria.h"
#include <limits.h>
#include <vector>
#include <stack>
#include <unordered_set>
#include "GraphMatch.h"
#include <mutex>
#include <queue>
#include <utility>
#include <atomic>
#include "DataGraph.h"
#include "SearchData.h"
#include "ManualStack.h"
#include "LockFreeQueue.h"
#include "SegMap.h"
#include "BinaryRange.h"
#include "Minfo.h"
#include "FlatHashMap.h"
#include "AlignedAtomic.h"
#include "Csr.h"

class ThreadPool;

// template <typename T>
// class LockFreeQueue;

/**
 * 主要用于执行子图搜索的类。
 */
class GraphSearch
{
public:
    // 无参构造
    GraphSearch() {}

    // 初始构造
    GraphSearch(std::vector<std::vector<int>> *temp_res, std::vector<std::pair<int, int>> *p_s, int block_size, int j, DataGraph &g, DataGraph &h, MatchCriteria &_criteria, int limit = INT_MAX, int delta = INT_MAX, std::pair<int, int> *p = nullptr, std::vector<std::mutex> *mtx = nullptr, std::mutex *res_mutex = nullptr, LockFreeQueue<GraphMatch> *results = nullptr, std::vector<LockFreeQueue<SearchData>> *q = nullptr, std::vector<int> *all_edge_idxs = nullptr, ThreadPool *pool = nullptr)
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
        // this->mtx = mtx;
        this->res_mutex = res_mutex;
        this->res = results;
        this->q = q;
        this->all_edge_idxs = all_edge_idxs;
        this->pool = pool;
    }

    /**
     * 执行子图搜索，要求查询图和原始图之间的边顺序必须匹配。
     * 如果在查询图中 a -> b 在 b -> c 之前，则在原始图中也必须如此。
     * 返回所有匹配的子图。返回的数据结构是一个包含所有匹配子图的列表，
     * 子图以 g 中的边索引表示。如果为空，则表示未找到匹配。
     * @param g  要搜索的有向图。
     * @param h  要匹配的有向查询图。
     * @param limit  要查找的最大子图数量。
     * @param delta  匹配边之间允许的最大时间间隔。
     * @return  匹配 h 的子图列表。
     */
    // std::vector<GraphMatch> findOrderedSubgraphs(const Graph &g, const Graph &h, int limit = INT_MAX, int delta = INT_MAX);

    /**
     * 执行子图搜索，要求查询图和原始图之间的边顺序必须匹配。
     * 如果在查询图中 a -> b 在 b -> c 之前，则在原始图中也必须如此。
     * 返回所有匹配的子图。返回的数据结构是一个包含所有匹配子图的列表，
     * 子图以 g 中的边索引表示。如果为空，则表示未找到匹配。
     * @param g  要搜索的有向图。
     * @param h  要匹配的有向查询图。
     * @param criteria  多态类，指定两条边是否符合查询条件。
     * @param limit  要查找的最大子图数量。
     * @param delta  匹配边之间允许的最大时间间隔。
     * @return  匹配 h 的子图列表。
     */
    void findOrderedSubgraphs(LockFreeQueue<SearchData> *task_queue, int *edge_idx, std::mutex *edge_mutex, int thread_id, int thread_num, LockFreeQueue<SearchData> *other_task_queue);

    /**
     * 处理新任务的查找
     * @param edge_idx  块起始边索引
     */
    void findNewOrderedSubgraphs(int *edge_idx, std::mutex *edge_mtx, int thread_id, int thread_num, AlignedAtomic<int> *phase_tasks_beg, AlignedAtomic<int> *phase_tasks_end);

    /**
     * 处理旧任务的窃取，这部分需要转化为队列
     */
    void findOthgerOrderedSubgraphs(LockFreeQueue<SearchData> *task_queue, int *edge_idx, std::mutex *edge_mutex, int thread_id, int thread_num, LockFreeQueue<SearchData> *other_task_queue);

    /**
     * 线程对于窃取到的任务进行处理
     */
    void stealOrderedSubgraphs(int *edge_idx, std::mutex *edge_mtx, int thread_id, int thread_num, AlignedAtomic<int> *phase_tasks_beg, AlignedAtomic<int> *phase_tasks_end);

    /**
     * 执行子图搜索，边的顺序无关紧要，通常忽略事件的时间/日期。
     * 返回的数据结构是一个包含所有匹配子图的列表，
     * 子图以 g 中的边索引表示。如果为空，则表示未找到匹配。
     * @param g  要搜索的有向图。
     * @param h  要匹配的有向查询图。
     * @param limit  要查找的最大子图数量。
     * @return  匹配 h 的子图列表。
     */
    // std::vector<GraphMatch> findAllSubgraphs(const Graph &g, const Graph &h, int limit = INT_MAX);

    /**
     * 执行子图搜索，边的顺序无关紧要，通常忽略事件的时间/日期。
     * 返回的数据结构是一个包含所有匹配子图的列表，
     * 子图以 g 中的边索引表示。如果为空，则表示未找到匹配。
     * @param g  要搜索的有向图。
     * @param h  要匹配的有向查询图。
     * @param criteria  多态类，指定两条边是否符合查询条件。
     * @param limit  要查找的最大子图数量。
     * @return  匹配 h 的子图列表。
     */
    // std::vector<GraphMatch> findAllSubgraphs(const Graph &g, const Graph &h, const MatchCriteria &criteria, int limit = INT_MAX);

    // std::vector<int> edge_idxs;

public:
    /** 创建一个映射，表示 G 中哪些节点可以映射到 H 中的节点 */
    std::vector<std::unordered_set<int>> mapPossibleNodes();

    /** 执行递归的无序图搜索，找到第一个匹配的子图后停止 */
    bool search(int &numAssigned, std::vector<std::unordered_set<int>> &h2gPossible);

    /** 执行递归的无序图搜索，将每个匹配的子图存储到结果中 */
    bool search(int &numAssigned, std::vector<std::unordered_set<int>> &h2gPossible, std::vector<GraphMatch> &results);

    /** 如果已分配的数量与相应的边匹配，则返回 true */
    bool matchesSoFar(int numAssigned);

    /** 在搜索前选择一个高效的边索引列表，
     * 查找与查询边 h_i 匹配的边。返回值是 G 中匹配边的索引。
     * 如果未找到匹配边，则返回 G 的边数。 */
    void findNextMatch(int h_i, int startIndex, std::vector<int> *all_edge_idxs, Graph &g);

    /** 在 edgesToSearch 列表中（从 startIndex 开始）搜索，
     * 查找与查询边 h_i 匹配的边。返回值是 G 中匹配边的索引。
     * 如果未找到匹配边，则返回 G 的边数。 */
    bool findNextMatch(int h_i, int g_idx);

    /**
     * 执行二分查找以找到最佳起始位置。
     * @param g_i  要查找的边索引（或更大的索引）。
     * @param edgeIndexes  要搜索的列表。
     * @return   列表中满足条件的边索引（>= g_i）。
     */
    int findStart(int g_i, const std::vector<int> &edgeIndexes, std::vector<int> &all_edge, BinaryRange *binary_range, int bg, int ed, int vOffset, int vertex, int now_Offset);

    /**
     * 执行二分搜索找到满足时间约束的最后位置
     */
    int findEnd(int g_i, const std::vector<int> &edgeIndexes, Graph &g, BinaryRange *binary_range, int bg, int ed);

    /** 将给定的栈转换为向量，不修改原栈 */
    std::vector<int> convert(std::stack<int> s);

    /** 将给定的栈和最后一条边转换为 GraphMatch 对象，不修改原栈 */
    GraphMatch convert(const std::stack<int> &s, int g_lastEdge);

    /**
     * 获取对应对象
     */
    SearchData getData();

    /**
     * 根据Data设置对应值
     * 为了避免子任务窃取才使用拷贝构造
     */
    void setData(SearchData &&data);

    /**
     * 直接移动构造,减少时间消耗
     * 从任务队列中获取元素采用移动构造
     */
    void setData(SearchData &&data, bool move);

    void setData(const SearchData &data);

    // GraphSearch(const GraphSearch &) = delete;
    // GraphSearch &operator=(const GraphSearch &) = delete;
    // GraphSearch(GraphSearch &&) = delete;
    // GraphSearch &operator=(GraphSearch &&) = delete;

    // 私有数据成员
    DataGraph *_g, *_h;       // 原始图和查询图
    MatchCriteria *_criteria; // 匹配条件
    int _delta;               // 边匹配允许的最大时间间隔

    time_t _firstEdgeTime; // 第一条匹配边的时间
    // std::vector<int> _h2gNodes;

    int _h2gNodes[5];

    std::pair<int, int> candidates[5];

    // std::vector<int> _g2hNodes; // 节点映射表
    // std::vector<int> _numSearchEdgesForNode; // 每个节点的搜索边计数
    // std::vector<std::pair<int, int>> map;
    // std::stack<int> _sg_edgeStack; // 存储搜索路径中的边索引，回溯需要
    // ManualStack<int> _sg_edgeStack;
    // std::vector<int> _allEdges; // 所有边的索引列表
    // std::vector<std::pair<int, int>> candidates; // 当前层的候选边列表，边数量-1
    // 存储motif的开始匹配点
    int h_i = 0;
    // 存储图边的当前匹配点
    int g_i = -1;
    // 存储当前层级的搜索数组
    const std::vector<int> *arr[5] = {nullptr};
    
    // 存储这几个层级数组的方向，0为in_csr; 1为out_csr
    bool direct[5] = {0};
    // const std::vector<int> *brr = nullptr;
    // 存储压入的边索引
    // std::vector<int> temp_e;

    // 表示当前数组和范围的层级
    // int level = 0;
    // 存储当前层级的搜索范围，区间为[beg, end]
    int beg = -1;
    int end = -1;
    std::pair<int, int> *p = nullptr;
    int limit = INT_MAX;
    // std::vector<std::mutex> *mtx = nullptr;
    std::mutex *res_mutex = nullptr;
    // std::vector<GraphMatch> *res = nullptr;
    LockFreeQueue<GraphMatch> *res = nullptr;
    std::vector<LockFreeQueue<SearchData>> *q = nullptr;
    std::vector<int> *all_edge_idxs = nullptr;
    ThreadPool *pool = nullptr;
    // std::vector<queue<GraphSearch>>* queues = nullptr;
    // std::vector<mutex>* mutexes = nullptr;
    int block_size = 1;
    // 是否采用二分搜索
    bool binary_search = true;
    // 是否二分左边界
    bool binary_left = true;

    std::vector<std::pair<int, int>> *p_s = nullptr;
    std::vector<std::vector<int>> *temp_res = nullptr;
    // 记录数组的索引
    int vec_idx = 0;

    long long res_num = 0;

    int part_idx;

    // std::vector<std::vector<BinaryRange>> *in_binary_range;
    // std::vector<std::vector<BinaryRange>> *out_binary_range;

    BinaryRange *in_binary_range;
    BinaryRange *out_binary_range;
    std::vector<Minfo> *minfos;

    FlatHashMap *in_block_map;
    FlatHashMap *out_block_map;

    std::vector<int> *in_part_values;
    std::vector<int> *in_part_col_idx;
    std::vector<int> *in_part_row_ptr;
    std::pair<int, int> *in_range;
    std::vector<int> *out_part_values;
    std::vector<int> *out_part_col_idx;
    std::pair<int, int> *out_range;
    std::vector<int> *out_part_row_ptr;
    int nodes[5];
    Minfo temp;
    int start_idx = 0;
    int end_idx = 0;
    int all_thread_nums = 64;

    int expanding_vertex[5] = {0};

    Csr *csr;

    /**
     * 存储其他numa窃取到得任务
     */
    SearchData other_tasks[64];

    /**
     * 分别表示二分搜索准备时间，二分搜索的时间，检查时间，传输时间
     */
    // uint64_t binary_prepare{0};
    // uint64_t binary_search_time{0};
    // uint64_t check_time{0};
    // uint64_t trans_time{0};
};

#endif /* GRAPHSEARCH_H */