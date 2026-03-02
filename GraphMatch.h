/* 
 * File:   GraphMatch.h
 * Author: D3M430
 *
 * Created on June 2, 2017, 11:21 AM
 */

#ifndef GRAPHMATCH_H
#define	GRAPHMATCH_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "Edge.h"

/**
 * @brief 存储原始图中与查询图匹配的边和节点列表。
 */
class GraphMatch
{
public:
    /**
     * @brief 添加一条匹配的边，表示原始图中的边与查询图中的边匹配。
     * @param gEdge 原始图中匹配的边。
     * @param hEdge 查询图中匹配的边。
     */
    void addEdge(const Edge &gEdge, const Edge &hEdge);

    /**
     * @brief 获取原始图中属于匹配子图的边索引列表。
     * @return 包含边索引的向量。
     */
    const std::vector<int> &edges() const { return _edges; }

    /**
     * @brief 获取原始图中属于匹配子图的节点索引列表。
     * @return 包含节点索引的向量。
     */
    const std::vector<int> &nodes() const { return _nodes; }

    /**
     * @brief 检查原始图中的指定节点是否属于当前匹配子图。
     * @param u 要检查的节点索引。
     * @return 如果节点存在则返回 true，否则返回 false。
     */
    bool hasNode(int u) const;

    /**
     * @brief 获取原始图中匹配边对应的查询图中的边索引。
     * @param gEdge 原始图中边的索引。
     * @return 查询图中对应边的索引。
     */
    int getQueryEdge(int gEdge) const { return _gEdge2hEdgeMap.find(gEdge)->second; }

    /**
     * @brief 获取原始图中匹配节点对应的查询图中的节点索引。
     * @param gNode 原始图中节点的索引。
     * @return 查询图中对应节点的索引。
     */
    int getQueryNode(int gNode) const { return _gNode2hNodeMap.find(gNode)->second; }

    /**
     * @brief 显示 GraphMatch 的内容，用于测试。
     */
    void disp() const;

private:
    std::vector<int> _edges; // 原始图中匹配子图的边索引列表
    std::vector<int> _nodes; // 原始图中匹配子图的节点索引列表
    std::unordered_set<int> _nodeSet; // 原始图中匹配子图的节点集合（用于快速查找）
    std::unordered_map<int, int> _gEdge2hEdgeMap; // 原始图边到查询图边的映射
    std::unordered_map<int, int> _gNode2hNodeMap; // 原始图节点到查询图节点的映射
};

#endif	/* GRAPHMATCH_H */