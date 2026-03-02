#include "GraphFilter.h"
#include "CertGraph.h"
#include <iostream>
#include <unordered_set>
#include <string>

using namespace std;

/**
 * @brief 根据匹配条件过滤图 g 的边，并将符合条件的边复制到 g2 中。
 * @param g 源图。
 * @param h 包含匹配条件的图。
 * @param criteria 匹配条件。
 * @param g2 过滤后的目标图。
 */
void GraphFilter::filter(const Graph& g, const Graph& h, const MatchCriteria& criteria, Graph &g2)
{
    cout << "Filtering graph" << endl;

    int g_n = g.numNodes(); // 获取源图的节点数
    int g_m = g.numEdges(); // 获取源图的边数

    int h_n = h.numNodes(); // 获取匹配条件图的节点数
    int h_m = h.numEdges(); // 获取匹配条件图的边数

    // 遍历源图的所有边
    for (int g_i = 0; g_i < g_m; g_i++)
    {
        // 遍历匹配条件图的所有边
        for (int h_i = 0; h_i < h_m; h_i++)
        {
           /**
            * 直接过滤掉自环的情况
           */
            // int source = g._edges[g_i].source();
            // int target = g._edges[g_i].dest();

            // if (source == target)
            // {
            //     break;
            // }
            // 如果找到匹配的边，则将其复制到目标图并停止搜索
            // 删除掉源图中超过时间戳要求的边，并且要求对应的属性要求
            if (criteria.isEdgeMatch(g, g_i, h, h_i))
            {
                g2.copyEdge(g_i, g); // 复制边
                break;
            }
        }
    }
}

/**
 * @brief 根据匹配条件过滤 CertGraph 图 g 的边，并将符合条件的边复制到 g2 中。
 * @param g 源图。
 * @param h 包含匹配条件的图。
 * @param criteria 匹配条件。
 * @param g2 过滤后的目标图。
 */
void GraphFilter::filter(const CertGraph &g, const CertGraph &h, const MatchCriteria &criteria, CertGraph &g2)
{
    cout << "Filtering graph" << endl;

    int g_n = g.numNodes(); // 获取源图的节点数
    int g_m = g.numEdges(); // 获取源图的边数

    int h_n = h.numNodes(); // 获取匹配条件图的节点数
    int h_m = h.numEdges(); // 获取匹配条件图的边数

    // 检查匹配条件图是否有度限制
    bool hasDegRes = false;
    for (int h_v = 0; h_v < h_n; h_v++)
    {
        if (h.hasDegRestrictions(h_v)) // 如果某个节点有度限制
        {
            hasDegRes = true;
            break;
        }
    }

    // 如果存在度限制，则使用不同的方法进行过滤
    if (hasDegRes)
    {
        bool useAllEdges = false; // 是否需要处理所有边类型
        unordered_set<string> edgeTypes; // 存储需要处理的边类型

        // 找到节点限制中使用的边类型
        for (int h_v = 0; h_v < h_n; h_v++)
        {
            if (h.hasDegRestrictions(h_v))
            {
                const vector<DegRestriction> &restricts = h.getDegRestrictions(h_v);
                for (const DegRestriction &restrict : restricts)
                {
                    edgeTypes.insert(restrict.edgeType()); // 插入边类型
                }
            }
        }

        // 添加匹配条件图中其他使用的边类型
        for (int h_e = 0; h_e < h_m; h_e++)
        {
            const string &type = h.getEdgeType(h_e);
            if (type.empty()) // 如果边类型为空，则需要处理所有边
                useAllEdges = true;
            else
                edgeTypes.insert(type); // 插入边类型
        }

        // 遍历源图的所有边
        for (int g_i = 0; g_i < g_m; g_i++)
        {
            const string &type = g.getEdgeType(g_i); // 获取当前边的类型
            if (useAllEdges || edgeTypes.find(type) != edgeTypes.end()) // 如果边类型符合要求
            {
                g2.copyEdge(g_i, g); // 复制边
            }
        }
    }
    else
    {
        // 如果没有度限制，则按常规方式过滤
        for (int g_i = 0; g_i < g_m; g_i++)
        {
            // 遍历匹配条件图的所有边
            for (int h_i = 0; h_i < h_m; h_i++)
            {
                // 如果找到匹配的边，则将其复制到目标图并停止搜索
                if (criteria.isEdgeMatch(g, g_i, h, h_i))
                {
                    g2.copyEdge(g_i, g); // 复制边
                    break;
                }
            }
        }
    }
}

/**
 * @brief 根据时间范围过滤 CertGraph 图 g 的边，并将符合条件的边复制到 g2 中。
 * @param g 源图。
 * @param start 时间范围的起始时间。
 * @param end 时间范围的结束时间。
 * @param g2 过滤后的目标图。
 */
void GraphFilter::filter(const CertGraph &g, time_t start, time_t end, CertGraph &g2)
{
    const vector<Edge> &edges = g.edges(); // 获取源图的所有边

    // 遍历所有边
    for (const Edge &edge : edges)
    {
        time_t t = edge.time(); // 获取边的时间戳
        // 如果边的时间在指定范围内，则复制到目标图
        if (t >= start && t <= end)
        {
            g2.copyEdge(edge.index(), g); // 复制边
        }
    }
}