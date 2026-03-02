#include <iostream>
#include <sys/resource.h>
#include "CmdArgs.h"
#include "DataGraph.h"
#include "FileIO.h"
#include "GraphFilter.h"
#include "GraphSearch.h"
#include "MatchCriteria_DataGraph.h"
#include <atomic>
#include <mutex>
#include <thread>
#include <queue>
#include "ThreadPool.h"
#include "LockFreeQueue.h"
#include <time.h>
#include "MemoryPool.h"
#include "GraphMatch.h"
#include "BinaryRange.h"
#include "Minfo.h"
#include "Precoing.h"
#include "NumaTool.h"
#include "GraphReader.h"
#include "FlatHashMap.h"
#include "Csr.h"

using namespace std;

// 存储l3 cache的值
size_t l3_cache = 1024 * 98304 / 8 * 5;

// 存储每块的大小
int block_size = 0;

// 存储当前块的索引
int part_now = 0;

void saveVectorToFile(const std::vector<int> &vec)
{
    // 创建文件输出流对象
    const std::string filename = "output.txt";

    std::ofstream outFile(filename);

    // 检查文件是否成功打开
    if (!outFile.is_open())
    {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }

    // 遍历 vector 并将值写入文件（默认每行一个数字）
    for (const auto &num : vec)
    {
        outFile << num << '\n';
    }

    // 关闭文件流
    outFile.close();

    std::cout << "数据已成功写入文件: " << filename << std::endl;
}

void saveVectorsToFile(const std::vector<vector<int>> &vec)
{
    // 创建文件输出流对象
    const std::string filename = "output1.txt";

    std::ofstream outFile(filename);

    // 检查文件是否成功打开
    if (!outFile.is_open())
    {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }

    // 遍历 vector 并将值写入文件（默认每行一个数字）
    for (const auto &nums : vec)
    {
        for (const auto &num : nums)
        {
            outFile << num << ' ';
        }
        outFile << '\n';
    }

    // 关闭文件流
    outFile.close();

    std::cout << "数据已成功写入文件: " << filename << std::endl;
}

/**
 * 预处理起始边，由于起始边采用二分查找方式搜索符合要求的，因此需要直接把符合要求的起始边一次性处理完成，若依次增加，则重复搜索
 */
vector<int> store(DataGraph &g, DataGraph &h, MatchCriteria &criteria)
{
    vector<int> vec;

    for (int i = 0; i < g.edges().size(); i++)
    {
        int g_u = g.edges()[i].source();
        int g_v = g.edges()[i].dest();
        int h_u = h.edges()[0].source();
        int h_v = h.edges()[0].dest();
        if ((h_u == h_v && g_u == g_v) || (h_u != h_v && g_u != g_v))
        {
            if (criteria.isEdgeMatch(g, i, h, 0))
            {
                vec.push_back(i);
            }
        }
    }

    cout << vec.size() << endl;

    return vec;
}

/**
 * 分块函数，将分块后的全局有序列表的块范围存储到vector中
 * 固定各部分的边索引
 */
vector<pair<int, int>> *partition(Graph &g)
{
    // 存储分块的大小
    vector<pair<int, int>> *vec = new vector<pair<int, int>>();
    vec->reserve(1000);
    // 计算各边的内存占用
    size_t size_edge = sizeof(Edge);
    // 计算cache内可以存放的边数量，采用一半去存储
    size_t num_blocks = l3_cache / size_edge;
    cout << "边数据内存占用" << size_edge << endl;
    cout << "GraphSearch任务占用:" << sizeof(SearchData) << endl;
    // size_t num_blocks = 3;
    // 边列表的总边数
    size_t num_edges = g.numEdges();

    block_size = num_blocks;

    // 存储各块的情况
    for (int i = 0;; i++)
    {
        int begin = i * num_blocks;
        int end = min(num_edges, (i + 1) * num_blocks);
        // 存储范围为[begin, end)
        vec->push_back(make_pair(begin, end));
        if (end >= num_edges)
        {
            break;
        }
    }

    return vec;
}

void saveRes(string input, string query, long long time, long long res, long long pre_time)
{
    std::ofstream ofs("res_final.txt", std::ios::app);
    input = input.substr(input.find_last_of("/") + 1);
    query = query.substr(query.find_last_of("/") + 1);
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    ofs << input << " " << query << " " << time << " " << res << " " << pre_time << " " << usage.ru_maxrss << std::endl;
    ofs.close();
}

void saveBreakDown(string input, string query, uint64_t totaltime, uint64_t binary_prapre, uint64_t binary_search_time, uint64_t transtime, uint64_t check_time)
{
    std::ofstream ofs("breakdown.txt", std::ios::app);
    input = input.substr(input.find_last_of("/") + 1);
    query = query.substr(query.find_last_of("/") + 1);
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    ofs << input << " " << query << " " << totaltime << " " << binary_prapre << " " << binary_search_time << " " << transtime << " " << check_time << std::endl;
    ofs.close();
}

void saveScala(string input, string query, long long time)
{
    std::ofstream ofs("NOLoad.txt", std::ios::app);
    input = input.substr(input.find_last_of("/") + 1);
    query = query.substr(query.find_last_of("/") + 1);
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    ofs << input << " " << query << " " << time << " " << usage.ru_maxrss << std::endl;
    ofs.close();
}

void print_peak_memory()
{
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    std::cout << "Peak memory usage: " << usage.ru_maxrss << " KB" << std::endl;
}

