/* 
 * File:   GraphFilter.h
 * Author: D3M430
 *
 * Created on January 20, 2017, 8:18 AM
 */

#ifndef GRAPHFILTER_H
#define	GRAPHFILTER_H

#include "Graph.h"
#include "CertGraph.h"
#include "MatchCriteria.h"

/**
 * @brief 用于根据搜索条件过滤或复制图的类。
 *        在某些情况下可以显著提高性能。
 */
class GraphFilter
{
public:
    /**
     * @brief 根据查询图及其匹配条件，过滤掉不匹配的边，并将结果复制到目标图。
     * @param g 要过滤/复制的源图。
     * @param h 查询图，包含匹配条件的参考图。
     * @param criteria 查询图的匹配条件。
     * @param g2 目标图，基于匹配条件添加节点和边。
     */
    static void filter(const Graph &g, const Graph &h, const MatchCriteria &criteria, Graph &g2);

    /**
     * @brief 根据查询图及其匹配条件，过滤掉不匹配的边，并将结果复制到目标 CERT 图。
     * @param g 要过滤/复制的源 CERT 图。
     * @param h 查询图，包含匹配条件的参考图。
     * @param criteria 查询图的匹配条件。
     * @param g2 目标 CERT 图，基于匹配条件添加节点和边。
     */
    static void filter(const CertGraph &g, const CertGraph &h, const MatchCriteria &criteria, CertGraph &g2);

    /**
     * @brief 根据指定的时间范围，过滤掉不在该范围内的边，并将结果复制到目标图。
     * @param g 要过滤/复制的源 CERT 图。
     * @param start 感兴趣时间范围的起始时间。
     * @param end 感兴趣时间范围的结束时间。
     * @param g2 目标 CERT 图，基于时间范围添加节点和边。
     */
    static void filter(const CertGraph &g, time_t start, time_t end, CertGraph &g2);
};

#endif	/* GRAPHFILTER_H */