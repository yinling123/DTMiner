#include "MatchCriteria_Weighted.h"
#include "LabeledWeightedGraph.h"
#include <iostream>

using namespace std;

/**
 * 添加一条边的最小权重要求。
 * @param h_i 查询图中边的索引。
 * @param minWeight 该边的最小权重值。
 */
void MatchCriteria_Weighted::addMinWeight(int h_i, double minWeight)
{
    this->_minWeights[h_i] = minWeight; // 将边索引与最小权重值存入映射表
}

/**
 * 判断两条边是否匹配，基于权重的最小值要求。
 * @param g 数据图（我们正在搜索的图）。
 * @param gEdgeIndex 数据图中的边索引。
 * @param h 查询图（我们正在查找的目标图）。
 * @param hEdgeIndex 查询图中的边索引。
 * @return 如果匹配则返回 true，否则返回 false。
 */
bool MatchCriteria_Weighted::isEdgeMatch(const Graph& g, int gEdgeIndex, const Graph& h, int hEdgeIndex) const
{
    // 首先调用基类的边匹配方法
    if (MatchCriteria::isEdgeMatch(g, gEdgeIndex, h, hEdgeIndex) == false)
        return false;

    // 如果查询图中的该边没有设置最小权重要求，则认为匹配成功
    if (_minWeights.find(hEdgeIndex) == _minWeights.end())
    {
        //cout << "No min weight for this edge, so it's acceptable" << endl;
        return true;
    }

    // 否则，检查数据图中的边是否满足最小权重要求
    //cout << "Casting to PajekGraph" << endl;
    LabeledWeightedGraph &wg = (LabeledWeightedGraph&)g; // 将数据图转换为带权重的图类型
    //cout << "Getting edge weight" << endl;
    double w = wg.getEdgeWeight(gEdgeIndex); // 获取数据图中边的权重
    //cout << "Weight = " << w << endl;
    double minW = _minWeights.find(hEdgeIndex)->second; // 获取查询图中边的最小权重要求
    //cout << "Min weight = " << minW << endl;

    // 如果数据图中的边权重大于等于最小权重要求，则认为匹配成功
    return w >= minW;
}