// 线程入口函数
// void thread_func(ThreadPool *pool, std::unique_ptr<GraphSearch> obj, Graph &g, const Graph &h, const MatchCriteria &criteria, pair<int, int> p, int limit, int delta, int *counter, mutex *mtx, vector<GraphMatch> *res, queue<GraphSearch> *q, vector<int> *all_edge_idxs, vector<vector<int>> *temp_res, int *add_size)
// {
//     obj->findOrderedSubgraphs(g, h, criteria, p, limit, delta, counter, mtx, res, q, all_edge_idxs, temp_res, add_size, pool);
// }

/**
 * 把edge index和vertex全部转化为csr
 */
void getCSR(Graph &g, Csr &csr)
{
    int nums = 0;
    for (int i = 0; i < g.numNodes(); i++)
    {
        csr.in_vertex.push_back(nums);
        csr.in_offset.insert(csr.in_offset.end(), g._nodes[i].inEdges().begin(), g._nodes[i].inEdges().end());
        nums += g._nodes[i].inEdges().size();
    }
    csr.in_vertex.push_back(nums);

    nums = 0;
    for (int i = 0; i < g.numNodes(); i++)
    {
        csr.out_vertex.push_back(nums);
        csr.out_offset.insert(csr.out_offset.end(), g._nodes[i].outEdges().begin(), g._nodes[i].outEdges().end());
        nums += g._nodes[i].outEdges().size();
    }
    csr.out_vertex.push_back(nums);
    csr._edges = g._edges;
}

