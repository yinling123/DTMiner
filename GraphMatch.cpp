#include "GraphMatch.h"
#include <iostream>

using namespace std;

/**
 * @brief 添加一条边及其映射关系到图匹配对象中。
 * @param gEdge 源图中的边。
 * @param hEdge 查询图中的边。
 */
void GraphMatch::addEdge(const Edge &gEdge, const Edge &hEdge)
{
    int g_i = gEdge.index(), h_i = hEdge.index(); // 获取两条边的索引
    _edges.push_back(g_i); // 将源图边的索引添加到边列表中
    _gEdge2hEdgeMap[g_i] = h_i; // 建立源图边到查询图边的映射

    int g_u = gEdge.source(), g_v = gEdge.dest(); // 获取源图边的起点和终点
    int h_u = hEdge.source(), h_v = hEdge.dest(); // 获取查询图边的起点和终点

    _gNode2hNodeMap[g_u] = h_u; // 建立源图节点到查询图节点的映射
    _gNode2hNodeMap[g_v] = h_v; // 同上
    _nodes.push_back(g_u); // 将源图节点添加到节点列表中
    _nodes.push_back(g_v); // 同上
    _nodeSet.insert(g_u); // 将源图节点添加到节点集合中
    _nodeSet.insert(g_v); // 同上
}

/**
 * @brief 检查当前图匹配对象中是否包含指定节点。
 * @param u 要检查的节点索引。
 * @return 如果节点存在则返回 true，否则返回 false。
 */
bool GraphMatch::hasNode(int u) const
{
    return _nodeSet.find(u) != _nodeSet.end(); // 在节点集合中查找是否存在该节点
}

/**
 * @brief 显示当前图匹配对象的内容，包括节点集合和节点列表。
 */
void GraphMatch::disp() const
{
    cout << "NodeSet = "; // 输出节点集合
    for (int u : _nodeSet)
        cout << u << " "; // 遍历并打印节点集合中的每个节点
    cout << endl;

    cout << "Nodes = "; // 输出节点列表
    for (int u : _nodes)
        cout << u << " "; // 遍历并打印节点列表中的每个节点
    cout << endl;
}