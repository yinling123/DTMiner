/* 
 * 文件: EdgeMatchCriteria_CERT.h
 * 作者: D3M430
 *
 * 创建时间: 2017年2月13日 下午2:08
 */

#ifndef EDGEMATCHCRITERIA_CERT_H
#define EDGEMATCHCRITERIA_CERT_H

#include "Graph.h"
#include "MatchCriteria.h"

/**
 * 定义了 CERT 图的匹配标准，用于判断搜索图 (G) 中的边或节点是否与查询图 (H) 中的边或节点匹配。
 */
class MatchCriteria_CERT : public MatchCriteria
{
public:
    /**
     * 重载自 MatchCriteria 的方法，专门用于 CertGraphs。
     * 判断搜索图中的边是否与查询图中的边匹配。
     * @param g 搜索图（我们正在搜索的图）。
     * @param gEdgeIndex 搜索图中需要比较的边索引。
     * @param h 查询图（我们正在查找的目标图）。
     * @param hEdgeIndex 查询图中需要比较的边索引。
     * @return 如果搜索图中的边与查询图中的边匹配，则返回 true。
     */
    virtual bool isEdgeMatch(const Graph &g, int gEdgeIndex, const Graph &h, int hEdgeIndex) const override;
    
    /**
     * 重载自 MatchCriteria 的方法，专门用于 CertGraphs。
     * 判断搜索图中的节点是否与查询图中的节点匹配。
     * @param g 搜索图（我们正在搜索的图）。
     * @param gNodeIndex 搜索图中需要比较的节点索引。
     * @param h 查询图（我们正在查找的目标图）。
     * @param hNodeIndex 查询图中需要比较的节点索引。
     * @return 如果搜索图中的节点与查询图中的节点匹配，则返回 true。
     */
    virtual bool isNodeMatch(const Graph &g, int gNodeIndex, const Graph &h, int hNodeIndex) const override;
};

#endif	/* EDGEMATCHCRITERIA_CERT_H */