int main(int argc, char **argv)
{

    /**
     * 将主线程绑定到0号NUMA节点的0号逻辑线程，确定对应的NUMA节点
     */
    bind_to_core(0);
    numa_set_localalloc();

    // try
    // {
    // 解析命令行参数， 读取输入时序图和查询图等数据
    CmdArgs args(argc, argv);
    if (!args.success())
        return -1;

    // 定义在控制台显示的最大边数（用于测试）
    const int MAX_NUM_EDGES_FOR_DISP = 50;

    // 加载数据图
    cout << "从文件 " << args.graphFname() << " 加载数据图" << endl;
    // 一次性加载点数据和边数据，分别进行存储
    DataGraph g = loadFile(args.graphFname());
    cout << g.nodes().size() << " 个节点, " << g.edges().size() << " 条边" << endl;
    // if (g.numEdges() < MAX_NUM_EDGES_FOR_DISP)
    //     g.disp(); // 显示图的详细信息
    cout << endl;

    MatchCriteria_DataGraph criteria;
    cout << "过滤数据图以提高查询性能。" << endl;

    // 创建数据图
    DataGraph &g2 = g;
    // 设置数据图的点边属性
    // g2.setNodeAttributesDef(g.nodeAttributesDef());
    // g2.setEdgeAttributesDef(g.edgeAttributesDef());

    // 根据满足要求的边时间生成新的源图
    // GraphFilter::filter(g, h, criteria, g2); // 过滤后的图存储在 g2 中
    cout << g2.nodes().size() << " 个节点, " << g2.edges().size() << " 条边" << endl;
    // if (g2.numEdges() < MAX_NUM_EDGES_FOR_DISP)
    //     g2.disp(); // 显示过滤后的图
    cout << endl;
    // 记录每个查询和每个时间差值下的子图计数
    vector<vector<int>> queryDeltaCounts;

    // 打印分块结果
    // cout << "分块结果：" << endl;
    // for (auto &p : *part)
    // {
    //     cout << "[" << p.first << ", " << p.second << ")" << endl;
    // }

    vector<int> in_part_values;
    vector<int> in_part_col_idx;
    vector<int> in_part_row_ptr(1, 0);
    std::pair<int, int> *in_range = nullptr;
    FlatHashMap in_block_map(g2.numNodes());
    int times = 0;
    vector<int> out_part_values;
    vector<int> out_part_col_idx;
    vector<int> out_part_row_ptr(1, 0);
    std::pair<int, int> *out_range;

    FlatHashMap out_block_map(g2.numNodes());

    Csr csr;
    getCSR(g, csr);

    /**
     * 转化为csr，先每个顶点对于所有时序块生成对应的起始边索引，按照这个顺序构建索引图
     */
    // BinaryRange* in_part_ranges = new BinaryRange[g2.numNodes()][part->size];
    auto start_map = std::chrono::high_resolution_clock::now();
    vector<pair<int, int>> *part = partition(g2);

    cout << "开始进行映射表生成" << endl;
    {
        int thread_temp = 64;
        vector<thread> threads;

        start_map = std::chrono::high_resolution_clock::now();
        for (int id = 0; id < thread_temp; id++)
        {
            threads.emplace_back([&](int thread_id)
                                 {
                                     // 绑定核心
                                     int bind_id = thread_id >= 32 ? thread_id + 32 : thread_id;
                                    //  int bind_id = thread_id;
                                     bind_to_core(bind_id);
                                     int size = g2.numNodes();
                                     int bg = size / thread_temp * thread_id;
                                     // if(idx < new_end)
                                     int ed = bg + size / thread_temp;
                                     if (size % thread_temp > thread_id)
                                     {
                                         bg += thread_id;
                                         ed += thread_id + 1;
                                     }
                                     else
                                     {
                                         bg += size % thread_temp;
                                         ed += size % thread_temp;
                                     }

                                     for (int i = bg; i < ed; i++)
                                     {
                                         // in_part_ranges.push_back(vector<BinaryRange>());
                                         // in_part_ranges[i].resize(part->size());
                                         // 倒序访问，这样能保证每个节点的分块集合是递增的

                                         // vector<int> temp_values;
                                         // vector<int> temp_col_idx;
                                         const vector<int> &vEdges = g2.nodes()[i].inEdges();
                                         std::pair<int, int> key;
                                         std::pair<int, int> value;
                                         /**
                                          * 倒序手动存储下一个末值来确保当前块可存储当前块起始和下一个块的开始索引
                                          */
                                         int next_beg = vEdges.size() - 1;

                                         for (int j = part->size() - 1; j >= 0; j--)
                                         {
                                             /**
                                              * 获取对应节点的出入边
                                              * 然后按照顺序去寻找到满足在当前块内的第一条边索引
                                              * 如果下一个块的边索引为edge_size()则说明下一个块不存在索引，全部在当前块内
                                              * edge_size在运行过程中会自动进行满足条件的
                                              * 无解情况下this_block = size, next = size - 1,这样可以保证无解,且避免修改二分
                                              * 按照边索引分块，不是按照时间分块
                                              * 要是同一个块连续存储会不会更好命中,j i这种存储
                                              * csr只存储有解的范围
                                              * 这个是存储从当前块开始的起始边索引，因为需要判断是否存在解，索引不可以把范围局限在当前分块内部
                                              * 要把范围统计为当前块之后的，所以如果当前循环不存在解析，则当前块之后的全部无解
                                              */
                                             bool flag = false;
                                             for (int k = 0; k < vEdges.size(); k++)
                                             {
                                                 /**
                                                  * 确保只存储边完全落下的索引
                                                  */
                                                 if (vEdges[k] >= part->at(j).first && vEdges[k] < part->at(j).second)
                                                 {
                                                     // in_part_ranges[i * part->size() + j].this_block_start = k;
                                                     // temp_values.push_back(k);
                                                     // temp_col_idx.push_back(j);
                                                     key = std::make_pair(i, j);
                                                     value = std::make_pair(k + csr.in_vertex[i], next_beg + csr.in_vertex[i]);
                                                     next_beg = k;
                                                     in_block_map.insert(key, value);
                                                     flag = true;
                                                     times++;
                                                    //  std::cout << "插入成功" << std::endl;
                                                    // std::cout << "顶点 " << i << "插入成功" << value.first << " " << value.second << std::endl; 
                                                     break;
                                                 }
                                             }

                                             // 表示没找到，则当前所有起始范围和最后范围全部变为edge_size
                                             if (flag == false)
                                             {

                                                 // in_part_ranges[i * part->size() + j].this_block_start = vEdges.size();
                                                 // in_part_ranges[i * part->size() + j].next_block_start = vEdges.size() - 1;
                                             }
                                             else if (flag == true && j == part->size() - 1)
                                             {
                                                 // in_part_ranges[i * part->size() + j].next_block_start = vEdges.size() - 1;
                                             }
                                             else
                                             {
                                                 // if (in_part_ranges[i * part->size() + j + 1].this_block_start == vEdges.size())
                                                 // {
                                                 //     // in_part_ranges[i * part->size() + j].next_block_start = in_part_ranges[i * part->size() + j + 1].this_block_start - 1;
                                                 // }
                                                 // else
                                                 // {
                                                 //     // in_part_ranges[i * part->size() + j].next_block_start = in_part_ranges[i * part->size() + j + 1].this_block_start;
                                                 // }
                                             }
                                         }

                                         /**
                                          * 转换顺序，从小块到大块
                                          */
                                         // reverse(temp_values.begin(), temp_values.end());
                                         // reverse(temp_col_idx.begin(), temp_col_idx.end());

                                         // in_part_values.insert(in_part_values.end(), temp_values.begin(), temp_values.end());
                                         // in_part_col_idx.insert(in_part_col_idx.end(), temp_col_idx.begin(), temp_col_idx.end());
                                         // in_part_row_ptr.push_back(in_part_values.size());

                                         // if (temp_col_idx.size() >= 2)
                                         // {
                                         //     in_range[i] = std::make_pair(temp_col_idx[0], temp_col_idx[temp_col_idx.size() - 1]);
                                         // }
                                         // else if (temp_col_idx.size() == 1)
                                         // {
                                         //     in_range[i] = std::make_pair(temp_col_idx[0], temp_col_idx[0]);
                                         // }
                                         // else
                                         // {
                                         //     in_range[i] = std::make_pair(-1, -1);
                                         // }
                                    }

                                    for (int i = bg; i < ed; i++)
                                    {
                                         // out_part_ranges.push_back(vector<BinaryRange>());
                                         // out_part_ranges[i].resize(part->size());
                                         // 倒序访问，这样能保证每个节点的分块集合是递增的

                                         // vector<int> temp_values;
                                         // vector<int> temp_col_idx;
                                         const vector<int> &vEdges = g2.nodes()[i].outEdges();
                                         std::pair<int, int> key;
                                         std::pair<int, int> value;
                                         int next_beg = vEdges.size() - 1;

                                         for (int j = part->size() - 1; j >= 0; j--)
                                         {
                                             /**
                                              * 获取对应节点的出入边
                                              * 然后按照顺序去寻找到满足在当前块内的第一条边索引
                                              * 如果下一个块的边索引为edge_size()则说明下一个块不存在索引，全部在当前块内
                                              * edge_size在运行过程中会自动进行满足条件的
                                              */
                                             // const vector<int> &vEdges = g2.nodes()[i].outEdges();
                                             bool flag = false;
                                             for (int k = 0; k < vEdges.size(); k++)
                                             {
                                                 /**
                                                  * 确保只存储刚好边落下的块，减少无效存储
                                                  */
                                                 if (vEdges[k] >= part->at(j).first && vEdges[k] < part->at(j).second)
                                                 {
                                                     // out_part_ranges[i * part->size() + j].this_block_start = k;
                                                     // temp_values.push_back(k);
                                                     // temp_col_idx.push_back(j);
                                                     key = std::make_pair(i, j);
                                                     value = std::make_pair(k + csr.out_vertex[i], next_beg + csr.out_vertex[i]);
                                                     next_beg = k;
                                                     out_block_map.insert(key, value);
                                                     flag = true;
                                                     times++;
                                                     // std::cout << "插入成功" << std::endl;
                                                    // std::cout << "顶点 " << i << "插入成功" << value.first << " " << value.second << std::endl; 
                                                     break;
                                                 }
                                             }

                                             // 表示没找到，则当前所有起始范围和最后范围全部变为edge_size
                                             if (flag == false)
                                             {
                                                 // out_part_ranges[i * part->size() + j].this_block_start = vEdges.size();
                                                 // out_part_ranges[i * part->size() + j].next_block_start = vEdges.size() - 1;
                                             }
                                             else if (flag == true && j == part->size() - 1)
                                             {
                                                 // out_part_ranges[i * part->size() + j].next_block_start = vEdges.size() - 1;
                                             }
                                             else
                                             {
                                                 // if (out_part_ranges[i * part->size() + j + 1].this_block_start == vEdges.size())
                                                 // {
                                                 //     // out_part_ranges[i * part->size() + j].next_block_start = out_part_ranges[i * part->size() + j + 1].this_block_start - 1;
                                                 // }
                                                 // else
                                                 // {
                                                 //     // out_part_ranges[i * part->size() + j].next_block_start = out_part_ranges[i * part->size() + j + 1].this_block_start;
                                                 // }
                                             }
                                         }

                                         /**
                                          * 转换顺序，从小块到大块
                                          */
                                         // reverse(temp_values.begin(), temp_values.end());
                                         // reverse(temp_col_idx.begin(), temp_col_idx.end());

                                         // out_part_values.insert(out_part_values.end(), temp_values.begin(), temp_values.end());
                                         // out_part_col_idx.insert(out_part_col_idx.end(), temp_col_idx.begin(), temp_col_idx.end());
                                         // out_part_row_ptr.push_back(out_part_values.size());

                                         // if (temp_col_idx.size() >= 2)
                                         // {
                                         //     out_range[i] = std::make_pair(temp_col_idx[0], temp_col_idx[temp_col_idx.size() - 1]);
                                         // }
                                         // else if (temp_col_idx.size() == 1)
                                         // {
                                         //     out_range[i] = std::make_pair(temp_col_idx[0], temp_col_idx[0]);
                                         // }
                                         // else
                                         // {
                                         //     out_range[i] = std::make_pair(-1, -1);
                                         // }
                                     } },
                                 id);
        }

        for (int id = 0; id < thread_temp; id++)
        {
            threads[id].join();
        }
    }

    in_block_map.finalize_preprocessing();
    out_block_map.finalize_preprocessing();

    auto end_map = std::chrono::high_resolution_clock::now();
    cout << times << "预处理（分块映射表时间）: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_map - start_map).count() << "ms" << endl;

    // cout << times << " 开始计算出边分块索引" << endl;

    // vector<vector<BinaryRange>> out_part_ranges(g2.numNodes());
    // BinaryRange *out_part_ranges = new BinaryRange[g2.numNodes() * part->size()];
    // {
    //     int thread_temp = 64;
    //     vector<thread> threads;
    //     for (int id = 0; id < thread_temp; id++)
    //     {
    //         threads.emplace_back([&](int thread_id)
    //                              {
    //                                  int size = g2.numNodes();
    //                                  int bg = size / thread_temp * thread_id;
    //                                  // if(idx < new_end)
    //                                  int ed = bg + size / thread_temp;
    //                                  if (size % thread_temp > thread_id)
    //                                  {
    //                                      bg += thread_id;
    //                                      ed += thread_id + 1;
    //                                  }
    //                                  else
    //                                  {
    //                                      bg += size % thread_temp;
    //                                      ed += size % thread_temp;
    //                                  }
    //                              },
    //                              id);
    //     }
    //     for (int id = 0; id < thread_temp; id++)
    //     {
    //         threads[id].join();
    //     }
    // }

    in_part_row_ptr.shrink_to_fit();
    in_part_values.shrink_to_fit();
    in_part_col_idx.shrink_to_fit();
    out_part_row_ptr.shrink_to_fit();
    out_part_values.shrink_to_fit();
    out_part_col_idx.shrink_to_fit();

    /**
     * 确实不会是两倍，因为这个还和节点数量相关
     * 最多就是边数量的两倍；一般达不到，因为这个还和可能多边属于一个顶点
     */
    // cout << "总插入数量" << times << endl;
    // in_block_map.finalize_preprocessing();
    // out_block_map.finalize_preprocessing();

    /**
     * 一次性完成数据的拷贝操作
     */

    // BlockPool<SearchData> mem_pool(40960, 0);
    // BlockPool<GraphMatch> pool_memory(1, 0);
    bind_to_core(32);
    numa_set_localalloc();
    cout << "开始拷贝数据图" << endl;

    /**
     * 销毁不必要的数据
     */
    std::vector<Node> ().swap(g2._nodes);
    std::vector<Edge> ().swap(g2._edges);
    DataGraph g_numa_1 = g2;
    Csr csr_numa_1 = csr;
    // DataGraph h_numa_1 = h;

    cout << "开始拷贝索引表" << endl;
    FlatHashMap in_block_map_numa_1 = in_block_map;
    FlatHashMap out_block_map_numa_1 = out_block_map;

    // BlockPool<SearchData> mem_numa_1(40960, 1);
    // BlockPool<GraphMatch> pool_mem_numa_1(1);
    // LockFreeQueue<GraphMatch> results_numa_1(&pool_mem_numa_1);
    vector<pair<int, int>> part_numa_1 = *part;
    bind_to_core(0);
    numa_set_localalloc();

    /**
     * 遍历所有请求的查询图
     * 通过-qf命令指定对应的文件夹
     */
    for (int i = 0; i < args.queryFnames().size(); i++)
    {
        /**
         * 给后台一些时间清理旧数据，避免频繁启动新线程
         */
        // this_thread::sleep_for(std::chrono::seconds(10));
        const string &queryFname = args.queryFnames()[i];

        // 加载查询图
        cout << "从文件 " << queryFname << " 加载查询图" << endl;
        // 读取点数据和边数据
        // DataGraph h = FileIO::loadGenericGDF(queryFname);
        DataGraph h = loadFile(queryFname);
        cout << h.nodes().size() << " 个节点, " << h.edges().size() << " 条边" << endl;
        // if (h.numEdges() < MAX_NUM_EDGES_FOR_DISP)
        //     h.disp(); // 显示图的详细信息
        cout << endl;

        // 检查节点和边属性定义是否匹配
        // if (h.nodeAttributesDef() != g.nodeAttributesDef())
        //     throw "查询图和数据图之间的节点属性定义不匹配。";
        // if (h.edgeAttributesDef() != h.edgeAttributesDef())
        //     throw "查询图和数据图之间的边属性定义不匹配。";

        // 创建匹配条件并过滤数据图以提高查询性能，默认不匹配权重边
        // 过滤边仅仅过滤的是属性不匹配且时间越界的，这歌不存在，默认读取的全部都符合要求

        std::cout << "开始进行查询图模式分析" << std::endl;

        Precoing precoing;
        std::vector<Minfo> minfos = precoing.getMinfo(h);
        std::cout << "预处理结果" << std::endl;
        for (auto &minfo : minfos)
        {
            std::cout << "baseNode:" << minfo.baseNode << " constraintNode:" << minfo.constraintNode << " io:" << minfo.io << " mappedNodes:" << minfo.mappedNodes << " newNode:" << minfo.newNode << std::endl;
        }

        /**
         * 进行顶点扩展顺序处理
         */
        int nodes[5] = {0};
        auto edge_t = h.edges()[0];
        int idx = 0;
        nodes[idx++] = edge_t.source();
        nodes[idx++] = edge_t.dest();
        for (int i = 1; i < minfos.size(); i++)
        {
            if (minfos[i].newNode == 1)
            {
                nodes[idx++] = -minfos[i].constraintNode;
            }
            else if (minfos[i].newNode == 2)
            {
                nodes[idx++] = minfos[i].constraintNode;
                nodes[idx++] = minfos[i].baseNode;
            }
        }

        /**
         * 节点的扩展顺序如下
         */
        cout << "节点扩展情况如下" << endl;
        for (auto &t : nodes)
        {
            cout << t << " ";
        }
        cout << endl;

        /**
         * 将过滤后的图按照L3cache大小进行分块
         */
        // 启动线程数量
        int thread_num = 64;
        // 一次性申请30GB内存
        size_t POOL_SIZE = 30L * 1024 * 1024 * 1024;
        ThreadPool pool(thread_num / 2, 0);
        /**
         * 一次性申请102400块，在0号NUMA节点申请
         */
        BlockPool<SearchData> mem_pool(40960, 0);
        BlockPool<GraphMatch> pool_memory(1, 0);
        LockFreeQueue<GraphMatch> results(&pool_memory);
        // results.pool = &pool_memory;

        /**
         * 进行分块二分的预备工作
         * 对于每个顶点都遍历下对应的出入边集合，然后生成对应的分块集合
         * 先解决入边的分块，再解决出边的分块
         * in_part_ranges内部的每个元素都表示当前点在不同块的出入边范围
         */
        // vector<vector<BinaryRange>> in_part_ranges(g2.numNodes());
        // BinaryRange *in_part_ranges = new BinaryRange[g2.numNodes() * part->size()];

        /**
         * 以太访数据集点数量过于庞大，不方便先coo再csr；还是应该直接csr进行处理
         */
        // BinaryRange *in_part_ranges = static_cast<BinaryRange *>(
        //     numa_alloc_onnode(sizeof(BinaryRange) * g2.numNodes() * part->size(), 0));
        // BinaryRange *out_part_ranges = static_cast<BinaryRange *>(
        //     numa_alloc_onnode(sizeof(BinaryRange) * g2.numNodes() * part->size(), 0));

        /**
         * 打印csr
         */
        // cout << "打印csr" << endl;
        // for_each(in_part_values.begin(), in_part_values.end(), [](int i) { cout << i << " "; });
        // cout << endl;
        // for_each(in_part_col_idx.begin(), in_part_col_idx.end(), [](int i) { cout << i << " "; });
        // cout << endl;
        // for_each(in_part_row_ptr.begin(), in_part_row_ptr.end(), [](int i) { cout << i << " "; });
        // cout << endl;F
        // cout << "打印csr" << endl;
        // for_each(out_part_values.begin(), out_part_values.end(), [](int i) { cout << i << " "; });
        // cout << endl;
        // for_each(out_part_col_idx.begin(), out_part_col_idx.end(), [](int i) { cout << i << " "; });
        // cout << endl;
        // for_each(out_part_row_ptr.begin(), out_part_row_ptr.end(), [](int i) { cout << i << " "; });
        // cout << endl;

        // 遍历所有请求的时间差值限制
        vector<int> deltaCounts; // 存储每个时间差值下的子图计数
        for (time_t delta : args.deltaValues())
        {
            cout << "使用时间差值 = " << delta << endl;

            // 在更大的数据图中搜索查询图
            cout << "在更大的数据图中搜索查询图" << endl;
            int limit = INT_MAX; // 不限制结果数量

            // 多线程：定义全局索引
            int edge_idx = 0;
            // 全局队列锁
            mutex mtx;
            // 全局结果锁
            mutex res_mtx;

            // 多线程：定义vector数组存储多线程各自需要的变量
            // vector<GraphMatch> results;
            vector<GraphSearch> sears;
            vector<thread> threads;
            // vector<int> vec = store(g2, h, criteria);

            // 全局任务队列，每个块对应一个，存储每个块的旧任务
            vector<deque<GraphSearch>> queues(part->size());

            // 修改任务队列为无锁安全队列实现
            vector<LockFreeQueue<SearchData>> queues_lockfree(part->size());
            // queues_lockfree.reserve(part->size());
            for (int i = 0; i < part->size(); i++)
            {
                queues_lockfree[i].init(&mem_pool);
            }

            // 全局任务队列对应的锁
            vector<mutex> mutexes(part->size());

            queue<GraphSearch> q;
            vector<vector<int>> temp_res;

            // 创建一个全局有序边索引列表，直接保留一部分
            vector<int> all_edge_idxs(10);
            for (int i = 0; i < 10; i++)
            {
                all_edge_idxs[i] = i;
            }

            // cout << "总共分为" << part->size() << "块" << endl;
            pool.pool_memory = &mem_pool;
            // pool.in_binary_range = in_part_ranges;
            // pool.out_binary_range = out_part_ranges;
            pool.in_part_col_idx = &in_part_col_idx;
            pool.in_part_values = &in_part_values;
            pool.in_part_row_ptr = &in_part_row_ptr;
            pool.in_range = in_range;
            pool.out_part_col_idx = &out_part_col_idx;
            pool.out_part_values = &out_part_values;
            pool.out_part_row_ptr = &out_part_row_ptr;
            pool.out_range = out_range;
            pool.in_block_map = &in_block_map;
            pool.out_block_map = &out_block_map;

            pool.minfos = &minfos;
            /**
             * 处理对应分块前半部分的所有新数据和生成的所有旧数据
             * 只有小于new_end才处理当前块的新任务
             */
            pool.start_part = 0;
            pool.end_part = part->size();
            pool.new_end = part->size() / 2 - 1;
            pool.nodes = nodes;
            pool.numa_id = 0;
            pool.csr = &csr;

            pool.init(&temp_res, part, block_size, i, g2, h, criteria, limit, delta, nullptr, &mutexes, &res_mtx, &results, &queues_lockfree, &all_edge_idxs, &pool);

            /**
             * 转换绑定的逻辑核心
             */
            bind_to_core(32);
            numa_set_localalloc();

            cout << "开始拷贝数据到NUMA1节点" << endl;

            ThreadPool pool_numa_1(thread_num / 2, 1);
            BlockPool<SearchData> mem_numa_1(40960, 1);
            BlockPool<GraphMatch> pool_mem_numa_1(1);
            LockFreeQueue<GraphMatch> results_numa_1(&pool_mem_numa_1);
            // vector<pair<int, int>> part_numa_1 = *part;
            // vector<vector<BinaryRange>> in_part_ranges_numa_1 = in_part_ranges;
            // vector<vector<BinaryRange>> out_part_ranges_numa_1 = out_part_ranges;

            /**
             * 这个每次都提前析构了，后续可能得修改下，不然会double free
             */
            vector<LockFreeQueue<SearchData>> queues_lockfree_numa_1(part_numa_1.size());
            vector<int> all_edge_idxs_numa_1 = all_edge_idxs;

            // cout << "开始拷贝数据图" << endl;
            // DataGraph g_numa_1 = g2;
            DataGraph h_numa_1 = h;

            // cout << "开始拷贝索引表" << endl;
            // BinaryRange *in_part_ranges_numa_1 = new BinaryRange[part_numa_1.size() * g_numa_1.numNodes()];
            // BinaryRange *out_part_ranges_numa_1 = new BinaryRange[part_numa_1.size() * g_numa_1.numNodes()];
            // BinaryRange *in_part_ranges_numa_1 = static_cast<BinaryRange *>(
            //     numa_alloc_onnode(sizeof(BinaryRange) * g2.numNodes() * part->size(), 1));
            // BinaryRange *out_part_ranges_numa_1 = static_cast<BinaryRange *>(
            //     numa_alloc_onnode(sizeof(BinaryRange) * g2.numNodes() * part->size(), 1));

            // for (int i = 0; i < g_numa_1.numNodes(); i++)
            // {
            //     for (int j = 0; j < part_numa_1.size(); j++)
            //     {
            //         in_part_ranges_numa_1[i * part_numa_1.size() + j] = in_part_ranges[i * part_numa_1.size() + j];
            //         out_part_ranges_numa_1[i * part_numa_1.size() + j] = out_part_ranges[i * part_numa_1.size() + j];
            //     }
            // }
            vector<int> in_part_values_numa_1 = in_part_values;
            vector<int> in_part_col_idx_numa_1 = in_part_col_idx;
            vector<int> in_part_row_ptr_numa_1 = in_part_row_ptr;
            std::pair<int, int> *in_range_numa_1 = nullptr;
            vector<int> out_part_values_numa_1 = out_part_values;
            vector<int> out_part_col_idx_numa_1 = out_part_col_idx;
            vector<int> out_part_row_ptr_numa_1 = out_part_row_ptr;
            std::pair<int, int> *out_range_numa_1 = nullptr;
            // FlatHashMap in_block_map_numa_1 = in_block_map;
            // FlatHashMap out_block_map_numa_1 = out_block_map;

            // memcpy(in_range_numa_1, in_range, sizeof(std::pair<int, int>) * g2.numNodes());
            // memcpy(out_range_numa_1, out_range, sizeof(std::pair<int, int>) * g2.numNodes());

            // memcpy(in_part_ranges_numa_1, in_part_ranges, sizeof(BinaryRange) * g_numa_1.numNodes() * part_numa_1.size());
            // memcpy(out_part_ranges_numa_1, out_part_ranges, sizeof(BinaryRange) * g_numa_1.numNodes() * part_numa_1.size());

            cout << "拷贝完成" << endl;
            vector<mutex> mutexes_numa_1(part->size());
            queue<GraphSearch> q_numa_1;
            vector<vector<int>> temp_res_numa_1;
            MatchCriteria_DataGraph criteria_numa_1 = criteria;
            mutex res_mtx_numa_1;
            vector<Minfo> minfos_numa_1 = minfos;
            for (int i = 0; i < part_numa_1.size(); i++)
            {
                queues_lockfree_numa_1[i].init(&mem_numa_1);
            }

            pool_numa_1.pool_memory = &mem_numa_1;
            // pool_numa_1.in_binary_range = in_part_ranges_numa_1;
            // pool_numa_1.out_binary_range = out_part_ranges_numa_1;
            pool_numa_1.in_part_col_idx = &in_part_col_idx_numa_1;
            pool_numa_1.in_part_values = &in_part_values_numa_1;
            pool_numa_1.in_part_row_ptr = &in_part_row_ptr_numa_1;
            pool_numa_1.in_range = in_range_numa_1;
            pool_numa_1.out_part_col_idx = &out_part_col_idx_numa_1;
            pool_numa_1.out_part_values = &out_part_values_numa_1;
            pool_numa_1.out_range = out_range_numa_1;
            pool_numa_1.out_part_row_ptr = &out_part_row_ptr_numa_1;

            pool_numa_1.minfos = &minfos_numa_1;
            pool_numa_1.new_end = part_numa_1.size();
            pool_numa_1.in_block_map = &in_block_map_numa_1;
            pool_numa_1.out_block_map = &out_block_map_numa_1;
            /**
             * 处理分块后一半的所有新数据和对应新数据生成的旧数据
             */
            pool_numa_1.start_part = 0;
            pool_numa_1.end_part = part_numa_1.size();
            pool_numa_1.nodes = nodes;
            pool_numa_1.numa_id = 1;
            pool_numa_1.csr = &csr_numa_1;
            pool_numa_1.init(&temp_res_numa_1, &part_numa_1, block_size, i, g_numa_1, h_numa_1, criteria_numa_1, limit, delta, nullptr, &mutexes_numa_1, &res_mtx_numa_1, &results_numa_1, &queues_lockfree_numa_1, &all_edge_idxs_numa_1, &pool_numa_1);

            pool.otherPool = &pool_numa_1;
            pool_numa_1.otherPool = &pool;
            /**
             * 统计下NUMA0和NUMA1最后的结束条件
             * 0号NUMA需要遍历完成所有分块
             * 1号只需要遍历遍历后半个分块
             */
            // int nums_numa_0 = thread_num / 2 * part_numa_1.size();
            // int nums_numa_1 = thread_num / 2 * (pool_numa_1.end_part - pool_numa_1.start_part);
            int nums_numa_0 = thread_num / 2;
            int nums_numa_1 = thread_num / 2;

            // 按照块进行处理
            // for (int i = 0; i < part->size(); i++)
            // {
            // cout << "第 " << i << " 块" << endl;
            /**
             * 队列不为空才可运行，可以确定目前q队列中元素一定全是同层级，并且计算了begin和end的，而且还可扩展;q_new和q_tmp队列同理
             * while循环完成后，可以确保q队列的元素一定是level == h_i；但在过程中不可确保
             * */
            // int add_size = 0;
            // 对于旧任务进行处理
            // if (!queues[i].empty())
            {
                // int edge_idx = 0;
                // std::mutex edge_mtx;

                // // std::cout << "创建GraphSearch对象" << std::endl;
                // vector<GraphSearch> sears;
                // for (int idx = 0; idx < thread_num; idx++)
                // {
                //     // std::cout << "创建对象前" << (&queues[i]) -> size() << std::endl;
                //     sears.push_back(GraphSearch());
                // }

                // // std::cout << "创建对象后" << (&queues[i]) -> size() << std::endl;

                // // std::cout << "Starting threads" << std::endl;
                // vector<thread> threads;
                // for (int idx = 0; idx < thread_num; idx++)
                // {
                //     // std::cout << "创建线程前" << (&queues[i]) -> size() << std::endl;
                //     threads.push_back(thread(&GraphSearch::findOrderedSubgraphs, &sears[idx], &queues[i], &edge_idx, &edge_mtx));
                // }

                // cout << "开始执行旧任务" << endl;
                // for (int idx = 0; idx < thread_num; idx++)
                // {
                //     threads[idx].join();
                // }
                // queues[i] = deque<GraphSearch>();
                // cout << "执行旧任务完成" << endl;
                // pool.setOld(i);

                // // cout << "开始等待旧任务执行" << endl;
                // while (true)
                // {
                //     if (pool.getCount() == thread_num)
                //     {
                //         break;
                //     }
                //     this_thread::sleep_for(std::chrono::nanoseconds(10));
                // }
                // cout << "旧任务执行完成" << endl;
            }

            /**
             * 处理新任务，这部分采用多线程轮流取起始边的方式处理
             */
            // for (int j = part->at(i).first; j < part->at(i).second; j++)
            // {
            //     GraphSearch g_s_new(part, block_size, j, g2, h, criteria, limit, delta, &(part->at(i)), &mutexes, &res_mtx, &results, &queues, &all_edge_idxs, &pool);
            //     pool.enqueueGraphSearch(g_s_new);
            // }

            // cout << "开始等待" << endl;
            // // 释放当前块的空间
            // pool.wait();
            // cout << "等待完成" << endl;

            /**
             * 尝试下采用线程直接处理新任务
             */
            // int edge_idx = part->at(i).first;
            // std::mutex edge_mtx;

            // // std::cout << "创建GraphSearch对象" << std::endl;
            // vector<GraphSearch> sears;
            // for (int idx = 0; idx < thread_num; idx++)
            // {
            //     sears.push_back(GraphSearch(&temp_res, part, block_size, 0, g2, h, criteria, limit, delta, &(part->at(i)), &mutexes, &res_mtx, &results, &queues, &all_edge_idxs, &pool));
            // }

            // // std::cout << "Starting threads" << std::endl;
            // vector<thread> threads;
            // for (int i = 0; i < thread_num; i++)
            // {
            //     threads.push_back(thread(&GraphSearch::findNewOrderedSubgraphs, &sears[i], &edge_idx, &edge_mtx));
            // }

            // cout << "开始执行新任务" << endl;
            // for (int i = 0; i < thread_num; i++)
            // {
            //     threads[i].join();
            // }
            // cout << "新任务执行完毕" << endl;
            // cout << "开始等待新任务执行" << endl;
            cout << nums_numa_0 << " " << nums_numa_1 << endl;

            cout << "启动任务" << endl;
            auto start = std::chrono::high_resolution_clock::now();
            pool.start();
            pool_numa_1.start();
            while (true)
            {
                if (pool_numa_1.getCount() == nums_numa_1)
                {
                    break;
                }
                this_thread::sleep_for(std::chrono::nanoseconds(10));
            }
            while (true)
            {
                if (pool.getCount() == nums_numa_0)
                {
                    break;
                }
                this_thread::sleep_for(std::chrono::nanoseconds(20));
            }

            // pool.wait();
            cout << "任务执行完成" << endl;
            // }

            auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Total time: " << duration.count() << " ms" << std::endl;

            // cout << "找到 " << results.size() << " 个匹配的子图。" << endl;

            cout << "找到 " << (pool.res_num + pool_numa_1.res_num) << " 个匹配的子图。" << endl;

            // cout << "窃取任务队列 " << (pool.old_steal + pool_numa_1.old_steal) << endl;
            // cout << "任务队列执行 " << (pool.old_process + pool_numa_1.old_process) << endl;

            // cout << "任务队列的维护时间,出入队时间:" << LockFreeQueue<SearchData>::total_times / 1000 << " ms" << std::endl;

            // cout << "单线程最多执行任务队列时间:" << pool.max_time / 1000 << " ms" << std::endl;

            // cout << "二分最多执行时间:" << pool.binary_t / 1000 << " ms" << std::endl;

            // delete part;

            // 打印最后结果
            // saveScala(args.graphFname(), queryFname, duration.count());

            // saveVectorsToFile(temp_res);
            /**
             * 将结果全部保留
             */
            saveRes(args.graphFname(), queryFname, duration.count(), pool.res_num + pool_numa_1.res_num, std::chrono::duration_cast<std::chrono::milliseconds>(end_map - start_map).count());

            /**
             * 统计各个线程的总执行时间
             */
            // uint64_t binary_prepare = pool.binary_prepare + pool_numa_1.binary_prepare;
            // uint64_t binary_search_time = pool.binary_search_time + pool_numa_1.binary_search_time;
            // uint64_t check_time = pool.check_time + pool_numa_1.check_time;
            // uint64_t trans_time = pool.trans_time + pool_numa_1.trans_time;

            // saveBreakDown(args.graphFname(), queryFname, duration.count() * 1000000, binary_prepare, binary_search_time, trans_time, check_time);

            // print_peak_memory();
            // deltaCounts.push_back(results.size());

            // cout << "最后的边索引: " << edge_idx << endl;

            // // 创建所有匹配子图的组合图
            // cout << "创建所有匹配子图的组合图" << endl;
            // DataGraph combo = g2.createSubGraph(results);
            // if (combo.numEdges() < MAX_NUM_EDGES_FOR_DISP)
            //     combo.disp(); // 显示组合图
            // cout << endl;

            // // 保存结果到文件
            // string outFname = args.outFname();
            // if (outFname.empty())
            //     outFname = args.createOutFname(args.graphFname(), queryFname, delta);
            // cout << "将结果保存到 " << outFname << endl;
            // FileIO::saveGenericGDF(combo, outFname);
        }
        queryDeltaCounts.push_back(deltaCounts);
        this_thread::sleep_for(chrono::seconds(5));
    }
    cout << "完成！\n"
         << endl;

    // // 如果有多个查询或多个时间差值，显示计数表
    // if (args.queryFnames().size() > 1 || args.deltaValues().size() > 1)
    // {
    //     cout << "查询,时间差值,计数" << endl;
    //     for (int qi = 0; qi < args.queryFnames().size(); qi++)
    //     {
    //         string query = FileIO::getFname(args.queryFnames()[qi]);
    //         for (int di = 0; di < args.deltaValues().size(); di++)
    //         {
    //             time_t delta = args.deltaValues()[di];
    //             cout << query << "," << delta << "," << queryDeltaCounts[qi][di] << endl;
    //         }
    //     }
    // }

    delete part;
    // }
    // catch (exception &e)
    // {
    //     cout << "发生错误: " << e.what() << endl;
    // }
    // catch (const char *msg)
    // {
    //     cout << "发生错误: " << msg << endl;
    // }
    // catch (...)
    // {
    //     cout << "发生未知异常。" << endl;
    // }
    return 0;
}