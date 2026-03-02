#ifndef MATCH_CRITERIA_DATA_GRAPH_H
#define MATCH_CRITERIA_DATA_GRAPH_H

#include "Attributes.h"
#include "DataGraph.h"
#include "Graph.h"
#include "MatchCriteria.h"

/**
 * 定义了 DataGraph 图的匹配标准，用于判断搜索图 (G) 中的边或节点是否与查询图 (H) 中的边或节点匹配。
 */
class MatchCriteria_DataGraph : public MatchCriteria
{
public:
    /**
     * 重载自 MatchCriteria 的方法，专门用于 DataGraphs。
     * 判断搜索图中的边是否与查询图中的边匹配。
     * @param g 搜索图（我们正在搜索的图）。
     * @param gEdgeIndex 搜索图中需要比较的边索引。
     * @param h 查询图（我们正在查找的目标图）。
     * @param hEdgeIndex 查询图中需要比较的边索引。
     * @return 如果搜索图中的边与查询图中的边匹配，则返回 true。
     */
    virtual bool isEdgeMatch(const Graph &g, int gEdgeIndex, const Graph &h, int hEdgeIndex) const override;
    
    /**
     * 重载自 MatchCriteria 的方法，专门用于 DataGraphs。
     * 判断搜索图中的节点是否与查询图中的节点匹配。
     * @param g 搜索图（我们正在搜索的图）。
     * @param gNodeIndex 搜索图中需要比较的节点索引。
     * @param h 查询图（我们正在查找的目标图）。
     * @param hNodeIndex 查询图中需要比较的节点索引。
     * @return 如果搜索图中的节点与查询图中的节点匹配，则返回 true。
     */
    virtual bool isNodeMatch(const Graph &g, int gNodeIndex, const Graph &h, int hNodeIndex) const override;
    
    /**
     * 判断两组属性是否匹配。
     * @param a1 第一组属性。
     * @param a2 第二组属性。
     * @return 如果两组属性匹配，则返回 true。
     */
    virtual bool doAttributesMatch(const Attributes &a1, const Attributes &a2) const;
};

#endif