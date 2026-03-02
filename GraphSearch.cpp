#include <limits.h>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <iostream>
#include "GraphSearch.h"
#include "Graph.h"
#include "Edge.h"
#include <limits.h>
#include <mutex>
#include <vector>
#include <queue>
#include <memory>
#include "DataGraph.h"
#include "ThreadPool.h"
#include "LockFreeQueue.h"
#include "HighResClock.h"

using namespace std;

// extern thread_local long long int times;

/**
 * @brief 查找所有匹配子图（无限制条件）。
 * 如果没有指定匹配条件，则使用默认的“接受一切”条件。
 */
// vector<GraphMatch> GraphSearch::findAllSubgraphs(const Graph &g, const Graph &h, int limit)
// {
//     MatchCriteria criteria; // 默认匹配条件
//     return this->findAllSubgraphs(g, h, criteria, limit);
// }

/**
 * @brief 查找所有匹配子图（带匹配条件）。
 * @param g 原始图。
 * @param h 查询图。
 * @param criteria 匹配条件。
 * @param limit 最大匹配数量限制。
 * @return 包含所有匹配子图的列表。
 */
// vector<GraphMatch> GraphSearch::findAllSubgraphs(const Graph &g, const Graph &h, const MatchCriteria &criteria, int limit)
// {
//     _g = &g; // 存储原始图
//     _h = &h; // 存储查询图
//     _criteria = &criteria; // 存储匹配条件

//     bool debugOutput = false; // 调试输出开关

//     vector<GraphMatch> results; // 存储匹配结果
//     int n = _g->numNodes(); // 原始图节点数
//     int m = _g->numEdges(); // 原始图边数

//     // 创建可能的节点映射列表
//     vector<unordered_set<int>> h2gPossible = this->mapPossibleNodes();

//     // 初始化节点映射表
//     _h2gNodes.clear();
//     _h2gNodes.resize(h.numNodes(), -1); // 查询图节点到原始图节点的映射
//     _g2hNodes.clear();
//     _g2hNodes.resize(n, -1); // 原始图节点到查询图节点的映射

//     // 执行子图搜索
//     int numAssigned = 0; // 当前已分配的节点数
//     this->search(numAssigned, h2gPossible, results);

//     return results;
// }

/**
 * @brief 创建查询图节点到原始图节点的可能映射。
 * @return 包含可能映射的列表。
 */
vector<unordered_set<int>> GraphSearch::mapPossibleNodes()
{
    vector<unordered_set<int>> possible(_h->numNodes()); // 存储可能的节点映射
    int n = _g->numNodes();                              // 原始图节点数

    // 遍历查询图的每个节点
    for (int h_v = 0; h_v < _h->numNodes(); h_v++)
    {
        // 遍历原始图的每个节点，检查是否满足匹配条件
        for (int g_v = 0; g_v < n; g_v++)
        {
            if (_criteria->isNodeMatch(*_g, g_v, *_h, h_v))
            {
                possible[h_v].insert(g_v); // 添加到可能映射列表
            }
        }
    }
    return possible;
}

/**
 * @brief 递归搜索匹配的子图。
 * @param numAssigned 当前已分配的节点数。
 * @param h2gPossible 可能的节点映射列表。
 * @return 如果找到匹配则返回 true，否则返回 false。
 */
bool GraphSearch::search(int &numAssigned, vector<unordered_set<int>> &h2gPossible)
{
    // if (!matchesSoFar(numAssigned)) // 检查当前节点/边是否匹配
    //     return false;

    // if (numAssigned == _h->numNodes()) // 如果所有节点都已分配，表示找到匹配
    //     return true;

    // // 对当前节点尝试所有可能的映射
    // int h_v = numAssigned;
    // const auto &possible = h2gPossible[h_v];
    // for (int g_v : possible)
    // {
    //     if (_g2hNodes[g_v] < 0) // 如果该节点尚未被映射
    //     {
    //         _h2gNodes[h_v] = g_v; // 更新映射表
    //         _g2hNodes[g_v] = h_v;
    //         numAssigned++;
    //         if (search(numAssigned, h2gPossible)) // 递归搜索
    //             return true;
    //         _g2hNodes[g_v] = -1; // 回溯
    //         _h2gNodes[h_v] = -1;
    //         numAssigned--;
    //     }
    // }

    return false;
}

/**
 * @brief 递归搜索匹配的子图，并将结果存储到结果列表中。
 * @param numAssigned 当前已分配的节点数。
 * @param h2gPossible 可能的节点映射列表。
 * @param results 存储匹配结果的列表。
 * @return 如果找到匹配则返回 true，否则返回 false。
 */
bool GraphSearch::search(int &numAssigned, vector<unordered_set<int>> &h2gPossible, vector<GraphMatch> &results)
{
    // if (!matchesSoFar(numAssigned)) // 检查当前节点/边是否匹配
    //     return false;

    // if (numAssigned == _h->numNodes()) // 如果所有节点都已分配，表示找到匹配
    // {
    //     GraphMatch matchingEdges; // 创建匹配对象
    //     for (const Edge &hEdge : _h->edges())
    //     {
    //         int h_u = hEdge.source();
    //         int h_v = hEdge.dest();
    //         int g_u = _h2gNodes[h_u];
    //         int g_v = _h2gNodes[h_v];

    //         const vector<int> &gEdges = _g->getEdgeIndexes(g_u, g_v);
    //         for (int g_e : gEdges)
    //         {
    //             if (_criteria->isEdgeMatch(*_g, g_e, *_h, hEdge.index()))
    //             {
    //                 matchingEdges.addEdge(_g->edges()[g_e], hEdge); // 添加匹配边
    //             }
    //         }
    //     }
    //     results.push_back(matchingEdges); // 将匹配结果添加到列表
    //     return true;d
    // }

    // // 对当前节点尝试所有可能的映射
    // int h_v = numAssigned;
    // const auto &possible = h2gPossible[h_v];
    // for (int g_v : possible)
    // {
    //     if (_g2hNodes[g_v] < 0) // 如果该节点尚未被映射
    //     {
    //         _h2gNodes[h_v] = g_v; // 更新映射表
    //         _g2hNodes[g_v] = h_v;
    //         numAssigned++;
    //         search(numAssigned, h2gPossible, results); // 递归搜索
    //         _g2hNodes[g_v] = -1;                       // 回溯
    //         _h2gNodes[h_v] = -1;
    //         numAssigned--;
    //     }
    // }

    return false;
}

/**
 * @brief 检查当前已分配的节点和边是否匹配。
 * @param numAssigned 当前已分配的节点数。
 * @return 如果匹配则返回 true，否则返回 false。
 */
bool GraphSearch::matchesSoFar(int numAssigned)
{
    for (const Edge &edge : _h->edges())
    {
        int h_u = edge.source();
        int h_v = edge.dest();
        if (h_u < numAssigned && h_v < numAssigned)
        {
            int g_u = _h2gNodes[h_u];
            int g_v = _h2gNodes[h_v];

            bool hasEdge = _g->hasEdge(g_u, g_v);
            if (hasEdge)
            {
                const vector<int> edges = _g->getEdgeIndexes(g_u, g_v);
                for (int e : edges)
                {
                    if (_criteria->isEdgeMatch(*_g, e, *_h, edge.index()))
                    {
                        hasEdge = true;
                        break;
                    }
                }
            }
            if (!hasEdge)
                return false;
        }
    }
    return true;
}

/**
 * @brief 查找有序子图（无限制条件）。
 * 如果没有指定匹配条件，则使用默认的“接受一切”条件。
 */
// void GraphSearch::findOrderedSubgraphs(const Graph &g, const Graph &h, int limit, int delta)
// {
//     MatchCriteria criteria; // 默认匹配条件
//     return findOrderedSubgraphs(g, h, criteria, limit, delta);
// }

/**
 * 在执行过程中每层边发生变化的时候都重新执行二分搜索去修改对应层的候选边范围
 * 修改部分内容即可
 */
