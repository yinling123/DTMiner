/* 
 * 文件: CertGraph.h
 * 作者: D3M430
 *
 * 创建时间: 2017年2月6日，上午11:36
 */

#ifndef CERTGRAPH_H
#define CERTGRAPH_H

#include "LabeledWeightedGraph.h"
#include "Graph.h"
#include "GraphMatch.h"
#include "DegRestriction.h" // 用于搜索图的度限制条件
#include "Roles.h"
#include <string>
#include <time.h>
#include <map> // 用于按日期/时间排序的红黑树
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * 继承自带标签、加权、有向时间图设计的子类，
 * 专门用于处理 CERT 合成网络数据集。
 * 数据集链接：https://resources.sei.cmu.edu/library/asset-view.cfm?assetid=508099
 */
class CertGraph : public LabeledWeightedGraph {
public:
    // 节点类型常量
    static constexpr const char* USER_NODE = "user"; // 用户节点
    static constexpr const char* PC_NODE = "pc"; // 计算机节点
    static constexpr const char* FILENAME_NODE = "filename"; // 文件名节点
    static constexpr const char* CONTENT_NODE = "content"; // 内容节点
    static constexpr const char* FILETREE_NODE = "filetree"; // 文件树节点
    static constexpr const char* ADDRESS_NODE = "address"; // 地址节点
    static constexpr const char* DOMAIN_NODE = "domain"; // 域节点

    /** 返回拼接的字符串，包含用户ID和角色（如果未知则返回 UNKNOWN） */
    static std::string getUserRole(const std::string &user, const Roles &roles);

    CertGraph(int windowDuration) : LabeledWeightedGraph(windowDuration) { }
    /** 重写基类方法。添加带有适当标签的节点（否则节点标签为空字符串）。 */
    void addLabeledNode(int v, std::string label) override;
    /** 添加一个带有所有 CERT 元数据的节点 */
    void addTypedNode(const std::string &name, const std::string &type); // , double weight);
    /** 重写基类方法。添加权重为 0.0 的边 */
    void addEdge(int u, int v, time_t dateTime) override;
    /** 重写基类方法。添加带有指定权重的边 */
    void addWeightedEdge(int u, int v, time_t dateTime, double weight) override;
    /** 添加一条带有所有 CERT 元数据的边 */
    void addEdge(const std::string &source, const std::string &dest, time_t dateTime, const std::string &type);
    /** 添加一条带有所有 CERT 元数据的边 */
    void addEdge(int u, int v, time_t dateTime, const std::string &type);
    /** 重写基类方法。从另一个图中复制指定的边，包括权重信息 */
    void copyEdge(int edgeIndex, const Graph &g) override;
    /** 创建一个新的子图，仅包含给定的边及其相邻节点 */
    CertGraph createSubGraph(const std::vector<int> &edges);
    /** 基于所有给定的子图创建一个新的子图。
     * @param subGraphs  (输入) 要合并的匹配子图列表。
     * @param edgeCounts  (输出) 每条边在子图中出现的次数。
     * @param nodeCounts  (输出) 每个节点在子图中出现的次数。
     */
    CertGraph createSubGraph(const std::vector<GraphMatch> &subGraphs,
                             std::vector<int> &edgeCounts, std::vector<int> &nodeCounts);
    /** 基于所有给定的子图创建一个新的聚合子图，
     * 并忽略边类型。
     * @param ignoreDir  (输入) 如果为 true，则忽略边的方向。
     * @param subGraphs  (输入) 要合并的匹配子图列表。
     * @param edgeCounts  (输出) 原始图中该边对应的唯一边数量。
     */
    CertGraph createAggregateSubGraph(bool ignoreDir, const std::vector<GraphMatch> &subGraphs, std::vector<int> &edgeCounts);

    /** 获取节点的类型 */
    const std::string &getNodeType(int v) const { return _nodeTypes[v]; }
    /** 获取与节点关联的边类型 */
    const std::string &getEdgeType(int edgeIndex) const;
    /** 返回节点的出度（针对特定边类型）。
     * @param v  节点索引。
     * @param edgeType  边类型。 */
    int getOutDeg(int v, const std::string &edgeType) const;
    /** 返回节点的入度（针对特定边类型）。
     * @param v  节点索引。
     * @param edgeType  边类型。 */
    int getInDeg(int v, const std::string &edgeType) const;

    /** 获取给定节点的出边类型集合。
     * @param v  节点索引。 */
    const std::unordered_set<std::string> &getOutEdgeTypes(int v) const;
    /** 获取给定节点的入边类型集合。
     * @param v  节点索引。 */
    const std::unordered_set<std::string> &getInEdgeTypes(int v) const;

    /** 显示图的所有内容 */
    void disp() const override;
    /** 显示指定边的内容 */
    void disp(int edgeIndex) const override;

    /** 用于搜索图。期望某些节点的度数在特定范围内（针对特定边类型）。
     * @param v  节点索引。
     * @return  如果存在，返回该节点的所有度限制条件列表。 */
    const std::vector<DegRestriction> &getDegRestrictions(int v) const;
    /** 用于搜索图。返回该节点是否有任何度限制条件。 */
    bool hasDegRestrictions(int v) const;
    /** 为指定节点添加给定的限制条件。 */
    void addDegRestriction(int v, const DegRestriction &restrict);

    /** 返回给定节点是否需要在匹配子图中具有精确名称匹配。
     * @param v  节点索引。
     * @return  如果节点需要在子图中具有匹配名称，则返回 true。 */
    bool needsNameMatch(int v) const { return _needsNameMatch.find(v) != _needsNameMatch.end(); }
    /** 要求该节点在匹配子图中具有名称匹配。
     * @param v  节点索引。 */
    void setNeedsNameMatch(int v) { _needsNameMatch.insert(v); }

    /** 返回给定节点是否需要在匹配子图中具有正则表达式匹配。
     * @param v  节点索引。
     * @return  如果节点需要在子图中具有正则表达式匹配，则返回 true。 */
    bool needsRegexMatch(int v) const { return _regexMatch.find(v) != _regexMatch.end(); }
    /** 添加给定的正则表达式作为匹配子图中该节点的要求。
     * @param v  节点索引。
     * @param regex  需要在子图中匹配的正则表达式。 */
    void addRegex(int v, const std::regex &regex);
    /** 返回给定节点所需的正则表达式（如果存在），否则抛出异常。
     * @param v  节点索引。
     * @return  编译后的正则表达式。 */
    const std::regex &getRegex(int v) const { return _regexMatch.find(v)->second; }

protected:
    void updateOrderedEdges() const override;

private:
    std::vector<std::string> _nodeTypes; // 节点类型列表
    std::unordered_map<std::string, std::vector<int>> _typeOutDegs, _typeInDegs; // 边类型的出度和入度映射
    std::unordered_map<int, std::vector<DegRestriction>> _degRestricts; // 节点的度限制条件
    std::unordered_set<int> _needsNameMatch; // 需要名称匹配的节点集合
    std::unordered_map<int, std::regex> _regexMatch; // 节点的正则表达式匹配
    // 按日期/时间排序的有序映射
    std::map<time_t, std::vector<std::string>> _timeEdgeTypeMap;
    // 图的有序边列表
    mutable std::vector<std::string> _edgeTypes;
    std::vector<std::unordered_set<std::string>> _nodeOutEdgeTypes, _nodeInEdgeTypes; // 节点的出边和入边类型集合
};

#endif /* CERTGRAPH_H */