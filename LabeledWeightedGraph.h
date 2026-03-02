/* 
 * 文件: LabeledWeightedGraph.h
 * 作者: D3M430
 *
 * 创建时间: 2017年1月20日 上午8:30
 */

#ifndef LABELED_WEIGHTED_GRAPH_H
#define LABELED_WEIGHTED_GRAPH_H

#include "Graph.h"
#include "WeightRestriction.h"
#include <string>
#include <unordered_map>

/**
 * 带权重、有向、带标签的图。
 */
class LabeledWeightedGraph : public Graph
{
public:
    /**
     * 构造函数，支持设置时间窗口持续时间。
     * @param windowDuration 时间窗口持续时间，默认为 0。
     */
    LabeledWeightedGraph(int windowDuration = 0) : Graph(windowDuration) {}

    /**
     * 添加一个带标签的节点。如果未指定标签，则节点标签为空字符串。
     * @param v 节点索引。
     * @param label 节点的标签。
     */
    virtual void addLabeledNode(int v, std::string label);

    /**
     * 添加一条无权重的边，默认权重为 0.0。
     * @param u 边的起点节点索引。
     * @param v 边的终点节点索引。
     * @param dateTime 边的时间戳。
     */
    virtual void addEdge(int u, int v, time_t dateTime) override;

    /**
     * 添加一条带权重的边。
     * @param u 边的起点节点索引。
     * @param v 边的终点节点索引。
     * @param dateTime 边的时间戳。
     * @param weight 边的权重值。
     */
    virtual void addWeightedEdge(int u, int v, time_t dateTime, double weight);

    /**
     * 从另一张图中复制边，包括权重信息。
     * @param edgeIndex 要复制的边的索引。
     * @param g 包含目标边的图。
     */
    virtual void copyEdge(int edgeIndex, const Graph &g) override;

    /**
     * 获取指定边的权重。
     * @param edgeIndex 边的索引。
     * @return 边的权重值。
     */
    double getEdgeWeight(int edgeIndex) const;

    /**
     * 获取指定节点的标签。
     * @param nodeIndex 节点索引。
     * @return 节点的标签。
     */
    const std::string &getLabel(int nodeIndex) const;

    /**
     * 检查是否存在带有指定标签的节点。
     * @param label 节点的标签。
     * @return 如果存在则返回 true，否则返回 false。
     */
    bool hasLabeledNode(const std::string &label) const { return _nameMap.find(label) != _nameMap.end(); }

    /**
     * 根据节点标签获取节点索引。
     * @param nodeLabel 节点的标签。
     * @return 节点的索引。
     */
    int getIndex(const std::string &nodeLabel) const;

    /**
     * 显示图的基本信息，包括节点和边及其元数据。
     */
    void disp() const override;

    /**
     * 显示指定边的详细信息。
     * @param edgeIndex 边的索引。
     */
    void disp(int edgeIndex) const override;

    /**
     * 获取指定边的所有权重限制（用于搜索图）。
     * @param e 边的索引。
     * @return 权重限制列表（如果存在）。
     */
    const std::vector<WeightRestriction> &getWeightRestrictions(int e) const;

    /**
     * 检查指定边是否存在权重限制（用于搜索图）。
     * @param e 边的索引。
     * @return 如果存在权重限制则返回 true，否则返回 false。
     */
    bool hasWeightRestrictions(int e) const;

    /**
     * 为指定边添加权重限制。
     * @param e 边的索引。
     * @param restrict 权重限制对象。
     */
    void addWeightRestriction(int e, const WeightRestriction &restrict);

protected:
    /**
     * 更新按时间顺序排列的边列表。
     */
    virtual void updateOrderedEdges() const override;

private:
    std::vector<std::string> _nodeLabels; // 节点标签数组
    std::unordered_map<std::string, int> _nameMap; // 标签到节点索引的映射
    std::map<time_t, std::vector<double>> _timeEdgeWeightMap; // 按时间戳存储的边权重映射
    mutable std::vector<double> _edgeWeights; // 按时间顺序排列的边权重列表
    std::unordered_map<int, std::vector<WeightRestriction>> _weightRestricts; // 边的权重限制映射
};

#endif /* LABELED_WEIGHTED_GRAPH_H */