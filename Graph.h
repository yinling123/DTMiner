/* 
 * File:   Graph.h
 * Author: D3M430
 *
 * Created on January 13, 2017, 11:32 AM
 */

#ifndef GRAPH_H
#define	GRAPH_H

#include <vector>
#include <map>
#include <unordered_map>
#include <time.h>
#include "Node.h"
#include "Edge.h"

/**
 * @brief 定义了一个标准的有向图类，支持按时间顺序存储边。
 */
class Graph
{
public:
    /**
     * @brief 构造函数，初始化图对象。
     * @param windowDuration 时间窗口的持续时间（秒）。如果为 0，则不使用时间窗口。
     */
    Graph(int windowDuration = 0);

    /**
     * @brief 确保图中至少包含 v+1 个节点。
     * @param v 节点索引。
     */
    virtual void addNode(int v);

    /**
     * @brief 添加一条边，并根据需要调整节点数量。
     * @param u 源节点索引。
     * @param v 目标节点索引。
     */
    virtual void addEdge(int u, int v);

    /**
     * @brief 添加一条带时间戳的边，并根据需要调整节点数量。
     * @param u 源节点索引。
     * @param v 目标节点索引。
     * @param dateTime 边的时间戳。
     */
    virtual void addEdge(int u, int v, time_t dateTime);

    /**
     * @brief 从另一个图中复制指定索引的边（用于保持元数据一致性）。
     * @param edgeIndex 要复制的边索引。
     * @param g 包含边的源图。
     */
    virtual void copyEdge(int edgeIndex, const Graph &g);

    /**
     * @brief 检查图中是否存在从 u 到 v 的边。
     * @param u 源节点索引。
     * @param v 目标节点索引。
     * @return 如果存在边则返回 true，否则返回 false。
     */
    virtual bool hasEdge(int u, int v) const;

    /**
     * @brief 获取从 u 到 v 的所有边索引。
     * @param u 源节点索引。
     * @param v 目标节点索引。
     * @return 包含边索引的向量。
     */
    virtual const std::vector<int> &getEdgeIndexes(int u, int v) const;

    /**
     * @brief 返回图中所有节点的列表。
     * @return 包含所有节点的向量。
     */
    virtual const std::vector<Node> &nodes() const { return _nodes; }

    /**
     * @brief 返回图中所有边的有序列表（按时间顺序排列）。
     * @return 包含所有边的向量。
     */
    virtual const std::vector<Edge> &edges() const;

    /**
     * @brief 获取图中节点的数量（无需排序）。
     * @return 节点数量。
     */
    virtual int numNodes() const { return _nodes.size(); }

    /**
     * @brief 获取图中边的数量（无需排序）。
     * @return 边数量。
     */
    virtual int numEdges() const { return _numEdges; }

    /**
     * @brief 显示图的所有内容。
     */
    virtual void disp() const;

    /**
     * @brief 显示指定索引的边的内容。
     * @param edgeIndex 边的索引。
     */
    virtual void disp(int edgeIndex) const;

    /**
     * @brief 显示图中边的时间范围。
     */
    virtual void dispDateTimeRange() const;

    /**
     * @brief 获取当前时间窗口的持续时间（秒）。
     * @return 时间窗口的持续时间。
     */
    int windowDuration() const { return _windowDuration; }

    /**
     * @brief 设置时间窗口的持续时间。
     * @param duration 新的时间窗口持续时间。
     */
    void setWindowDuration(int duration);

    /**
     * @brief 获取当前时间窗口的起始时间。
     * @return 当前窗口的起始时间。
     */
    time_t windowStart() const; // { return _windowStart; }

    /**
     * @brief 获取当前时间窗口的结束时间。
     * @return 当前窗口的结束时间。
     */
    time_t windowEnd() const; // { return _windowEnd; }
    
public:
    /**
     * @brief 按时间顺序更新边的排序。
     */
    virtual void updateOrderedEdges() const;

    // 标志位，用于判断是否已完成边的完整排序
    mutable bool _edgesReady;

public:
    int _numEdges = 0; // 边的数量计数器
    mutable std::vector<Node> _nodes; // 节点列表
    int _windowDuration; // 时间窗口的持续时间
    time_t _windowStart, _windowEnd; // 当前时间窗口的起始和结束时间
    // 按时间排序的边映射
    mutable std::map<time_t, std::vector<Edge>> _timeEdgeMap;
    mutable std::vector<time_t> _edgeTimes; // 边的时间戳列表
    mutable std::vector<Edge> _edges; // 边的有序列表
    mutable std::unordered_map<int, std::unordered_map<int, std::vector<int>>> _nodeEdges; // 节点之间的边映射
};

#endif	/* GRAPH_H */