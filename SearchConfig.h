#ifndef SEARCH_CONFIG__H
#define SEARCH_CONFIG__H

#include <time.h>
#include <vector>
#include <string>

/**
 * 表示与实时流数据相关的变量。
 */
struct SearchConfig
{
public:
    /**
     * 默认构造函数，初始化结构体成员变量。
     * - useStreaming: 是否启用流式处理，默认为 false。
     * - duration: 从 MongoDB 数据库拉取下一块数据的时间长度（秒），默认为 0。
     * - delay: 拉取下一块数据前的等待时间间隔（秒），默认为 0。
     * - window: 图中保留的最大时间范围（秒），用于分析，不要与 delta 混淆，默认为 0。
     * - startDate, endDate: 固定的开始时间和结束时间，默认为 0。
     */
    SearchConfig() : useStreaming(false), duration(0), delay(0), window(0), startDate(0), endDate(0) {} 

    /** 
     * 是否实际使用流式处理。如果设置为 true，则启用流式处理。 
     */
    bool useStreaming;

    /** 
     * 从 MongoDB 数据库拉取下一块数据的时间长度（秒）。 
     */
    time_t duration;

    /** 
     * 拉取下一块数据前的等待时间间隔（秒）。 
     */
    time_t delay;

    /** 
     * 图中保留的最大时间范围（秒），用于分析。注意：这与 delta 值不同。 
     */
    time_t window;

    /** 
     * Delta 是匹配子图中边之间允许的最大时间差（秒）。 
     */
    time_t delta;

    /** 
     * 要查找子图的唯一节点 ID 列表。 
     */
    std::vector<std::string> find_subgraphs_nodes;

    /** 
     * 查询名称，用于为给定节点查找链接。 
     */
    std::string find_subgraphs_query;

    /** 
     * 固定的开始时间和结束时间。 
     */
    time_t startDate, endDate;
};

#endif