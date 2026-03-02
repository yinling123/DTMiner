#include "MatchCriteria_CERT.h"
#include "CertGraph.h"
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
bool MatchCriteria_CERT::isEdgeMatch(const Graph& g, int gEdgeIndex, const Graph& h, int hEdgeIndex) const
{
    // 首先调用基类的边匹配方法
    if (MatchCriteria::isEdgeMatch(g, gEdgeIndex, h, hEdgeIndex) == false)
        return false;

    CertGraph &cg = (CertGraph&)g; // 将数据图转换为 CertGraph 类型
    CertGraph &ch = (CertGraph&)h; // 将查询图转换为 CertGraph 类型

    // 检查边类型
    const string &hEdgeType = ch.getEdgeType(hEdgeIndex); // 获取查询图中边的类型
    if (hEdgeType.empty() == false) // 仅在查询图中边类型非空时检查
    {
        const string &gEdgeType = cg.getEdgeType(gEdgeIndex); // 获取数据图中边的类型
        if (gEdgeType.compare(hEdgeType) != 0) // 比较边类型是否一致
            return false;
    }

    // 获取查询图和数据图中边的源节点和目标节点
    const Edge &hEdge = ch.edges()[hEdgeIndex];
    int hSource = hEdge.source();
    int hDest = hEdge.dest();

    const Edge &gEdge = cg.edges()[gEdgeIndex];
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
bool MatchCriteria_CERT::isNodeMatch(const Graph &g, int gNodeIndex, const Graph &h, int hNodeIndex) const
{
    // 首先调用基类的节点匹配方法
    if (MatchCriteria::isNodeMatch(g, gNodeIndex, h, hNodeIndex) == false)
        return false;

    CertGraph &cg = (CertGraph&)g; // 将数据图转换为 CertGraph 类型
    CertGraph &ch = (CertGraph&)h; // 将查询图转换为 CertGraph 类型

    // 检查节点名称（如果需要）
    if (ch.needsNameMatch(hNodeIndex)) // 查询图中是否需要名称匹配
    {
        if (ch.getLabel(hNodeIndex) != cg.getLabel(gNodeIndex)) // 比较节点名称
            return false;
    }

    // 检查节点类型
    const string &hType = ch.getNodeType(hNodeIndex); // 获取查询图中节点的类型
    if (hType.empty() == false) // 仅在查询图中节点类型非空时检查
    {
        const string &gType = cg.getNodeType(gNodeIndex); // 获取数据图中节点的类型
        if (gType.compare(hType) != 0) // 比较节点类型是否一致
            return false;
    }

    // 检查度数限制
    if (ch.hasDegRestrictions(hNodeIndex)) // 查询图中是否有度数限制
    {
        const vector<DegRestriction> &restricts = ch.getDegRestrictions(hNodeIndex); // 获取度数限制列表
        for (const DegRestriction &restrict : restricts) // 遍历每个度数限制
        {
            int deg = 0;
            if (restrict.isOutDeg()) // 检查出度
                deg = cg.getOutDeg(gNodeIndex, restrict.edgeType());
            else // 检查入度
                deg = cg.getInDeg(gNodeIndex, restrict.edgeType());

            if (restrict.isLessThan()) // 检查是否小于限制值
            {
                if (deg >= restrict.value())
                    return false;
            }
            else // 检查是否大于限制值
            {
                if (deg <= restrict.value())
                    return false;
            }
        }
    }

    // 检查正则表达式限制
    if (ch.needsRegexMatch(hNodeIndex)) // 查询图中是否需要正则表达式匹配
    {
        const regex &rx = ch.getRegex(hNodeIndex); // 获取查询图中节点的正则表达式
        if (regex_search(cg.getLabel(gNodeIndex), rx) == false) // 检查数据图中节点标签是否匹配正则表达式
            return false;
    }

    // 检查相邻的出边类型
    const unordered_set<string> &gOutTypes = cg.getOutEdgeTypes(gNodeIndex); // 获取数据图中节点的出边类型集合
    const unordered_set<string> &hOutTypes = ch.getOutEdgeTypes(hNodeIndex); // 获取查询图中节点的出边类型集合
    for (const string &type : hOutTypes) // 遍历查询图中的出边类型
    {
        if (!type.empty()) // 仅在类型非空时检查
        {
            if (gOutTypes.find(type) == gOutTypes.end()) // 检查数据图中是否存在该类型
                return false;
        }
    }

    // 检查相邻的入边类型
    const unordered_set<string> &gInTypes = cg.getInEdgeTypes(gNodeIndex); // 获取数据图中节点的入边类型集合
    const unordered_set<string> &hInTypes = ch.getInEdgeTypes(hNodeIndex); // 获取查询图中节点的入边类型集合
    for (const string &type : hInTypes) // 遍历查询图中的入边类型
    {
        if (!type.empty()) // 仅在类型非空时检查
        {
            if (gInTypes.find(type) == gInTypes.end()) // 检查数据图中是否存在该类型
                return false;
        }
    }

    // 如果通过所有测试，则认为节点匹配
    return true;
}