#include "Graph.h"
#include "Node.h"
#include <algorithm>
#include <iostream>
#include <time.h>
#include <limits.h>

using namespace std;

/**
 * @brief 构造函数，初始化图对象。
 * @param windowDuration 时间窗口的持续时间（秒）。
 */
Graph::Graph(int windowDuration)
{
    _edgesReady = false; // 边是否已排序并准备好使用
    _windowDuration = windowDuration; // 时间窗口的持续时间
    _windowStart = 0; // 时间窗口的起始时间
    _windowEnd = 0; // 时间窗口的结束时间
    if (_windowDuration == 0) // 如果窗口持续时间为0，则设置为最大时间
    {
        _windowEnd = LONG_MAX;
    }
}

/**
 * @brief 添加节点到图中。
 * @param v 节点的索引。
 */
void Graph::addNode(int v)
{
    if (v >= _nodes.size()) // 如果节点索引超出当前范围，则调整大小
        _nodes.resize(v + 1);
}

/**
 * @brief 添加一条边，默认时间为当前边的数量。
 * @param u 源节点索引。
 * @param v 目标节点索引。
 */
void Graph::addEdge(int u, int v)
{
    this->addEdge(u, v, (time_t)_numEdges); // 默认时间是当前边的数量
}

/**
 * @brief 添加一条带时间戳的边。
 * @param u 源节点索引。
 * @param v 目标节点索引。
 * @param dateTime 边的时间戳。
 */
void Graph::addEdge(int u, int v, time_t dateTime)
{
    if (u < 0 || v < 0) // 确保节点索引非负
        throw "Vertices must be >= 0";

    int min_n = std::max(u, v) + 1; // 计算最小节点数量
    if (_nodes.size() < min_n) // 如果节点数量不足，则调整大小
        _nodes.resize(min_n);

    // 将边按时间存储到映射中
    _timeEdgeMap[dateTime].push_back(Edge(_numEdges, u, v, dateTime));
    _numEdges++; // 增加边计数

    _edgesReady = false; // 标记边需要重新排序

    // 更新时间窗口的起始和结束时间
    if (dateTime > _windowEnd)
    {
        _windowEnd = dateTime;
        _windowStart = _windowEnd - _windowDuration;
    }
}

/**
 * @brief 复制另一张图中的边。
 * @param edgeIndex 要复制的边索引。
 * @param g 包含边的源图。
 */
void Graph::copyEdge(int edgeIndex, const Graph &g)
{
    const Edge &edge = g.edges()[edgeIndex]; // 获取源图中的边
    this->addEdge(edge.source(), edge.dest(), edge.time()); // 添加到当前图中
}

/**
 * @brief 检查图中是否存在从 u 到 v 的边。
 * @param u 源节点索引。
 * @param v 目标节点索引。
 * @return 如果存在边则返回 true，否则返回 false。
 */
bool Graph::hasEdge(int u, int v) const
{
    if (!_edgesReady) // 如果边未排序，则先更新
        this->updateOrderedEdges();

    return _nodeEdges.find(u) != _nodeEdges.end() && _nodeEdges[u].find(v) != _nodeEdges[u].end();
}

/**
 * @brief 获取从 u 到 v 的所有边索引。
 * @param u 源节点索引。
 * @param v 目标节点索引。
 * @return 包含边索引的向量。
 */
const vector<int> &Graph::getEdgeIndexes(int u, int v) const
{
    if (!_edgesReady) // 如果边未排序，则先更新
        this->updateOrderedEdges();

    if (!hasEdge(u, v)) // 如果不存在边，则抛出异常
        throw "There are no edges between the vertices selected.";

    return _nodeEdges[u][v];
}

/**
 * @brief 获取图中的所有边。
 * @return 包含所有边的向量。
 */
const vector<Edge> &Graph::edges() const
{
    // cache缺失严重
    if (!_edgesReady) // 如果边未排序，则先更新
    {
        this->updateOrderedEdges();
    }
    return _edges;
}

/**
 * @brief 显示图的基本信息（节点数和边数）。
 */
void Graph::disp() const
{
    if (!_edgesReady) // 如果边未排序，则先更新
        this->updateOrderedEdges();

    cout << _nodes.size() << " nodes" << endl; // 输出节点数
    cout << _edges.size() << " edges:" << endl; // 输出边数
    for (int i = 0; i < _edges.size(); i++) // 遍历所有边并显示
    {
        cout << "  ";
        disp(i);
    }
}

/**
 * @brief 显示指定索引的边信息。
 * @param edgeIndex 边的索引。
 */
void Graph::disp(int edgeIndex) const
{
    if (!_edgesReady) // 如果边未排序，则先更新
        this->updateOrderedEdges();

    const Edge &edge = _edges[edgeIndex]; // 获取指定索引的边
    cout << "[" << edgeIndex << "] " << edge.source() << " -> " << edge.dest() << endl; // 输出边信息
}

/**
 * @brief 显示图的时间范围信息。
 */
