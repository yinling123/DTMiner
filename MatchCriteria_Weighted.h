/* 
 * 文件: EdgeMatchCriteria_Weighted.h
 * 作者: D3M430
 *
 * 创建时间: 2017年1月20日 上午8:45
 */

#ifndef EDGEMATCHCRITERIA_WEIGHTED_H
#define EDGEMATCHCRITERIA_WEIGHTED_H

#include "MatchCriteria.h"
#include "Graph.h"
#include <vector>
#include <unordered_map>

/**
 * 定义了带权重图的匹配标准，用于判断搜索图 (G) 中的边是否满足查询图 (H) 中对应边的最小权重要求。
 */
class MatchCriteria_Weighted : public MatchCriteria
{
public:
    /**
     * 设置查询图中某条边的最小权重要求。
     * @param h_i 查询图中边的索引。
     * @param minWeight 该边的最小权重值。
     */
    void addMinWeight(int h_i, double minWeight);

    /**
     * 判断搜索图中的边是否与查询图中的边匹配。
     * 如果搜索图中的边权重大于等于查询图中对应边的最小权重要求（或没有设置权重要求），则返回 true。
     * @param g 搜索图（我们正在搜索的图）。
     * @param gEdgeIndex 搜索图中需要比较的边索引。
     * @param h 查询图（我们正在查找的目标图）。
     * @param hEdgeIndex 查询图中需要比较的边索引。
     * @return 如果搜索图中的边与查询图中的边匹配，则返回 true。
     */
    virtual bool isEdgeMatch(const Graph &g, int gEdgeIndex, const Graph &h, int hEdgeIndex) const override;
    
    /**     
     * 始终返回 true，因为此类型的匹配标准不涉及节点元数据。
     * @param g 搜索图（我们正在搜索的图）。
     * @param gNodeIndex 搜索图中需要比较的节点索引。
     * @param h 查询图（我们正在查找的目标图）。
     * @param hNodeIndex 查询图中需要比较的节点索引。
     * @return 始终返回 true。
     */
    virtual bool isNodeMatch(const Graph &g, int gNodeIndex, const Graph &h, int hNodeIndex) const override { return true; }

private:
    std::unordered_map<int, double> _minWeights; // 存储查询图中每条边的最小权重要求，键为边索引，值为最小权重。
};

#endif /* EDGEMATCHCRITERIA_WEIGHTED_H */