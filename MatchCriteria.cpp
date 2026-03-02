#include "MatchCriteria.h"

/**
 * 判断两条边是否匹配。
 * 匹配条件是：搜索图中的边的时间戳必须在指定的时间窗口范围内。
 * @param g 搜索图（我们正在搜索的图）。
 * @param gEdgeIndex 搜索图中需要比较的边索引。
 * @param h 查询图（我们正在查找的目标图）。
 * @param hEdgeIndex 查询图中需要比较的边索引。
 * @return 如果搜索图中的边时间戳在时间窗口范围内，则返回 true；否则返回 false。
 */
bool MatchCriteria::isEdgeMatch(const Graph &g, int gEdgeIndex, const Graph &h, int hEdgeIndex) const
{
    const Edge &e = g.edges()[gEdgeIndex]; // 获取搜索图中的边
    time_t t = e.time(); // 获取该边的时间戳
    return t >= g.windowStart() && t <= g.windowEnd(); // 判断时间戳是否在时间窗口范围内
}

/**
 * 判断两个节点是否匹配。
 * 该方法始终返回 true，因为基类的匹配标准不涉及节点的具体属性。
 * @param g 搜索图（我们正在搜索的图）。
 * @param gNodeIndex 搜索图中需要比较的节点索引。
 * @param h 查询图（我们正在查找的目标图）。
 * @param hNodeIndex 查询图中需要比较的节点索引。
 * @return 始终返回 true。
 */
bool MatchCriteria::isNodeMatch(const Graph &g, int gNodeIndex, const Graph &h, int hNodeIndex) const
{
    return true; // 基类中节点匹配始终返回 true
}