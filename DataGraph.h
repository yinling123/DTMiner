#ifndef DATA_GRAPH__H
#define DATA_GRAPH__H

#include "Attributes.h"
#include "AttributesDef.h"
#include "Graph.h"
#include "GraphMatch.h"
#include <unordered_map>
#include <vector>

/**
 * DataGraph 类是标准有向时间图 (Graph) 的子类。
 * 它扩展了 Graph 类以支持节点和边的属性。
 */
class DataGraph : public Graph
{
public:
    /**
     * 添加一个节点，使用整数标识符。
     * @param v 节点的整数标识符。
     */
    virtual void addNode(int v) override;

    /**
     * 添加一个带有自定义名称和属性的节点。
     * @param name 节点的名称。
     * @param a 节点的属性。
     */
    virtual void addNode(const std::string &name, const Attributes &a);

    /**
     * 添加一条边，默认时间戳为当前边的数量。
     * @param u 源节点。
     * @param v 目标节点。
     */
    virtual void addEdge(int u, int v) override;

    /**
     * 添加一条带有指定时间戳的边。
     * @param u 源节点。
     * @param v 目标节点。
     * @param dateTime 边的时间戳。
     */
    virtual void addEdge(int u, int v, time_t dateTime) override;

    /**
     * 添加一条带有指定属性和时间戳的边。
     * @param source 源节点的名称。
     * @param dest 目标节点的名称。
     * @param dateTime 边的时间戳。
     * @param a 边的属性。
     */
    virtual void addEdge(const std::string &source, const std::string &dest, time_t dateTime, const Attributes &a);

    /**
     * 从另一个图中复制一条边。
     * @param edgeIndex 要复制的边的索引。
     * @param g 来源图。
     */
    virtual void copyEdge(int edgeIndex, const Graph &g) override;

    /**
     * 设置节点的属性定义。
     * @param def 节点的属性定义。
     */
    virtual void setNodeAttributesDef(const AttributesDef &def);

    /**
     * 设置边的属性定义。
     * @param def 边的属性定义。
     */
    virtual void setEdgeAttributesDef(const AttributesDef &def);

    /**
     * 根据一组图匹配结果生成子图。
     * @param matches 图匹配结果的向量。
     * @return 表示子图的新 DataGraph 对象。
     */
    virtual DataGraph createSubGraph(const std::vector<GraphMatch> &matches) const;

    /**
     * 根据节点标识符返回节点名称。
     * @param v 节点的标识符。
     * @return 节点的名称。
     */
    virtual const std::string &getName(int v) const;

    /**
     * 显示节点信息。
     * 包括节点的标识符、名称和属性。
     * @param u 节点的标识符。
     */
    virtual void dispNode(int u) const;

    /**
     * 显示边的信息。
     * 包括源节点、目标节点、时间戳和属性。
     * @param e 边的索引。
     */
    virtual void dispEdge(int e) const;

    /**
     * 显示图中所有边的信息。
     */
    virtual void disp() const;

    /**
     * 返回所有节点的属性。
     * @return 节点属性的向量。
     */
    const std::vector<Attributes> &nodeAttributes() const;

    /**
     * 返回所有边的属性。
     * @return 边属性的向量。
     */
    const std::vector<Attributes> &edgeAttributes() const;

    /**
     * 返回节点的属性定义。
     * @return 节点的属性定义。
     */
    const AttributesDef &nodeAttributesDef() const;

    /**
     * 返回边的属性定义。
     * @return 边的属性定义。
     */
    const AttributesDef &edgeAttributesDef() const;

public:
    /**
     * 根据时间戳更新边的顺序。
     * 清除旧的边属性，并按时间顺序重建。
     */
    virtual void updateOrderedEdges() const override;

private:
    /**
     * 存储节点名称的向量。
     * 索引对应节点的整数标识符。
     */
    std::vector<std::string> _nodeNames;

    /**
     * 节点名称到整数标识符的映射。
     */
    std::unordered_map<std::string, int> _nodeNameMap;

    /**
     * 节点和边的属性定义。
     */
    AttributesDef _nodeAttributesDef, _edgeAttributesDef;

    /**
     * 存储每个节点的属性。
     */
    std::vector<Attributes> _nodeAttributes;

    /**
     * 按时间戳存储边的属性。
     * 键为时间戳，值为该时间戳下的边属性列表。
     */
    std::map<time_t, std::vector<Attributes>> _timeEdgeAttributesMap;

    /**
     * 按顺序存储所有边的属性。
     */
    mutable std::vector<Attributes> _edgeAttributes;
};

#endif