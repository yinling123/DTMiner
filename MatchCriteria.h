/* 
 * 文件: EdgeMatchCriteria.h
 * 作者: D3M430
 *
 * 创建时间: 2017年1月16日 上午10:57
 */

#ifndef EDGEMATCHCRITERIA_H
#define EDGEMATCHCRITERIA_H

#include "Graph.h"

/**
 * 基类，允许用户定义搜索图 (G) 中的边或节点是否与查询图 (H) 中的边匹配的标准。
 * 默认情况下，仅确保边的时间戳在当前时间窗口范围内。
 */
class MatchCriteria
{
public:
    /**
     * 纯虚函数，用于判断搜索图中的边是否与查询图中的边匹配。
     * @param g 搜索图（我们正在搜索的图）。
     * @param gEdgeIndex 搜索图中需要比较的边索引。
     * @param h 查询图（我们正在查找的目标图）。
     * @param hEdgeIndex 查询图中需要比较的边索引。
     * @return 如果搜索图中的边与查询图中的边匹配，则返回 true；否则返回 false。
     */
    virtual bool isEdgeMatch(const Graph &g, int gEdgeIndex, const Graph &h, int hEdgeIndex) const;
    
    /**
     * 纯虚函数，用于判断搜索图中的节点是否与查询图中的节点匹配。
     * @param g 搜索图（我们正在搜索的图）。
     * @param gNodeIndex 搜索图中需要比较的节点索引。
     * @param h 查询图（我们正在查找的目标图）。
     * @param hNodeIndex 查询图中需要比较的节点索引。
     * @return 如果搜索图中的节点与查询图中的节点匹配，则返回 true；否则返回 false。
     */
    virtual bool isNodeMatch(const Graph &g, int gNodeIndex, const Graph &h, int hNodeIndex) const;
};

#endif /* EDGEMATCHCRITERIA_H */