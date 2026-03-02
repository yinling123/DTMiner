#include "MatchCriteria_DataGraph.h"
#include "DataGraph.h"
#include <iostream>

using namespace std;

/**
 * 判断两条边是否匹配。
 * @param g 数据图。
 * @param gEdgeIndex 数据图中的边索引。
 * @param h 查询图。
 * @param hEdgeIndex 查询图中的边索引。
 * @return 如果匹配则返回 true，否则返回 false。
 */
bool MatchCriteria_DataGraph::isEdgeMatch(const Graph& g, int gEdgeIndex, const Graph& h, int hEdgeIndex) const
{
    // 首先调用基类的边匹配方法
    if (MatchCriteria::isEdgeMatch(g, gEdgeIndex, h, hEdgeIndex) == false)
        return false;

    DataGraph &dg = (DataGraph&)g; // 将数据图转换为 DataGraph 类型
    DataGraph &dh = (DataGraph&)h; // 将查询图转换为 DataGraph 类型

    // 检查边属性是否匹配
    if (this->doAttributesMatch(dg.edgeAttributes()[gEdgeIndex], dh.edgeAttributes()[hEdgeIndex]) == false)
        return false;

    // 获取查询图和数据图中边的源节点和目标节点
    const Edge &hEdge = dh.edges()[hEdgeIndex];
    int hSource = hEdge.source();
    int hDest = hEdge.dest();

    const Edge &gEdge = dg.edges()[gEdgeIndex];
    int gSource = gEdge.source();
    int gDest = gEdge.dest();

    // 检查源节点和目标节点是否匹配
    if (!isNodeMatch(g, gSource, h, hSource)) // 源节点匹配检查
        return false;
    if (!isNodeMatch(g, gDest, h, hDest)) // 目标节点匹配检查
        return false;

    // 如果通过所有测试，则认为边匹配
    return true;
}

/**
 * 判断两个节点是否匹配。
 * @param g 数据图。
 * @param gNodeIndex 数据图中的节点索引。
 * @param h 查询图。
 * @param hNodeIndex 查询图中的节点索引。
 * @return 如果匹配则返回 true，否则返回 false。
 */
bool MatchCriteria_DataGraph::isNodeMatch(const Graph &g, int gNodeIndex, const Graph &h, int hNodeIndex) const
{
    // 首先调用基类的节点匹配方法
    if (MatchCriteria::isNodeMatch(g, gNodeIndex, h, hNodeIndex) == false)
        return false;

    DataGraph &dg = (DataGraph&)g; // 将数据图转换为 DataGraph 类型
    DataGraph &dh = (DataGraph&)h; // 将查询图转换为 DataGraph 类型

    // 检查节点属性是否匹配
    if (this->doAttributesMatch(dg.nodeAttributes()[gNodeIndex], dh.nodeAttributes()[hNodeIndex]) == false)
        return false;

    // TODO: 可能还需要检查相邻边（类似于 CERT MatchCriteria 的实现）

    // 如果通过所有测试，则认为节点匹配
    return true;
}

/**
 * 检查两组属性是否匹配。
 * @param a1 第一组属性。
 * @param a2 第二组属性。
 * @return 如果匹配则返回 true，否则返回 false。
 */
bool MatchCriteria_DataGraph::doAttributesMatch(const Attributes& a1, const Attributes& a2) const
{
    // 检查两组属性的数量是否一致
    if (a1.floatValues().size() != a2.floatValues().size() ||
        a1.intValues().size() != a2.intValues().size() ||
        a1.stringValues().size() != a2.stringValues().size())
    {
        cerr << "第一组属性:" << endl;
        cerr << a1.floatValues().size() << " 个浮点值" << endl;
        cerr << a1.intValues().size() << " 个整数值" << endl;
        cerr << a1.stringValues().size() << " 个字符串值" << endl;
        cerr << "第二组属性:" << endl;
        cerr << a2.floatValues().size() << " 个浮点值" << endl;
        cerr << a2.intValues().size() << " 个整数值" << endl;
        cerr << a2.stringValues().size() << " 个字符串值" << endl;
        throw "无法比较属性。每种类型的属性数量不匹配！";
    }

    // 检查浮点值是否匹配
    const auto &f1 = a1.floatValues();
    const auto &f2 = a2.floatValues();
    for (int i = 0; i < f1.size(); i++)
    {
        float x1 = f1[i], x2 = f2[i];
        FloatRestrictions r2 = a2.floatRestrictions()[i]; // 获取浮点值的限制条件
        if (r2.any()) // 如果允许任意值，则跳过检查
            continue;
        if (r2.hasMin() && x1 < r2.min()) // 检查最小值限制
            return false;
        if (r2.hasMax() && x1 > r2.max()) // 检查最大值限制
            return false;
        if (r2.exact() && x1 != x2) // 检查精确值匹配
            return false;
    }

    // 检查整数值是否匹配
    const auto &i1 = a1.intValues();
    const auto &i2 = a2.intValues();
    for (int i = 0; i < i1.size(); i++)
    {
        int x1 = i1[i], x2 = i2[i];
        IntRestrictions r2 = a2.intRestrictions()[i]; // 获取整数值的限制条件
        if (r2.any()) // 如果允许任意值，则跳过检查
            continue;
        if (r2.hasMin() && x1 < r2.min()) // 检查最小值限制
            return false;
        if (r2.hasMax() && x1 > r2.max()) // 检查最大值限制
            return false;
        if (r2.exact() && x1 != x2) // 检查精确值匹配
            return false;
    }

    // 检查字符串值是否匹配
    const auto &s1 = a1.stringValues();
    const auto &s2 = a2.stringValues();
    for (int i = 0; i < s1.size(); i++)
    {
        StringRestrictions r2 = a2.stringRestrictions()[i]; // 获取字符串值的限制条件
        if (r2.any()) // 如果允许任意值，则跳过检查
            continue;
        if (r2.exact() && s1[i] != s2[i]) // 检查精确值匹配
            return false;
    }

    // 如果通过所有测试，则认为属性匹配
    return true;
}