void GraphSearch::findOrderedSubgraphs(LockFreeQueue<SearchData> *task_queue, int *edge_idx, std::mutex *edge_mutex, int thread_id, int thread_num, LockFreeQueue<SearchData> *other_task_queue)
{
    // std::cout << task_queue->size() << std::endl;
    int m = _h->numEdges();
    int n = _h->numNodes();
    // int idx = -1;
    int size = task_queue->size();
    int nums = 32;
    // int bg = size / thread_num * thread_id;
    // int ed = bg + size / thread_num;
    // if (size % thread_num > thread_id)
    // {
    //     bg += thread_id;
    //     ed += thread_id + 1;
    // }
    // else
    // {
    //     bg += size % thread_num;
    //     ed += size % thread_num;
    // }
    // idx = bg;

    // std::cout << "旧任务thread " << thread_id << " start from " << bg << " to " << ed << std::endl;

    while (true)
    {

        if (start_idx >= end_idx)
        {
            while (true)
            {
                bool excepted = false;
                if (pool->steal_flag[thread_id].compare_exchange_strong(excepted, true, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    /**
                     * 一次性获取nums个任务
                     */
                    start_idx = pool->old_task_beg[thread_id + part_idx * thread_num].fetch_add(nums, std::memory_order_release);
                    end_idx = std::min(int(pool->old_task_end[thread_id + part_idx * thread_num].load(std::memory_order_relaxed)), start_idx + nums);
                    if (start_idx >= end_idx)
                    {
                        pool->steal_flag[thread_id].store(false, std::memory_order_release);
                        return;
                    }
                    pool->steal_flag[thread_id].store(false, std::memory_order_release);
                    break;
                }
            }
        }

        // while (idx < ed)
        for (; start_idx < end_idx; start_idx++)
        {
            // int idx = -1;

            // {
            //     std::unique_lock<std::mutex> lock(*edge_mutex);
            //     if ((*edge_idx) >= task_queue->size())
            //     {
            //         return;
            //     }
            //     *this = task_queue->at(*edge_idx);
            //     binary_search = true;
            //     binary_left = false;
            //     // std::cout << "开始执行旧边" << (*edge_idx) << " " << task_queue->size() << std::endl;
            //     // std::cout << block_size << " " << p -> first << " " << p -> second << std::endl;
            //     (*edge_idx)++;
            // }

            {
                // 先计算当前要获取的任务索引
                // if (idx == -1)
                // {
                //     idx = thread_id;
                // }
                // else
                // {
                //     idx += thread_num;
                // }

                // if (idx >= task_queue->size())
                // {
                //     return;
                // }

                // std::cout << "旧任务" << idx << " " << task_queue->size() << std::endl;

                // if(idx == 1024){
                //     std:cout << "开始debug" << std::endl;
                // }

                /**
                 * 访问1024刚好越块，导致出现错误
                 * 一开始获取旧任务是move操作
                 */
                // std::cout << "开始访问: " << idx << " " << "当前队列大小: " << size << std::endl;
                // taskqueue中获取是可以直接转移过来的,后续反正还是析构

                this->setData(std::move(task_queue->at(start_idx)), false);

                binary_search = true;
                binary_left = false;
                // idx++;
            }

            /**
             * 获取一开始的h_i，如果后续回溯到现在的层级，直接return
             */
            int now_h_i = h_i - 1;
            /**
             * 执行后续层级的查找和匹配
             */
            while (true)
            {
                // 如果栈中没有元素，直接出栈
                // if (_sg_edgeStack.empty())
                // {
                //     return;
                // }

                /**
                 * 如果需要二分，则进行二分搜索当前层级的候选边范围,h_i表示当前要匹配的层级
                 * 当前分块队列压入的新任务在一开始层次需要重新二分右边界，每次计算得都是当前块内的
                 * 旧任务才需要重新二分
                 * || binary_search
                 */
                // std::cout << (p == nullptr) << std::endl;

                if (candidates[h_i].first == -1 || (binary_search && candidates[h_i].first < p->second))
                {
                    findNextMatch(h_i, g_i, all_edge_idxs, *_g);
                    binary_search = false;
                    binary_left = true;
                }

                // 如果当前层级的候选边不存在，直接回溯，弹出边并且重置当前层级候选边
                if (candidates[h_i].first > candidates[h_i].second || candidates[h_i].first >= candidates[h_i].second + 1)
                {
                    candidates[h_i].first = candidates[h_i].second = -1;
                    h_i--;
                    if (h_i == now_h_i)
                    {
                        break;
                    }

                    continue;
                }

                /**
                 * 当前层级的候选边存在，则将当前层级的候选边加入到栈中，并更新当前层级的搜索范围；并标识可以查找下一层级
                 * flag表示当前是否找到了对应边
                 */
                bool flag = false;
                int idx = 0;
                int g_u;
                int g_v;
                for (idx = candidates[h_i].first; idx <= candidates[h_i].second; idx++)
                {
                    // 不可以越块访问
                    int cur_time = arr[h_i]->at(idx);

                    g_u = csr->_edges[cur_time].source();
                    g_v = csr->_edges[cur_time].dest();
                    int g_time = csr->_edges[cur_time].time();

                    /**
                     * 当前层直接回溯
                     */
                    if (g_time > _delta + _firstEdgeTime)
                    {
                        break;
                    }

                    if (cur_time >= p->second)
                    {
                        // 当前边索引大于分块的边索引，压入到对应队列中
                        int id = cur_time / block_size;
                        // GraphSearch gs = *this;
                        // gs.p = &(p_s->at(id));
                        // mtx->at(id).lock();

                        // uint64_t start = HighResClock::now_ns();

                        q->at(id).enqueue(std::move(this->getData()), thread_id);

                        // uint64_t end = HighResClock::now_ns();
                        // trans_time += (end - start);

                        // pool->res_num.fetch_add(1);

                        // mtx->dsdsdt(id).unlock();
                        break;
                    }

                    if (findNextMatch(h_i, cur_time))
                    {
                        // int id = arr[h_i]->at(idx);

                        /**最后一层满足要求，循环遍历，不回溯最后一层
                         * 修改映射和计数
                         */
                        if (h_i + 1 == m)
                        {
                            g_i = cur_time + 1;
                            candidates[h_i].first = idx + 1;

                            res_num++;

                            continue;
                        }

                        g_u = csr->_edges[cur_time].source();
                        g_v = csr->_edges[cur_time].dest();
                        int h_u = _h->_edges[h_i].source();
                        int h_v = _h->_edges[h_i].dest();

                        // if (_h2gNodes[h_u] == -1)
                        {
                            _h2gNodes[h_u] = g_u;
                        }

                        // if (_h2gNodes[h_v] == -1)
                        {
                            _h2gNodes[h_v] = g_v;
                        }

                        // map[h_u].first = g_u;
                        // map[h_u].second++;
                        // map[h_v].first = g_v;
                        // map[h_v].second++;

                        // _sg_edgeStack.push(cur_time);
                        g_i = cur_time + 1;
                        candidates[h_i].first = idx + 1;
                        flag = true;
                        break;
                    }
                }

                /**
                 * 如果当前块中找到了对应边，更新h_i；h_i表示下一层要匹配的层级
                 * 否则出栈，h_i不变
                 * 最后一层直接匹配完成，不进行回溯
                 */
                if (flag)
                {
                    h_i++;
                    // 如果当前栈内的元素数量>=边的容量，则压入结果队列；并且弹出当前边
                    // if (h_i >= _h->_edges.size())
                    {
                    }
                    // else
                    {
                        // 这一层找到了候选边，意味着下一层的状态需要更新，除了最后一层
                        candidates[h_i].first = candidates[h_i].second = -1;
                    }
                }
                else
                {
                    // 当前层级找不到，则修改并且返回到上一个层级

                    /**
                     * 修改映射和计数
                     */
                    candidates[h_i] = {-1, -1};
                    h_i--;
                    // int id = _sg_edgeStack.top();

                    // 修改下这部分
                    // int h_u = _h->_edges[h_i].source();
                    // int h_v = _h->_edges[h_i].dest();

                    // map[h_u].second--;
                    // map[h_v].second--;

                    // if (map[h_u].second == 0)
                    // {
                    //     _h2gNodes[h_u] = -1;
                    // }

                    // if (map[h_v].second == 0)
                    // {
                    //     _h2gNodes[h_v] = -1;
                    // }

                    // _numSearchEdgesForNode[g_u]--;
                    // _numSearchEdgesForNode[g_v]--;

                    // if (_numSearchEdgesForNode[g_u] == 0 || _numSearchEdgesForNode[g_v] == 0)
                    // {

                    //     for (int i = 0; i < _h->numNodes(); i++)
                    //     {
                    //         // 如果h2g的映射中存在g_u和g_v，则表示这两个点已经被映射
                    //         if (_h2gNodes[i] == g_u)
                    //         {
                    //             h_u = i;
                    //         }
                    //         if (_h2gNodes[i] == g_v)
                    //         {
                    //             h_v = i;
                    //         }
                    //     }
                    // }

                    // if (_numSearchEdgesForNode[g_u] == 0)
                    // {
                    //     _h2gNodes[h_u] = -1;
                    // }

                    // if (_numSearchEdgesForNode[g_v] == 0)
                    // {
                    //     _h2gNodes[h_v] = -1;
                    // }

                    // g_i = id + 1;
                    // _sg_edgeStack.pop();
                }
                if (h_i == now_h_i)
                {
                    break;
                }
            }
        }
    }
}

void GraphSearch::findNewOrderedSubgraphs(int *edge_idx, std::mutex *edge_mtx, int thread_id, int thread_num, AlignedAtomic<int> *phase_tasks_beg, AlignedAtomic<int> *phase_tasks_end)
{
    /**
     * 只有一开始的时候需要初始化
     * 由于重复利用变量，需要初始化
     */
    int m = _h->numEdges();
    int n = _h->numNodes();
    // _h2gNodes.clear();
    // _h2gNodes.resize(n, -1);
    // _numSearchEdgesForNode.clear();
    // _numSearchEdgesForNode.resize(_g->numNodes(), 0);
    // 用于替代_numSearchEdgesForNode
    // map.clear();
    // map.resize(n, {-1, 0});
    // candidates.clear();
    // candidates.resize(m, {-1, -1});
    for (int i = 0; i < m; i++)
    {
        candidates[i] = {-1, -1};
    }
    binary_left = binary_search = true;
    // 清空栈空间
    // while (!_sg_edgeStack.empty())
    // {
    //     _sg_edgeStack.pop();
    // }
    // _sg_edgeStack.clear();
    h_i = 0;

    // int idx = -1;
    // int times = 0;
    int nums = 32;

    while (true)
    {
        // {
        //     /**
        //      * 通过按照需要去获取，可以减少work_steal的情况；
        //      */
        //     unique_lock<std::mutex> lock(*edge_mtx);
        //     if (*edge_idx >= p->second)
        //     {
        //         return;
        //     }
        //     g_i = *edge_idx;
        //     *edge_idx += 1;
        //     // std::cout << "thread " << " get edge " << g_i << std::endl;
        // }

        {
            // if (idx == -1)
            // {
            //     idx = thread_id;
            // }
            // else
            // {
            //     idx += thread_num;
            // }

            // 1

            /**
             * 修改起始边索引
             */
            // g_i = idx;
            // std::cout << g_i << std::endl;

            // if (g_i % 10000 == 0)
            // {
            //     std::cout << g_i << " " << thread_id << std::endl;
            // }

            if (start_idx >= end_idx)
            {
                while (true)
                {
                    bool excepted = false;
                    if (pool->steal_flag[thread_id].compare_exchange_strong(excepted, true, std::memory_order_acquire, std::memory_order_relaxed))
                    {
                        /**
                         * 一次性获取nums个任务
                         */
                        start_idx = phase_tasks_beg[thread_id + part_idx * thread_num].fetch_add(nums, std::memory_order_release);
                        end_idx = std::min(int(phase_tasks_end[thread_id + part_idx * thread_num].load(std::memory_order_relaxed)), start_idx + nums);
                        if (start_idx >= end_idx)
                        {
                            pool->steal_flag[thread_id].store(false, std::memory_order_release);
                            return;
                        }
                        pool->steal_flag[thread_id].store(false, std::memory_order_release);
                        break;
                    }
                }
            }
            // std::cout << "新任务" << g_i << " " << p->second << std::endl;
        }

        for (; start_idx < end_idx; start_idx++)
        {
            g_i = start_idx;
            // }

            /**
             * 测试第一条边
             */
            if (h_i == 0)
            {
                // _numSearchEdgesForNode.resize(_g->numNodes(), 0);
                int h_source = _h->_edges[h_i].source();
                int h_end = _h->_edges[h_i].dest();

                int g_source = csr->_edges[g_i].source();
                int g_end = csr->_edges[g_i].dest();

                // 如果当前边不满足拓扑要求，直接退出
                if (!(h_source == h_end && g_source == g_end || h_source != h_end && g_source != g_end))
                {
                    continue;
                }

                // dfs过程不修改temp，而是修改自身值
                _h2gNodes[h_source] = g_source;
                _h2gNodes[h_end] = g_end;
                // std::cout << g_end << std::endl;
                // _numSearchEdgesForNode[g_source]++;
                // _numSearchEdgesForNode[g_end]++;
                // map[h_source].first = g_source;
                // map[h_source].second++;
                // map[h_end].first = g_end;
                // map[h_end].second++;
                h_i++;
                // _sg_edgeStack.push(g_i);
                _firstEdgeTime = csr->_edges[g_i].time();
                g_i++;

                // times = 0;
            }

            /**
             * 执行后续层级的查找和匹配
             */
            while (true)
            {

                // 如果栈中没有元素，直接出栈
                if (h_i == 0)
                {
                    break;
                }

                // 如果需要二分，则进行二分搜索当前层级的候选边范围,h_i表示当前要匹配的层级
                if (candidates[h_i].first == -1)
                {
                    findNextMatch(h_i, g_i, all_edge_idxs, *_g);
                }

                // 如果当前层级的候选边不存在，直接回溯，弹出边并且重置当前层级候选边
                if (candidates[h_i].first > candidates[h_i].second || candidates[h_i].first >= candidates[h_i].second + 1)
                {
                    candidates[h_i].first = candidates[h_i].second = -1;
                    h_i--;
                    continue;
                }

                /**
                 * 当前层级的候选边存在，则将当前层级的候选边加入到栈中，并更新当前层级的搜索范围；并标识可以查找下一层级
                 * flag表示当前是否找到了对应边
                 * 感觉没必要扫这个右边界，因为右边界本身就可扫描过来判断
                 */
                bool flag = false;
                int idx = 0;
                int g_u;
                int g_v;
                for (idx = candidates[h_i].first; idx <= candidates[h_i].second; idx++)
                {
                    // 不可以越块访问
                    int cur_time = arr[h_i]->at(idx);
                    g_u = csr->_edges[cur_time].source();
                    g_v = csr->_edges[cur_time].dest();
                    int g_time = csr->_edges[cur_time].time();

                    /**
                     * 当前层直接回溯
                     */
                    if (g_time > _delta + _firstEdgeTime)
                    {
                        break;
                    }

                    if (cur_time >= p->second)
                    {
                        // 当前边索引大于分块的边索引，压入到对应队列中
                        int id = cur_time / block_size;
                        // GraphSearch gs = *this;
                        // gs.p = &(p_s->at(id));
                        // mtx->at(id).lock();

                        // uint64_t start = HighResClock::now_ns();

                        q->at(id).enqueue(std::move(this->getData()), thread_id);

                        // uint64_t end = HighResClock::now_ns();

                        // trans_time += (end - start);

                        // mtx->at(id).unlock();
                        break;
                    }

                    if (findNextMatch(h_i, cur_time))
                    {
                        // int id = cur_time;

                        /**
                         * 最后一层满足要求，循环遍历，不回溯最后一层
                         * 修改映射和计数
                         */
                        if (h_i + 1 == m)
                        {
                            // flag = true;
                            g_i = cur_time + 1;
                            candidates[h_i].first = idx + 1;

                            res_num++;

                            // res_mutex->lock();

                            // res->enqueue(GraphMatch());

                            // pool->res_num.fetch_add(1);

                            // temp_res->push_back(temp_e);
                            // temp_e.clear();
                            // res_mutex->unlock();

                            // times++;

                            continue;
                        }

                        // int g_u = _g->_edges[cur_time].source();
                        // int g_v = _g->_edges[cur_time].dest();
                        int h_u = _h->_edges[h_i].source();
                        int h_v = _h->_edges[h_i].dest();

                        // if (_h2gNodes[h_u] == -1)
                        {
                            _h2gNodes[h_u] = g_u;
                        }

                        // if (_h2gNodes[h_v] == -1)
                        {
                            _h2gNodes[h_v] = g_v;
                        }

                        // map[h_u].first = g_u;
                        // map[h_u].second++;
                        // map[h_v].first = g_v;
                        // map[h_v].second++;

                        // _sg_edgeStack.push(cur_time);
                        g_i = cur_time + 1;
                        candidates[h_i].first = idx + 1;
                        flag = true;
                        // times++;
                        break;
                    }
                }

                /**
                 * 如果当前块中找到了对应边，更新h_i；h_i表示下一层要匹配的层级
                 * 否则出栈，h_i不变
                 * 优化了最后一层，直接采用bfs执行
                 */
                if (flag)
                {
                    h_i++;
                    // 如果当前栈内的元素数量>=边的容量，则压入结果队列；并且弹出当前边
                    // if (h_i >= m)
                    {
                    }
                    // else
                    {
                        // 这一层找到了候选边，意味着下一层的状态需要更新，除了最后一层
                        candidates[h_i].first = candidates[h_i].second = -1;
                    }
                }
                else
                {
                    // 当前层级找不到，则修改并且返回到上一个层级

                    /**
                     * 最后一层bfs，也是完全找到了，所以直接出栈
                     * 修改映射和计数
                     */
                    candidates[h_i] = {-1, -1};
                    h_i--;
                }
            }
        }
    }
}

void GraphSearch::findOthgerOrderedSubgraphs(LockFreeQueue<SearchData> *task_queue, int *edge_idx, std::mutex *edge_mutex, int thread_id, int thread_num, LockFreeQueue<SearchData> *other_task_queue)
{
    // std::cout << "执行 findOrderedSubgraphs()" << std::endl;
    /**
     * 测试第一条边
     */
    // if (h_i == 0)
    // {
    //     // _numSearchEdgesForNode.resize(_g->numNodes(), 0);
    //     /**
    //      * 初始化分配空间
    //      */
    //     _h2gNodes.clear();
    //     _h2gNodes.resize(_h->numNodes(), -1);
    //     // _numSearchEdgesForNode.clear();
    //     // _numSearchEdgesForNode.resize(_g->numNodes(), 0);
    //     // 用于替代_numSearchEdgesForNode
    //     map.clear();
    //     map.resize(_h->numNodes(), {-1, 0});

    //     int h_source = _h->edges()[h_i].source();
    //     int h_end = _h->edges()[h_i].dest();

    //     int g_source = _g->edges()[g_i].source();
    //     int g_end = _g->edges()[g_i].dest();

    //     // 如果当前边不满足拓扑要求，直接退出
    //     if (!(h_source == h_end && g_source == g_end || h_source != h_end && g_source != g_end))
    //     {
    //         return;
    //     }

    //     // dfs过程不修改temp，而是修改自身值
    //     _h2gNodes[h_source] = g_source;
    //     _h2gNodes[h_end] = g_end;
    //     // _numSearchEdgesForNode[g_source]++;
    //     // _numSearchEdgesForNode[g_end]++;
    //     map[h_source].first = g_source;
    //     map[h_source].second++;
    //     map[h_end].first = g_end;
    //     map[h_end].second++;
    //     h_i++;
    //     _sg_edgeStack.push(g_i);
    //     _firstEdgeTime = _g->edges()[g_i].time();
    //     g_i++;
    // }

    // std::cout << task_queue->size() << std::endl;
    int m = _h->numEdges();
    int n = _h->numNodes();
    // int idx = -1;
    int size = other_task_queue->size();
    int nums = 32;
    // int bg = size / thread_num * thread_id;
    // int ed = bg + size / thread_num;
    // if (size % thread_num > thread_id)
    // {
    //     bg += thread_id;
    //     ed += thread_id + 1;
    // }
    // else
    // {
    //     bg += size % thread_num;
    //     ed += size % thread_num;
    // }
    // idx = bg;

    // std::cout << "旧任务thread " << thread_id << " start from " << bg << " to " << ed << std::endl;

    while (true)
    {
        /**
         * 要是窃取成功就直接执行，这部分是未窃取执行得操作
         */
        if (start_idx >= end_idx)
        {
            while (true)
            {
                bool excepted = false;
                if (pool->steal_flag[thread_id].compare_exchange_strong(excepted, true, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    /**
                     * 一次性获取nums个任务
                     */
                    start_idx = pool->old_task_beg[thread_id + part_idx * thread_num].fetch_add(nums, std::memory_order_release);
                    end_idx = std::min(int(pool->old_task_end[thread_id + part_idx * thread_num].load(std::memory_order_relaxed)), start_idx + nums);
                    if (start_idx >= end_idx)
                    {
                        pool->steal_flag[thread_id].store(false, std::memory_order_release);
                        return;
                    }
                    // pool->old_process.fetch_add(end_idx - start_idx);
                    pool->steal_flag[thread_id].store(false, std::memory_order_release);
                    break;
                }
            }
        }

        // while (idx < ed)
        int temp_idx = start_idx;
        for (; start_idx < end_idx; start_idx++)
        {
            // int idx = -1;

            // {
            //     std::unique_lock<std::mutex> lock(*edge_mutex);
            //     if ((*edge_idx) >= task_queue->size())
            //     {
            //         return;
            //     }
            //     *this = task_queue->at(*edge_idx);
            //     binary_search = true;
            //     binary_left = false;
            //     // std::cout << "开始执行旧边" << (*edge_idx) << " " << task_queue->size() << std::endl;
            //     // std::cout << block_size << " " << p -> first << " " << p -> second << std::endl;
            //     (*edge_idx)++;
            // }

            {
                // 先计算当前要获取的任务索引
                // if (idx == -1)
                // {
                //     idx = thread_id;
                // }
                // else
                // {
                //     idx += thread_num;
                // }

                // if (idx >= task_queue->size())
                // {
                //     return;
                // }

                // std::cout << "旧任务" << idx << " " << task_queue->size() << std::endl;

                // if(idx == 1024){
                //     std:cout << "开始debug" << std::endl;
                // }

                /**
                 * 访问1024刚好越块，导致出现错误
                 * 一开始获取旧任务是move操作
                 */
                // std::cout << "开始访问: " << idx << " " << "当前队列大小: " << size << std::endl;
                // taskqueue中获取是可以直接转移过来的,后续反正还是析构

                // this->setData(std::move(other_task_queue->at(start_idx)), false);
                this->setData(std::move(other_tasks[start_idx - temp_idx]), false);

                binary_search = true;
                binary_left = false;
                // idx++;
            }

            /**
             * 获取一开始的h_i，如果后续回溯到现在的层级，直接return
             */
            int now_h_i = h_i - 1;
            /**
             * 执行后续层级的查找和匹配
             */
            while (true)
            {
                // 如果栈中没有元素，直接出栈
                // if (_sg_edgeStack.empty())
                // {
                //     return;
                // }

                /**
                 * 如果需要二分，则进行二分搜索当前层级的候选边范围,h_i表示当前要匹配的层级
                 * 当前分块队列压入的新任务在一开始层次需要重新二分右边界，每次计算得都是当前块内的
                 * 旧任务才需要重新二分
                 * || binary_search
                 */
                // std::cout << (p == nullptr) << std::endl;

                if (candidates[h_i].first == -1 || (binary_search && candidates[h_i].first < p->second))
                {
                    findNextMatch(h_i, g_i, all_edge_idxs, *_g);
                    binary_search = false;
                    binary_left = true;
                }

                // 如果当前层级的候选边不存在，直接回溯，弹出边并且重置当前层级候选边
                if (candidates[h_i].first > candidates[h_i].second || candidates[h_i].first >= candidates[h_i].second + 1)
                {
                    candidates[h_i].first = candidates[h_i].second = -1;
                    h_i--;
                    if (h_i == now_h_i)
                    {
                        break;
                    }

                    /**
                     * 这部分回溯是确保下一个匹配边是按照时序递增的
                     */
                    // g_i = _sg_edgeStack.top() + 1;
                    // _sg_edgeStack.pop();
                    continue;
                }

                /**
                 * 当前层级的候选边存在，则将当前层级的候选边加入到栈中，并更新当前层级的搜索范围；并标识可以查找下一层级
                 * flag表示当前是否找到了对应边
                 */
                bool flag = false;
                int idx = 0;
                int g_u;
                int g_v;
                for (idx = candidates[h_i].first; idx <= candidates[h_i].second; idx++)
                {
                    // 不可以越块访问
                    int cur_time = arr[h_i]->at(idx);

                    g_u = csr->_edges[cur_time].source();
                    g_v = csr->_edges[cur_time].dest();
                    int g_time = csr->_edges[cur_time].time();

                    /**
                     * 当前层直接回溯
                     */
                    if (g_time > _delta + _firstEdgeTime)
                    {
                        break;
                    }

                    if (cur_time >= p->second)
                    {
                        // 当前边索引大于分块的边索引，压入到对应队列中
                        int id = cur_time / block_size;
                        // GraphSearch gs = *this;
                        // gs.p = &(p_s->at(id));
                        // mtx->at(id).lock();

                        // uint64_t start = HighResClock::now_ns();

                        q->at(id).enqueue(std::move(this->getData()), thread_id);

                        // uint64_t end = HighResClock::now_ns();

                        // trans_time += (end - start);

                        // pool->res_num.fetch_add(1);

                        // mtx->dsdsdt(id).unlock();
                        break;
                    }

                    if (findNextMatch(h_i, cur_time))
                    {
                        // int id = arr[h_i]->at(idx);

                        /**最后一层满足要求，循环遍历，不回溯最后一层
                         * 修改映射和计数
                         */
                        if (h_i + 1 == m)
                        {
                            g_i = cur_time + 1;
                            candidates[h_i].first = idx + 1;

                            res_num++;

                            continue;
                        }

                        // int g_u = _g->_edges[cur_time].source();
                        // int g_v = _g->_edges[cur_time].dest();
                        int h_u = _h->_edges[h_i].source();
                        int h_v = _h->_edges[h_i].dest();

                        // if (_h2gNodes[h_u] == -1)
                        {
                            _h2gNodes[h_u] = g_u;
                        }

                        // if (_h2gNodes[h_v] == -1)
                        {
                            _h2gNodes[h_v] = g_v;
                        }

                        // map[h_u].first = g_u;
                        // map[h_u].second++;
                        // map[h_v].first = g_v;
                        // map[h_v].second++;

                        // _sg_edgeStack.push(cur_time);
                        g_i = cur_time + 1;
                        candidates[h_i].first = idx + 1;
                        flag = true;
                        break;
                    }
                }

                /**
                 * 如果当前块中找到了对应边，更新h_i；h_i表示下一层要匹配的层级
                 * 否则出栈，h_i不变
                 * 最后一层直接匹配完成，不进行回溯
                 */
                if (flag)
                {
                    h_i++;
                    // 如果当前栈内的元素数量>=边的容量，则压入结果队列；并且弹出当前边
                    // if (h_i >= _h->_edges.size())
                    {
                    }
                    // else
                    {
                        // 这一层找到了候选边，意味着下一层的状态需要更新，除了最后一层
                        candidates[h_i].first = candidates[h_i].second = -1;
                    }
                }
                else
                {
                    // 当前层级找不到，则修改并且返回到上一个层级

                    /**
                     * 修改映射和计数
                     */
                    candidates[h_i] = {-1, -1};
                    h_i--;
                }
                if (h_i == now_h_i)
                {
                    break;
                }
            }
        }
    }
}

/**
 * 当前任务窃取中，不允许回溯到超过当前窃取边之上的部分
 */
void GraphSearch::stealOrderedSubgraphs(int *edge_idx, std::mutex *edge_mtx, int thread_id, int thread_num, AlignedAtomic<int> *phase_tasks_beg, AlignedAtomic<int> *phase_tasks_end)
{
    /**
     * 直接继承分配的任务数据
     */
    int m = _h->numEdges();
    int n = _h->numNodes();

    // int idx = -1;
    int times = 0;
    {
        // if (idx == -1)
        // {
        //     idx = thread_id;
        // }
        // else
        // {
        //     idx += thread_num;
        // }

        // 1
        // g_i = phase_tasks_beg[thread_id].fetch_add(1);

        /**
         * 修改起始边索引
         */
        // g_i = idx;

        // if (g_i >= phase_tasks_end[thread_id])
        // {
        //     return;
        // }

        // std::cout << "新任务" << g_i << " " << p->second << std::endl;
    }

    /**
     * 测试第一条边
     */
    // if (h_i == 0)
    // {
    //     // _numSearchEdgesForNode.resize(_g->numNodes(), 0);
    //     int h_source = _h->_edges[h_i].source();
    //     int h_end = _h->_edges[h_i].dest();

    //     int g_source = _g->_edges[g_i].source();
    //     int g_end = _g->_edges[g_i].dest();

    //     // 如果当前边不满足拓扑要求，直接退出
    //     if (!(h_source == h_end && g_source == g_end || h_source != h_end && g_source != g_end))
    //     {
    //         continue;
    //     }

    //     // dfs过程不修改temp，而是修改自身值
    //     _h2gNodes[h_source] = g_source;
    //     _h2gNodes[h_end] = g_end;
    //     // _numSearchEdgesForNode[g_source]++;
    //     // _numSearchEdgesForNode[g_end]++;
    //     map[h_source].first = g_source;
    //     map[h_source].second++;
    //     map[h_end].first = g_end;
    //     map[h_end].second++;
    //     h_i++;
    //     _sg_edgeStack.push(g_i);
    //     _firstEdgeTime = _g->_edges[g_i].time();
    //     g_i++;

    //     times = 0;
    // }

    /**
     * 执行后续层级的查找和匹配
     */
    while (true)
    {

        // 如果栈中没有元素，直接出栈
        if (h_i == 0)
        {
            // std::cout << "执行完成窃取任务" << std::endl;
            break;
        }

        // 如果需要二分，则进行二分搜索当前层级的候选边范围,h_i表示当前要匹配的层级
        if (candidates[h_i].first == -1)
        {
            findNextMatch(h_i, g_i, all_edge_idxs, *_g);
        }

        // 如果当前层级的候选边不存在，直接回溯，弹出边并且重置当前层级候选边
        /**
         * 现在arr[h_i]直接变为csr数组，长度很大，需要注意修改candidates[h_i]的值
         */
        if (candidates[h_i].first > candidates[h_i].second || candidates[h_i].first >= candidates[h_i].second + 1)
        {
            candidates[h_i].first = candidates[h_i].second = -1;
            h_i--;
            // int id = _sg_edgeStack.top();
            // int g_u = _g->_edges[id].source();
            // int g_v = _g->_edges[id].dest();
            // int h_u = _h->_edges[h_i].source();
            // int h_v = _h->_edges[h_i].dest();
            // _numSearchEdgesForNode[g_u]--;
            // _numSearchEdgesForNode[g_v]--;

            // map[h_u].second--;
            // map[h_v].second--;

            // std::cout << h_i << " " << h_u << " " << h_v << std::endl;

            // if (map[h_u].second == 0)
            {
                // _h2gNodes[h_u] = -1;
            }

            // if (map[h_v].second == 0)
            {
                // _h2gNodes[h_v] = -1;
            }

            // if (_numSearchEdgesForNode[g_u] == 0 || _numSearchEdgesForNode[g_v] == 0)
            // {

            //     for (int i = 0; i < _h->numNodes(); i++)
            //     {
            //         // 如果h2g的映射中存在g_u和g_v，则表示这两个点已经被映射
            //         if (_h2gNodes[i] == g_u)
            //         {
            //             h_u = i;
            //         }
            //         if (_h2gNodes[i] == g_v)
            //         {
            //             h_v = i;
            //         }
            //     }
            // }

            // if (_numSearchEdgesForNode[g_u] == 0)
            // {
            //     _h2gNodes[h_u] = -1;
            // }

            // if (_numSearchEdgesForNode[g_v] == 0)
            // {
            //     _h2gNodes[h_v] = -1;
            // }

            // g_i = _sg_edgeStack.top() + 1;
            // _sg_edgeStack.pop();
            continue;
        }

        /**
         * 当前层级的候选边存在，则将当前层级的候选边加入到栈中，并更新当前层级的搜索范围；并标识可以查找下一层级
         * flag表示当前是否找到了对应边
         */
        bool flag = false;
        int idx = 0;

        int g_u;
        int g_v;
        for (idx = candidates[h_i].first; idx <= candidates[h_i].second; idx++)
        {
            // 不可以越块访问
            int cur_time = arr[h_i]->at(idx);
            g_u = csr->_edges[arr[h_i]->at(idx)].source();
            g_v = csr->_edges[arr[h_i]->at(idx)].dest();
            if (cur_time >= p->second)
            {
                // 当前边索引大于分块的边索引，压入到对应队列中
                int id = cur_time / block_size;
                // GraphSearch gs = *this;
                // gs.p = &(p_s->at(id));
                // mtx->at(id).lock();

                q->at(id).enqueue(std::move(this->getData()), thread_id);

                // mtx->at(id).unlock();
                break;
            }

            /**
             * 如果当前搜索树的总深度超过50，并且当前组存在空闲线程，则进行任务划分
             * 遍历所有，寻找空闲的数量
             * 每层窃取一个，就从当前层上面层到h_i层，直接窃取现在执行的
             * 子树级并行不允许重复
             */
            // if (times >= 50)
            // {
            //     int id = thread_id / 4;
            //     for (int i = id * 4; i < (id + 1) * 4 && i != thread_id; i++)
            //     {
            //         if (pool->re_distribute[i] == true)
            //         {
            //             pool->vec_graph[i].setData(std::move(this->getData()), true);
            //             for (int j = 1; j <= h_i; j++)
            //             {
            //                 /**
            //                  * 设置专门的函数，不允许回溯到对应层级，只允许执行这些，其他都不允许了，只可以执行窃取的部分
            //                  */
            //                 candidates[j].first++;
            //             }
            //             // 启动让它去执行
            //             pool->re_distribute[i] == false;
            //         }
            //     }
            //     // 回归状态
            //     idx = candidates[h_i].first - 1;
            //     times = 0;
            //     continue;
            // }
            if (findNextMatch(h_i, cur_time))
            {
                // int id = arr[h_i]->at(idx);

                /**
                 * 最后一层满足要求，循环遍历，不回溯最后一层
                 * 修改映射和计数
                 * bfs最后一层
                 */
                if (h_i + 1 == m)
                {
                    // flag = true;
                    g_i = cur_time + 1;
                    candidates[h_i].first = idx + 1;

                    res_num++;

                    // res_mutex->lock();

                    // res->enqueue(GraphMatch());

                    // pool->res_num.fetch_add(1);

                    // temp_res->push_back(temp_e);
                    // temp_e.clear();
                    // res_mutex->unlock();

                    times++;

                    continue;
                }

                // int g_u = _g->_edges[cur_time].source();
                // int g_v = _g->_edges[cur_time].dest();
                int h_u = _h->_edges[h_i].source();
                int h_v = _h->_edges[h_i].dest();

                // if (_h2gNodes[h_u] == -1)
                {
                    _h2gNodes[h_u] = g_u;
                }

                // if (_h2gNodes[h_v] == -1)
                {
                    _h2gNodes[h_v] = g_v;
                }

                // map[h_u].first = g_u;
                // map[h_u].second++;
                // map[h_v].first = g_v;
                // map[h_v].second++;

                // _sg_edgeStack.push(cur_time);
                g_i = cur_time + 1;
                candidates[h_i].first = idx + 1;
                flag = true;

                times++;
                break;
            }
        }

        /**
         * 如果当前块中找到了对应边，更新h_i；h_i表示下一层要匹配的层级
         * 否则出栈，h_i不变
         */
        if (flag)
        {
            h_i++;
            // 如果当前栈内的元素数量>=边的容量，则压入结果队列；并且弹出当前边
            // if (h_i >= m)
            {
                // ManualStack<int> st(_sg_edgeStack);
                // // std::cout << "准备转换" << std::endl;
                // // 下面的出现问题
                // vector<int> t;
                // while (!st.empty())
                // {
                //     int edge = st.top();
                //     st.pop();
                //     t.push_back(edge);
                // }

                // for (int i = t.size() - 1; i >= 0; i--)
                // {
                //     int edge = t[i];
                //     int src = _g->edges()[edge].source();
                //     int dst = _g->edges()[edge].dest();
                //     int time = _g->edges()[edge].time();
                //     temp_e.push_back(src);
                //     temp_e.push_back(dst);
                //     temp_e.push_back(time);
                // }

                // res_mutex->lock();
                // res->push_back(GraphMatch());
                // // temp_res->push_back(temp_e);
                // // temp_e.clear();
                // res_mutex->unlock();
                // h_i--;

                /**
                 * 修改映射和计数
                 */
                // int id = _sg_edgeStack.top();
                // int g_u = _g->_edges[id].source();
                // int g_v = _g->_edges[id].dest();
                // int h_u = _h->_edges[h_i].source();
                // int h_v = _h->_edges[h_i].dest();

                // map[h_u].second--;
                // map[h_v].second--;

                // // std::cout << h_i << " " << h_u << " " << h_v << std::endl;

                // if (map[h_u].second == 0)
                // {
                //     _h2gNodes[h_u] = -1;
                // }

                // if (map[h_v].second == 0)
                // {
                //     _h2gNodes[h_v] = -1;
                // }

                // _numSearchEdgesForNode[g_u]--;
                // _numSearchEdgesForNode[g_v]--;

                // if (_numSearchEdgesForNode[g_u] == 0 || _numSearchEdgesForNode[g_v] == 0)
                // {

                //     for (int i = 0; i < _h->numNodes(); i++)
                //     {
                //         // 如果h2g的映射中存在g_u和g_v，则表示这两个点已经被映射
                //         if (_h2gNodes[i] == g_u)
                //         {
                //             h_u = i;
                //         }
                //         if (_h2gNodes[i] == g_v)
                //         {
                //             h_v = i;
                //         }
                //     }
                // }

                // if (_numSearchEdgesForNode[g_u] == 0)
                // {
                //     _h2gNodes[h_u] = -1;
                // }

                // if (_numSearchEdgesForNode[g_v] == 0)
                // {
                //     _h2gNodes[h_v] = -1;
                // }

                // _sg_edgeStack.pop();
            }
            // else
            {
                // 这一层找到了候选边，意味着下一层的状态需要更新，除了最后一层
                candidates[h_i].first = candidates[h_i].second = -1;
            }
        }
        else
        {
            // 当前层级找不到，则修改并且返回到上一个层级

            /**
             * 最后一层bfs，也是完全找到了，所以直接出栈
             * 修改映射和计数
             */
            candidates[h_i] = {-1, -1};
            h_i--;
            // int id = _sg_edgeStack.top();
            // int g_u = _g->_edges[id].source();
            // int g_v = _g->_edges[id].dest();
            // int h_u = _h->_edges[h_i].source();
            // int h_v = _h->_edges[h_i].dest();

            // map[h_u].second--;
            // map[h_v].second--;

            // if (map[h_u].second == 0)
            {
                // _h2gNodes[h_u] = -1;
            }

            // if (map[h_v].second == 0)
            {
                // _h2gNodes[h_v] = -1;
            }

            // _numSearchEdgesForNode[g_u]--;
            // _numSearchEdgesForNode[g_v]--;

            // if (_numSearchEdgesForNode[g_u] == 0 || _numSearchEdgesForNode[g_v] == 0)
            // {

            //     for (int i = 0; i < _h->numNodes(); i++)
            //     {
            //         // 如果h2g的映射中存在g_u和g_v，则表示这两个点已经被映射
            //         if (_h2gNodes[i] == g_u)
            //         {
            //             h_u = i;
            //         }
            //         if (_h2gNodes[i] == g_v)
            //         {
            //             h_v = i;
            //         }
            //     }
            // }

            // if (_numSearchEdgesForNode[g_u] == 0)
            // {
            //     _h2gNodes[h_u] = -1;
            // }

            // if (_numSearchEdgesForNode[g_v] == 0)
            // {
            //     _h2gNodes[h_v] = -1;
            // }

            // g_i = id + 1;
            // _sg_edgeStack.pop();
        }
    }
}

/**
 * 查找过程中寻找当前层级的搜索范围和候选边
 */
void GraphSearch::findNextMatch(int h_i, int g_i, std::vector<int> *all_edge_idxs, Graph &g)
{
    /**
     * 统计下执行时间
     */
    // uint64_t start = HighResClock::now_ns();

    bool debugOutput = true;
    int last_time = _firstEdgeTime + _delta;

    // 获取查询图中的当前边
    const Edge &h_edge = _h->_edges[h_i];
    int h_u = h_edge.source(); // 查询图边的起点
    int h_v = h_edge.dest();   // 查询图边的终点

    // 默认情况下，从原始图的所有边开始搜索
    const vector<int> *searchEdges = all_edge_idxs;
    // 存储当前的搜索范围
    BinaryRange *binary_range = nullptr;

    /**
     * 转化为模式感知的switch case
     * 减少分支数量，并且提升速度
     */
    Minfo &minfo = minfos->at(h_i);
    int base = 0;
    int bg = 0, ed = 0;
    int vOffset = 0;
    int vertex = 0;
    int now_Offset = 0;
    /**
     * 根据当前是单点映射还是双点都映射进行额外地处理
     * 1表示出点未知；0表示入点未知；-1表示出入已知
     * 就是通过模式感知来弥补减少book-keeping和回溯操作的缺陷;
     * 因为如果没有模式感知，就需要通过book-keeping和回溯来修改当前节点的映射情况，
     * 通过设置为-1表示未映射，动态进行判断，这样会引入额外开销；而实际上这部分可以直接通过模式感知来优化，因为
     * 算法在实际匹配过程，是完全按照查询图模式的顺序来执行的，每一层的情况都一样，所以可以利用这个特性
     */
    if (minfo.io == 1)
    {
        switch (minfo.baseNode)
        {
        case 0:
            base = _h2gNodes[0];
            break;
        case 1:
            base = _h2gNodes[1];
            break;
        case 2:
            base = _h2gNodes[2];
            break;
        case 3:
            base = _h2gNodes[3];
            break;
        case 4:
            base = _h2gNodes[4];
            break;
        }

        // searchEdges = &_g->nodes()[base].outEdges();
        searchEdges = &csr->out_offset;
        vOffset = csr->out_vertex[base + 1];
        now_Offset = csr->out_vertex[base];
        direct[h_i] = 1;
        vertex = base;

        // binary_range = &out_binary_range[base * p_s -> size() + part_idx];
        // bg = out_binary_range[base * p_s->size() + part_idx].this_block_start;

        /**
         * 当前块大于当前点的最大有解分块索引，则表示当前无解
         * 这部分因为csr无法很好判断，所以需要另外处理
         * <= 当前最大有解分块索引，则表示当前有解
         */
        std::pair<int, int> key(base, part_idx);
        std::pair<int, int> value;
        if (out_block_map->get(key, value))
        {
            bg = value.first;
            ed = value.second;
        }
        else
        {
            /**
             * 先试下返回这个值，这个先不改
             */
            // bg = searchEdges->size();
            // ed = searchEdges->size() - 1;
            bg = vOffset;
            ed = vOffset - 1;
        }
    }
    else if (minfo.io == 0)
    {
        switch (minfo.baseNode)
        {
        case 0:
            base = _h2gNodes[0];
            break;
        case 1:
            base = _h2gNodes[1];
            break;
        case 2:
            base = _h2gNodes[2];
            break;
        case 3:
            base = _h2gNodes[3];
            break;
        case 4:
            base = _h2gNodes[4];
            break;
        }

        // searchEdges = &_g->nodes()[base].inEdges();
        searchEdges = &csr->in_offset;
        vOffset = csr->in_vertex[base + 1];
        direct[h_i] = 0;
        now_Offset = csr->in_vertex[base];
        vertex = base;
        // binary_range = &in_binary_range[base * p_s -> size() + part_idx];

        // bg = in_binary_range[base * p_s->size() + part_idx].this_block_start;

        std::pair<int, int> key(base, part_idx);
        std::pair<int, int> value;
        if (in_block_map->get(key, value))
        {
            bg = value.first;
            ed = value.second;
        }
        else
        {
            // bg = searchEdges->size();
            // ed = searchEdges->size() - 1;
            bg = vOffset;
            ed = vOffset - 1;
        }
    }
    else
    {
        int base0, base1;
        switch (minfo.baseNode)
        {
        case 0:
            base0 = _h2gNodes[0];
            break;
        case 1:
            base0 = _h2gNodes[1];
            break;
        case 2:
            base0 = _h2gNodes[2];
            break;
        case 3:
            base0 = _h2gNodes[3];
            break;
        case 4:
            base0 = _h2gNodes[4];
            break;
        }

        switch (minfo.constraintNode)
        {
        case 0:
            base1 = _h2gNodes[0];
            break;
        case 1:
            base1 = _h2gNodes[1];
            break;
        case 2:
            base1 = _h2gNodes[2];
            break;
        case 3:
            base1 = _h2gNodes[3];
            break;
        case 4:
            base1 = _h2gNodes[4];
            break;
        }

        // const vector<int> &uEdges = _g->nodes()[base1].outEdges();
        // const vector<int> &vEdges = _g->nodes()[base0].inEdges();
        int uSize = csr->out_vertex[base1 + 1] - csr->out_vertex[base1];
        int vSize = csr->in_vertex[base0 + 1] - csr->in_vertex[base0];

        if (uSize < vSize)
        {
            // searchEdges = &uEdges; // 使用较小的边集合
            searchEdges = &csr->out_offset;
            vOffset = csr->out_vertex[base1 + 1];
            now_Offset = csr->out_vertex[base1];
            direct[h_i] = 1;
            vertex = base1;
            // binary_range = &out_binary_range[base1 * p_s->size() + part_idx];

            // bg = out_binary_range[base1 * p_s->size() + part_idx].this_block_start;

            std::pair<int, int> key(base1, part_idx);
            std::pair<int, int> value;
            if (out_block_map->get(key, value))
            {
                bg = value.first;
                ed = value.second;
            }
            else
            {
                // bg = searchEdges->size();
                // ed = searchEdges->size() - 1;
                bg = vOffset;
                ed = vOffset - 1;
            }
        }
        else
        {
            // searchEdges = &vEdges;
            searchEdges = &csr->in_offset;
            vOffset = csr->in_vertex[base0 + 1];
            now_Offset = csr->in_vertex[base0];
            direct[h_i] = 0;
            vertex = base0;
            // binary_range = &in_binary_range[base0 * p_s->size() + part_idx];

            // bg = in_binary_range[base0 * p_s->size() + part_idx].this_block_start;

            std::pair<int, int> key(base0, part_idx);
            std::pair<int, int> value;
            if (in_block_map->get(key, value))
            {
                bg = value.first;
                ed = value.second;
            }
            else
            {
                // bg = searchEdges->size();
                // ed = searchEdges->size() - 1;
                bg = vOffset;
                ed = vOffset - 1;
            }
        }
    }

    /**
     * 存储当前层级的最后一边索引，但是这样就跟candiate[h_i].second重复了
     */
    // expanding_vertex[h_i] = vOffset;

    // 确定对应范围
    arr[h_i] = searchEdges;

    // [begin, end]
    // 找到搜索起始位置，时间戳>=g_i，仍然查找起始位置，但是终止位置需要额外判断，防止
    // if (binary_left)

    // uint64_t end = HighResClock::now_ns();
    // binary_prepare += end - start;

    if (candidates[h_i].first == -1)
    {
        beg = findStart(g_i, *searchEdges, *all_edge_idxs, binary_range, bg, ed, vOffset, vertex, now_Offset);
        candidates[h_i].first = beg;
    }

    /**
     * 同层只搜索一次
     */
    // beg = findStart(g_i, *searchEdges, *all_edge_idxs);

    // 获取对应候选边的范围
    // beg = start;

    // 找到终止位置，时间戳 <= g_i + delta
    // end = findEnd(g_i, *searchEdges, g, binary_range, bg, ed);
    /**
     * 这个是之前邻接表的写法，现在换成csr范围大了很多，所以这里需要调整
     */
    // end = searchEdges->size() - 1;
    end = vOffset - 1;

    candidates[h_i].second = end;

    // 如果找不到合适的起始位置，返回无法匹配
    // if (beg > end)
    //     return;

    // 执行搜索

    // std::cout << candidates[h_i].first << " " << candidates[h_i].second << std::endl;

    return;
}

/**
 * 找到满足时间条件的起始边位置
 * 找不到，则返回列表大小
 */
int GraphSearch::findStart(int g_i, const std::vector<int> &edgeIndexes, std::vector<int> &all_edge, BinaryRange *binary_range, int bg, int ed, int vOffset, int vertex, int now_Offset)
{
    // 对应于匹配起始边，即所有边都作为起始边
    /**
     * 不可能存在跳边的情况，所以不可能x存在跳边的情况，所以不可能x
     */
    // if (edgeIndexes.size() > g_i && edgeIndexes[g_i] == g_i)
    //     return g_i;

    // 如果边列表为空或所有边都小于 g_i，返回列表大小，这部分很必要，不然会出现[size - 1, size - 1]这种无解的情况
    // uint64_t start = HighResClock::now_ns();

    time_t last_time = _firstEdgeTime + _delta;
    // if (edgeIndexes.empty())
    //     return edgeIndexes.size();
    if (now_Offset >= vOffset)
    {
        // return edgeIndexes.size();
        return vOffset;
    }
    // if (edgeIndexes.back() < g_i)
    //     return edgeIndexes.size();
    if (edgeIndexes[vOffset - 1] < g_i)
    {
        // return edgeIndexes.size();
        return vOffset;
    }
    // if (edgeIndexes.front() >= g_i && csr->_edges[edgeIndexes.front()].time() <= last_time)
    //     return 0;
    /**
     * 所有内容都应该对应csr的索引实现
     */
    if (edgeIndexes[now_Offset] >= g_i && csr->_edges[edgeIndexes[now_Offset]].time() <= last_time)
        return now_Offset;

    // 否则，执行二分查找以找到满足>=gi的索引
    // int left = binary_range->this_block_start, right = binary_range->next_block_start;

    // int left = bg, right = ed == edgeIndexes.size() ? edgeIndexes.size() - 1 : ed;
    int left = bg, right = ed;
    // int left = 0, right = edgeIndexes.size() - 1;

    // std::cout << left << " " << right << " " << g_i << std::endl;

    while (left < right)
    {
        int mid = left + right >> 1;

        if (edgeIndexes[mid] >= g_i)
        {
            right = mid;
        }
        else
        {
            left = mid + 1;
        }
    }

    /**
     * 确保当前存在符合要求的解，而不是超过时间范围的边；即确保有解
     * 如果当前存在范围内的二分值，但是不满足时间要求，则返回无解
     * 这部分应该是多余的，返回-1的逻辑后续没啥处理
     */
    // if (left < edgeIndexes.size() && (_g->_edges[edgeIndexes[left]].time() > last_time || edgeIndexes[left] < g_i))
    // {
    //     return -1;
    // }

    if (left < vOffset && (csr->_edges[edgeIndexes[left]].time() > last_time || edgeIndexes[left] < g_i))
    {
        // uint64_t end = HighResClock::now_ns();

        // binary_search_time += end - start;
        return vOffset;
    }

    // uint64_t end = HighResClock::now_ns();

    // binary_search_time += end - start;

    return left;

    // while (true)
    // {
    //     if (right <= left)
    //         return left;

    //     int i = (right + left) / 2;
    //     int ei = edgeIndexes[i];

    //     if (ei == g_i)
    //         return i;
    //     if (ei >= g_i && i == left)
    //         return i;
    //     if (ei < g_i)
    //         left = i + 1;
    //     else
    //     {
    //         if (edgeIndexes[i - 1] < g_i)
    //             return i;
    //         right = i - 1;
    //     }
    // }
}

/**
 * 找到满足时间条件的终止边>=g_i && <= last_time
 * beg表示左边界，找不到则返回列表大小；
 * end若返回非列表大小，则表示找到了满足条件的终止边；否则返回列表大小
 * 由于第一层直接压入，并且后续可以确保查找的都是每次最多+1点的情况，即不存在两点都不映射的情况
 */
int GraphSearch::findEnd(int g_i, const std::vector<int> &edgeIndexes, Graph &g, BinaryRange *binary_range, int bg, int ed)
{
    // // 对应于匹配起始边，即所有边都作为起始边
    // if (edgeIndexes.size() > g_i && edgeIndexes[g_i] == g_i)
    //     return g_i;

    // // 如果边列表为空或所有边都小于 g_i，返回列表大小
    // if (edgeIndexes.empty())
    //     return edgeIndexes.size();
    // if (edgeIndexes.back() < g_i)
    //     return edgeIndexes.size();
    // if (edgeIndexes.front() >= g_i)
    //     return 0;

    // 终止时间
    time_t last_time = _firstEdgeTime + _delta;

    /**
     * 否则，执行二分查找以找到满足<=last_time且在当前块的右边界
     * 可以从当前起始位置开始来进行优化，减少二分流量
     * 二分最多也就返回到edgeIndexes.size() - 1
     * 要不所有边的出入边也分块下
     * */

    // int left = binary_range->this_block_start, right = binary_range->next_block_start;

    // int left = candidates[h_i].first - 1, right = binary_range->next_block_start;

    // int left = bg, right = ed == edgeIndexes.size() ? edgeIndexes.size() - 1 : ed;
    int left = bg, right = ed;

    // std::cout << "left: " << left << ", right: " << right << std::endl;

    // int left = 0, right = edgeIndexes.size() - 1;

    // if (left > right)
    // {
    //     return -1;
    // }
    // else if (left == right)
    // {
    //     return
    // }

    while (left < right)
    {
        int mid = (left + right + 1) >> 1;
        int mid_idx = edgeIndexes[mid];

        // cache缺失较严重1.56%
        if (csr->_edges[mid_idx].time() <= last_time)
        {
            left = mid;
            /**
             * 如果当前右边界越块，直接返回对应值，减少越块访问
             * 如果p -> second > last，即区间尾部在块内，无需执行这段
             * 如果p -> second <= last，即区间尾部在块外，直接返回left，减少越块访问，后续再搜索当前块内的
             * */
            if (mid_idx >= p->second)
            {
                return left;
            }
        }
        else
        {
            right = mid - 1;
        }
    }
    return left;
}

/**
 * 这部分也可以直接根据模式感知进行优化
 * 减少匹配次数并且抵消book-keepng和回溯的缺陷
 */
bool GraphSearch::findNextMatch(int h_i, int g_idx)
{

    // uint64_t start = HighResClock::now_ns();

    // bool debugOutput = false;

    // 获取查询图中的当前边
    // const Edge &h_edge = _h->_edges[h_i];
    // int h_u = h_edge.source(); // 查询图边的起点
    // int h_v = h_edge.dest();   // 查询图边的终点

    // 检查时间窗口限制（如果已有匹配边）
    // bool checkTime = !_sg_edgeStack.empty();

    // 搜索当前边的情况
    // for (int i = startIndex; i < edgesToSearch.size(); i++)
    // {
    // 获取原始图中的当前边索引

    // 获取原始图中的当前边
    const Edge &g_edge = csr->_edges[g_idx];
    int g_u = g_edge.source(); // 原始图边的起点
    int g_v = g_edge.dest();   // 原始图边的终点

    // 如果超出时间窗口限制，停止搜索
    // if (g_edge.time() - _firstEdgeTime > _delta)
    //     return false;

    // if (debugOutput)
    // {
    //     cout << "尝试边 " << g_i << ": " << g_u << ", " << g_v << "    ";
    //     cout << "需要匹配边 " << h_i << ": " << h_u << ", " << h_v << endl;
    //     cout << "   g[" << g_u << "]=" << _g2hNodes[g_u] << " g[" << g_v << "]=" << _g2hNodes[g_v] << endl;
    //     cout << "   h[" << h_u << "]=" << _h2gNodes[h_u] << " h[" << h_v << "]=" << _h2gNodes[h_v] << endl;
    // }

    // 表示g中这两个点未被映射

    /**
     * 通过模式感知优化边匹配步骤
     * constraintNode: < 0 表示是新加的点，新加的点要保证和之前所有的点
     * 不一样；>= 0 表示当前点不是新加的点，但是之前二分只保证了节点的
     * 前面进行出入候选列表判断的时候进行对应的修改minfo.io，保证后续地判断正确
     * 当minfo.constraintNode >= 0的时候，要确保对应的另一个边满足对应的要求
     * 只有第一边需要判断自环；其余的都已经模式感知好了，其实这个是允许自环现象的
     * 只是必须连续递增罢了，因为每次最多新增一点
     */
    Minfo *minfo = &minfos->at(h_i);
    if (minfo->io == -1)
    {
        /**
         * 只有特殊情况这样处理
         */
        temp = minfos->at(h_i);
        minfo = &temp;
        /**
         * 进行动态判断
         * 这个和双点映射也是一样的，逻辑都是找到出入边索引最小的点
         * 但是这个其实可以做点小优化，去取后续边最少的点判断，但是
         * 这样有多一次hash开销，平均多四次
         */
        int base0 = _h2gNodes[minfo->baseNode];
        int base1 = _h2gNodes[minfo->constraintNode];
        // const vector<int> &uEdges = _g->nodes()[base1].outEdges();
        // const vector<int> &vEdges = _g->nodes()[base0].inEdges();
        int uSize = csr->out_vertex[base1 + 1] - csr->out_vertex[base1];
        int vSize = csr->in_vertex[base0 + 1] - csr->in_vertex[base0];
        if (uSize < vSize)
        {
            minfo->constraintNode = minfo->baseNode;
            minfo->io = 1;
        }
        else
        {
            minfo->io = 0;
        }
    }

    int node = minfo->io ? g_v : g_u;
    bool checked = true;
    /**
     * 好像只需要动态调整这部分
     * 而且好像也并不需要预编码？只需要知道对应边的点的扩展顺序
     * mappedNodes表示前面的顺序
     * 因为这种情况无法排除重新连接0导致的干扰，所以还是设置为newNode判断
     * 这个就是修改下转化为两层判断
     * 自环最后还是占据两个node，所以这种情况下还是最多5个点，不过重复点也算，这个后续看看怎么优化；尽量还是自环算一个点，避免不准确
     */
    if (minfo->newNode == 1)
    {
        // switch (minfo->mappedNodes)
        // {
        // case 5:
        //     checked = (_h2gNodes[nodes[4]] != node);
        // case 4:
        //     checked = checked && (_h2gNodes[nodes[3]] != node);
        // case 3:
        //     checked = checked && (_h2gNodes[nodes[2]] != node);
        // case 2:
        //     checked = checked && (_h2gNodes[nodes[0]] != node) && (_h2gNodes[nodes[1]] != node);
        // }
        for (int i = 0; i < minfo->mappedNodes; i++)
        {
            checked = checked && (_h2gNodes[nodes[i]] != node);
        }
    }
    else if (minfo->newNode == 0)
    {
        // switch (minfo->constraintNode)
        // {
        // case 4:
        //     checked = (_h2gNodes[4] == node);
        //     break;
        // case 3:
        //     checked = (_h2gNodes[3] == node);
        //     break;
        // case 2:
        //     checked = (_h2gNodes[2] == node);
        //     break;
        // case 1:
        //     checked = (_h2gNodes[1] == node);
        //     break;
        // case 0:
        //     checked = (_h2gNodes[0] == node);
        //     break;
        // }
        checked = (_h2gNodes[minfo->constraintNode] == node);
    }
    else if (minfo->newNode == 2)
    {
        int node0 = g_u;
        int node1 = g_v;
        bool flag = (node0 != node1) && (g_u != g_v) || (node0 == node1) && (g_u == g_v);
        if (flag)
        {
            return false;
        }
        /**
         * 先source再dest
         */
        switch (minfo->mappedNodes - 1)
        {
        case 5:
            checked = (_h2gNodes[nodes[4]] != node0);
        case 4:
            checked = checked && (_h2gNodes[nodes[3]] != node0);
        case 3:
            checked = checked && (_h2gNodes[nodes[2]] != node0);
        case 2:
            checked = checked && (_h2gNodes[nodes[0]] != node0) && (_h2gNodes[nodes[1]] != node0);
        }

        if (node0 != node1)
        {
            switch (minfo->mappedNodes)
            {
            case 5:
                checked = (_h2gNodes[nodes[4]] != node1);
            case 4:
                checked = checked && (_h2gNodes[nodes[3]] != node1);
            case 3:
                checked = checked && (_h2gNodes[nodes[2]] != node1);
            case 2:
                checked = checked && (_h2gNodes[nodes[0]] != node1) && (_h2gNodes[nodes[1]] != node1);
            }
        }
    }

    // uint64_t end = HighResClock::now_ns();
    // check_time += end - start;

    return checked;
}

vector<int> GraphSearch::convert(stack<int> s)
{
    // 将栈中的元素转换为向量
    vector<int> v(s.size());
    for (int i = v.size() - 1; i >= 0; i--)
    {
        v[i] = s.top();
        s.pop();
    }
    return v;
}

GraphMatch GraphSearch::convert(const std::stack<int> &s, int g_lastEdge)
{
    // 创建一个 GraphMatch 对象
    GraphMatch gm;

    // 将栈中的边索引转换为向量，并添加最后一条边
    vector<int> gEdges = convert(s);
    gEdges.push_back(g_lastEdge);

    // 遍历所有边，将匹配的边添加到 GraphMatch 对象中
    for (int h_i = 0; h_i < gEdges.size(); h_i++)
    {
        int g_i = gEdges[h_i];
        gm.addEdge(_g->edges()[g_i], _h->edges()[h_i]);
    }

    return gm;
}

/**
 * 全部深拷贝
 */
SearchData GraphSearch::getData()
{
    // SearchData *data = new SearchData();
    SearchData data;

    data._firstEdgeTime = _firstEdgeTime;
    // data->_h2gNodes = _h2gNodes;
    // data->_sg_edgeStack = _sg_edgeStack;
    // data->candidates = candidates;

    for (int i = 0; i < 5; i++)
    {
        data._h2gNodes[i] = _h2gNodes[i];
        data.candidates[i] = candidates[i];
        // data.expanding_vertex[i] = expanding_vertex[i];
        data.direct[i] = direct[i];
    }

    data.h_i = h_i;
    // data.g_i = g_i;
    // data->map = map;
    // data.arr[0] = arr[0];
    // data.arr[1] = arr[1];
    // data.arr[2] = arr[2];
    // data.arr[3] = arr[3];
    // data.arr[4] = arr[4];
    // data.arr[5] = arr[5];
    // data.arr[6] = arr[6];
    // data.arr[7] = arr[7];

    return data;
}

void GraphSearch::setData(SearchData &&data)
{
    _firstEdgeTime = data._firstEdgeTime;
    // _h2gNodes = data._h2gNodes;
    // _sg_edgeStack = data._sg_edgeStack;
    // candidates = data.candidates;

    for (int i = 0; i < 5; i++)
    {
        _h2gNodes[i] = data._h2gNodes[i];
        candidates[i] = data.candidates[i];
        // expanding_vertex[i] = data.expanding_vertex[i];
    }

    // g_i = data.g_i;
    h_i = data.h_i;
    // map = data.map;
    // arr[0] = data.arr[0];
    // arr[1] = data.arr[1];
    // arr[2] = data.arr[2];
    // arr[3] = data.arr[3];
    // arr[4] = data.arr[4];
    // arr[5] = data.arr[5];
    // arr[6] = data.arr[6];
    // arr[7] = data.arr[7];
    // arr[8] = data.arr[8];
}

/**
 * 考不考虑释放内存呢
 */
void GraphSearch::setData(SearchData &&data, bool move)
{
    // uint64_t start = HighResClock::now_ns();

    _firstEdgeTime = data._firstEdgeTime;
    // _h2gNodes = std::move(data._h2gNodes);
    // _sg_edgeStack = std::move(data._sg_edgeStack);
    // candidates = std::move(data.candidates);

    for (int i = 0; i < 5; i++)
    {
        _h2gNodes[i] = data._h2gNodes[i];
        candidates[i] = data.candidates[i];
        // expanding_vertex[i] = data.expanding_vertex[i];
        direct[i] = data.direct[i];
        arr[i] = direct[i] == 0 ? &csr->in_offset : &csr->out_offset;
    }

    // g_i = data.g_i;
    h_i = data.h_i;
    // map = std::move(data.map);
    // arr[0] = data.arr[0];
    // arr[1] = data.arr[1];
    // arr[2] = data.arr[2];
    // arr[3] = data.arr[3];
    // arr[4] = data.arr[4];
    // arr[5] = data.arr[5];
    // arr[6] = data.arr[6];
    // arr[7] = data.arr[7];
    // arr[8] = data.arr[8];

    // uint64_t end = HighResClock::now_ns();
    // trans_time += end - start;
}

void GraphSearch::setData(const SearchData &data)
{
    _firstEdgeTime = data._firstEdgeTime;
    // _h2gNodes = data._h2gNodes;
    // _sg_edgeStack = data._sg_edgeStack;
    // candidates = data.candidates;

    for (int i = 0; i < 5; i++)
    {
        _h2gNodes[i] = data._h2gNodes[i];
        candidates[i] = data.candidates[i];
        // expanding_vertex[i] = data.expanding_vertex[i];
    }

    // g_i = data.g_i;
    h_i = data.h_i;
    // map = data.map;
    // arr[0] = data.arr[0];
    // arr[1] = data.arr[1];
    // arr[2] = data.arr[2];
    // arr[3] = data.arr[3];
    // arr[4] = data.arr[4];
    // arr[5] = data.arr[5];
    // arr[6] = data.arr[6];
    // arr[7] = data.arr[7];
    // arr[8] = data.arr[8];
}