void Graph::dispDateTimeRange() const
{
    time_t start = this->edges().front().time(); // 获取第一条边的时间
    struct tm *startTM = gmtime(&start); // 转换为可读格式
    cout << "Total Data Ingested Start Date/Time: (" << start << ") " << asctime(startTM);

    time_t end = this->edges().back().time(); // 获取最后一条边的时间
    struct tm *endTM = gmtime(&end); // 转换为可读格式
    cout << "Total Data Ingested End Date/Time: (" << end << ") " << asctime(endTM);

    time_t wStart = windowStart(); // 获取当前窗口的起始时间
    struct tm *wStartTM = gmtime(&wStart);
    cout << "Current Window Start Date/Time: (" << wStart << ") " << asctime(wStartTM);

    time_t wEnd = windowEnd(); // 获取当前窗口的结束时间
    struct tm *wEndTM = gmtime(&wEnd);
    cout << "Current Window End Date/Time: (" << wEnd << ") " << asctime(wEndTM);
}

/**
 * @brief 按时间顺序更新边的排序。
 */
void Graph::updateOrderedEdges() const
{
    int n = _nodes.size(); // 获取节点数量

    // 清空旧的边数据
    _edges.clear();
    for (Node &node : _nodes)
    {
        node.edges().clear();
        node.outEdges().clear();
        node.inEdges().clear();
    }

    // 按时间顺序构建边列表
    int edge_index = 0;
    for (auto &pair : _timeEdgeMap) // 遍历时间映射
    {
        time_t dateTime = pair.first; // 获取时间戳
        const vector<Edge> &edges = _timeEdgeMap.find(dateTime)->second; // 获取对应时间的边
        for (const Edge &edge : edges) // 遍历每条边
        {
            // 创建新边并添加到列表中
            Edge newEdge(edge_index, edge.source(), edge.dest(), edge.time());
            _edges.push_back(newEdge);
            // _edgeTimes.push_back(dateTime);

            int u = edge.source(), v = edge.dest(); // 获取源和目标节点

            // 更新节点的边信息
            // _nodes[u].edges().push_back(edge_index);
            // _nodes[v].edges().push_back(edge_index);
            _nodes[u].outEdges().push_back(edge_index);
            _nodes[v].inEdges().push_back(edge_index);

            edge_index++;
        }
    }

    // 更新节点边映射
    this->_nodeEdges.clear();
    // for (int u = 0; u < n; u++)
    // {
    //     const Node &node = _nodes[u];
    //     for (int e : node.outEdges()) // 遍历节点的出边
    //     {
    //         const Edge &edge = _edges[e];
    //         int v = edge.dest(); // 获取目标节点
    //         _nodeEdges[u][v].push_back(e); // 更新映射
    //     }
    // }

    /**
     * 排序完成必要的部分，就析构掉不必要的部分
    */
    std::cout << "开始过滤不必要的元素" << std::endl;
    std::unordered_map<int, std::unordered_map<int, std::vector<int>>> temp_nodeEdges;
    _nodeEdges.swap(temp_nodeEdges);
    std::vector<time_t> temp_edgeTimes;
    _edgeTimes.swap(temp_edgeTimes);
    std::map<time_t, std::vector<Edge>> temp_timeEdgeMap;
    _timeEdgeMap.swap(temp_timeEdgeMap);
    std::cout << "边已排序" << std::endl;
    
    _edgesReady = true; // 标记边已排序

    /**
     * 收缩node数组
    */
    for(Node &node : _nodes)
    {
        node.inEdges().shrink_to_fit();
        node.outEdges().shrink_to_fit();
    }
    _edges.shrink_to_fit();
}

/**
 * @brief 获取当前时间窗口的起始时间。
 * @return 当前窗口的起始时间。
 */
time_t Graph::windowStart() const
{
    time_t dataStart = _edgeTimes.front(); // 获取最早边的时间
    if (dataStart > _windowStart) // 如果最早边时间晚于窗口起始时间
        return dataStart;
    return _windowStart;
}

/**
 * @brief 获取当前时间窗口的结束时间。
 * @return 当前窗口的结束时间。
 */
time_t Graph::windowEnd() const
{
    time_t dataEnd = _edgeTimes.back(); // 获取最晚边的时间
    if (dataEnd < _windowEnd) // 如果最晚边时间早于窗口结束时间
        return dataEnd;
    return _windowEnd;
}

/**
 * @brief 设置时间窗口的持续时间。
 * @param duration 新的时间窗口持续时间。
 */
void Graph::setWindowDuration(int duration)
{
    if (duration == _windowDuration) // 如果持续时间未改变，则直接返回
        return;

    // 如果新的持续时间更短，则从末尾截断
    if (duration < _windowDuration)
        _windowStart = _windowEnd - duration;
    // 如果新的持续时间更长，则从前端扩展
    else
        _windowEnd = _windowStart + duration;

    _windowDuration = duration; // 更新窗口持续时间
}