#include "LabeledWeightedGraph.h"
#include <iostream>

using namespace std;

/**
 * 添加一条带权重的边到图中。
 * @param u 边的起点节点索引。
 * @param v 边的终点节点索引。
 * @param dateTime 边的时间戳。
 * @param weight 边的权重值。
 */
void LabeledWeightedGraph::addWeightedEdge(int u, int v, time_t dateTime, double weight)
{
    Graph::addEdge(u, v, dateTime); // 调用基类方法添加边
    _timeEdgeWeightMap[dateTime].push_back(weight); // 将权重与时间戳关联存储
    
    // 确保节点标签数组足够大以容纳新节点
    if (max(u, v) >= _nodeLabels.size())
    {
        _nodeLabels.resize(max(u, v) + 1);
    }
}

/**
 * 添加一条无权重的边到图中，默认权重为 0.0。
 * @param u 边的起点节点索引。
 * @param v 边的终点节点索引。
 * @param dateTime 边的时间戳。
 */
void LabeledWeightedGraph::addEdge(int u, int v, time_t dateTime)
{
    Graph::addEdge(u, v, dateTime); // 调用基类方法添加边
    _timeEdgeWeightMap[dateTime].push_back(0.0); // 默认权重为 0.0
    
    // 确保节点标签数组足够大以容纳新节点
    if (max(u, v) >= _nodeLabels.size())
    {
        _nodeLabels.resize(max(u, v) + 1);
    }
}

/**
 * 复制另一张图中的边到当前图中。
 * @param edgeIndex 要复制的边的索引。
 * @param g 包含目标边的图。
 */
void LabeledWeightedGraph::copyEdge(int edgeIndex, const Graph& g)
{
    LabeledWeightedGraph &wg = (LabeledWeightedGraph&)g; // 强制转换为 LabeledWeightedGraph 类型
    const Edge &edge = g.edges()[edgeIndex]; // 获取目标边
    int u = edge.source(), v = edge.dest(); // 获取边的起点和终点
    this->addWeightedEdge(u, v, edge.time(), wg.getEdgeWeight(edgeIndex)); // 添加带权重的边
    
    // 确保节点标签也被更新
    if (this->getLabel(u).empty())
        this->addLabeledNode(u, wg.getLabel(u));
    if (this->getLabel(v).empty())
        this->addLabeledNode(v, wg.getLabel(v));
}

/**
 * 添加一个带标签的节点到图中。
 * @param v 节点索引。
 * @param label 节点的标签。
 */
void LabeledWeightedGraph::addLabeledNode(int v, std::string label)
{
    if (v < 0)
        throw "Vertices must be >= 0"; // 确保节点索引非负
    this->addNode(v); // 调用基类方法添加节点
    if (this->_nodeLabels.size() <= v)
    {
        _nodeLabels.resize(v + 1); // 确保节点标签数组足够大
    }
    _nodeLabels[v] = label; // 设置节点标签
    _nameMap[label] = v; // 更新标签到节点索引的映射
}

/**
 * 获取指定边的权重。
 * @param edgeIndex 边的索引。
 * @return 边的权重值。
 */
double LabeledWeightedGraph::getEdgeWeight(int edgeIndex) const
{ 
    return _edgeWeights[edgeIndex]; 
}

/**
 * 获取指定节点的标签。
 * @param nodeIndex 节点索引。
 * @return 节点的标签。
 */
const string &LabeledWeightedGraph::getLabel(int nodeIndex) const
{    
    return _nodeLabels[nodeIndex];
}

/**
 * 根据节点标签获取节点索引。
 * @param nodeLabel 节点的标签。
 * @return 节点的索引。
 */
int LabeledWeightedGraph::getIndex(const std::string &nodeLabel) const
{
    return _nameMap.find(nodeLabel)->second;
}

/**
 * 显示图的基本信息，包括节点数和边数，并逐条显示边的详细信息。
 */
void LabeledWeightedGraph::disp() const
{
    cout << numNodes() << " nodes" << endl; // 输出节点数
    cout << numEdges() << " edges:" << endl; // 输出边数
    for (int i = 0; i < numEdges(); i++) // 遍历所有边并显示
    {
        cout << "  ";
        disp(i);
    }
}

/**
 * 显示指定边的详细信息。
 * @param edgeIndex 边的索引。
 */
void LabeledWeightedGraph::disp(int edgeIndex) const
{
    const Edge &edge = edges()[edgeIndex]; // 获取边对象
    cout << "[" << edgeIndex << "] " << edge.source() << " " << getLabel(edge.source()) << " -> " 
         << edge.dest() << " " << getLabel(edge.dest()) << " (" << _edgeWeights[edgeIndex] << ")" << endl;
}

/**
 * 获取指定边的权重限制列表。
 * @param e 边的索引。
 * @return 权重限制列表。
 */
const vector<WeightRestriction> &LabeledWeightedGraph::getWeightRestrictions(int e) const
{
    return _weightRestricts.find(e)->second;
}

/**
 * 检查指定边是否存在权重限制。
 * @param e 边的索引。
 * @return 如果存在权重限制则返回 true，否则返回 false。
 */
bool LabeledWeightedGraph::hasWeightRestrictions(int e) const
{
    return _weightRestricts.find(e) != _weightRestricts.end();
}

/**
 * 为指定边添加权重限制。
 * @param e 边的索引。
 * @param restrict 权重限制对象。
 */
void LabeledWeightedGraph::addWeightRestriction(int e, const WeightRestriction &restrict)
{
    _weightRestricts[e].push_back(restrict);
}

/**
 * 更新按时间顺序排列的边列表。
 */
void LabeledWeightedGraph::updateOrderedEdges() const
{
    Graph::updateOrderedEdges(); // 确保基类的边列表已更新
    
    _edgeWeights.clear(); // 清除旧的边权重列表
    
    // 按时间戳顺序构建边列表
    for (auto &pair : _timeEdgeWeightMap)
    {
        time_t dateTime = pair.first; // 时间戳
        const vector<double> &weights = _timeEdgeWeightMap.find(dateTime)->second; // 获取对应时间戳的权重列表
        for (double w : weights)
        {
            _edgeWeights.push_back(w); // 按时间顺序添加权重
        }
    }
}