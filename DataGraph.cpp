#include "DataGraph.h"
#include <iostream>

using namespace std;

/**
 * 添加一个节点，使用整数标识符。
 * 如果需要，调整内部数据结构大小以容纳新节点。
 * @param v 节点的整数标识符。
 */
void DataGraph::addNode(int v)
{
    string name = to_string(v);
    if (_nodeNames.size() <= v) {
        _nodeNames.resize(v + 1);
        _nodeAttributes.resize(v + 1);
    }
    _nodeNames[v] = to_string(v);
    _nodeNameMap[name] = v;
    Graph::addNode(v); // 调用基类方法添加节点。
}

/**
 * 添加一个带有自定义名称和属性的节点。
 * 验证属性是否与预定义的属性定义匹配。
 * 如果不匹配，则抛出异常。
 * @param name 节点的名称。
 * @param a 节点的属性。
 */
void DataGraph::addNode(const std::string &name, const Attributes &a)
{
    if (!_nodeAttributesDef.isSizeMatch(a)) {
        cerr << "添加的节点具有以下属性：" << endl;
        cerr << a.floatValues().size() << " 个浮点值" << endl;
        cerr << a.intValues().size() << " 个整数值" << endl;
        cerr << a.stringValues().size() << " 个字符串值" << endl;
        cerr << "属性定义要求：" << endl;
        cerr << _nodeAttributesDef.numFloatValues() << " 个浮点值" << endl;
        cerr << _nodeAttributesDef.numIntValues() << " 个整数值" << endl;
        cerr << _nodeAttributesDef.numStringValues() << " 个字符串值" << endl;
        throw "节点属性与定义中的属性不匹配。";
    }

    int u = _nodeNameMap.size();
    // 存储每个节点的名称
    _nodeNames.push_back(name);
    // 按照次序进行编号存储
    _nodeNameMap[name] = u;
    Graph::addNode(u); // 调用基类方法添加节点。
    _nodeAttributes.push_back(a);
}

/**
 * 添加一条边，默认时间戳为当前边的数量。
 * @param u 源节点。
 * @param v 目标节点。
 */
void DataGraph::addEdge(int u, int v)
{
    time_t dateTime = numEdges();
    this->addEdge(u, v, dateTime);
}

/**
 * 添加一条带有指定时间戳的边。
 * 初始化边的空属性。
 * @param u 源节点。
 * @param v 目标节点。
 * @param dateTime 边的时间戳。
 */
void DataGraph::addEdge(int u, int v, time_t dateTime)
{
    Graph::addEdge(u, v, dateTime); // 调用基类方法添加边。
    Attributes a;
    _edgeAttributes.push_back(a);
    _timeEdgeAttributesMap[dateTime].push_back(a);
}

/**
 * 添加一条带有指定属性和时间戳的边。
 * 验证属性是否与预定义的属性定义匹配。
 * 如果不匹配，则抛出异常。
 * @param source 源节点的名称。
 * @param dest 目标节点的名称。
 * @param dateTime 边的时间戳。
 * @param a 边的属性。
 */
void DataGraph::addEdge(const std::string &source, const std::string &dest, time_t dateTime, const Attributes &a)
{
    if (!_edgeAttributesDef.isSizeMatch(a)) {
        cerr << "添加的边具有以下属性：" << endl;
        cerr << a.floatValues().size() << " 个浮点值" << endl;
        cerr << a.intValues().size() << " 个整数值" << endl;
        cerr << a.stringValues().size() << " 个字符串值" << endl;
        cerr << "属性定义要求：" << endl;
        cerr << _edgeAttributesDef.numFloatValues() << " 个浮点值" << endl;
        cerr << _edgeAttributesDef.numIntValues() << " 个整数值" << endl;
        cerr << _edgeAttributesDef.numStringValues() << " 个字符串值" << endl;
        throw "边属性与定义中的属性不匹配。";
    }

    // 获取点索引，从0开始索引
    int u = _nodeNameMap[source];
    int v = _nodeNameMap[dest];
    // 添加边信息
    Graph::addEdge(u, v, dateTime); // 调用基类方法添加边。
    _edgeAttributes.push_back(a);
    _timeEdgeAttributesMap[dateTime].push_back(a);
}

/**
 * 从另一个图中复制一条边。
 * 确保所有节点和属性都被正确复制。
 * @param edgeIndex 要复制的边的索引。
 * @param g 来源图。
 */
void DataGraph::copyEdge(int edgeIndex, const Graph &g)
{
    const DataGraph &dg = (const DataGraph &)g;
    int n = g.nodes().size();
    if (n > _nodeNames.size()) {
        _nodeNames = dg._nodeNames;
        _nodeNameMap = dg._nodeNameMap;
        _nodeAttributes = dg._nodeAttributes;
    }

    const Edge &edge = dg.edges()[edgeIndex];
    const Attributes &a = dg.edgeAttributes()[edgeIndex];

    const string &node1 = _nodeNames[edge.source()];
    const string &node2 = _nodeNames[edge.dest()];
    this->addEdge(node1, node2, edge.time(), a);
}

/**
 * 设置节点的属性定义。
 * @param def 节点的属性定义。
 */
void DataGraph::setNodeAttributesDef(const AttributesDef &def)
{
    _nodeAttributesDef = def;
}

/**
 * 设置边的属性定义。
 * @param def 边的属性定义。
 */
void DataGraph::setEdgeAttributesDef(const AttributesDef &def)
{
    _edgeAttributesDef = def;
}

/**
 * 返回节点的属性定义。
 * @return 节点的属性定义。
 */
const AttributesDef &DataGraph::nodeAttributesDef() const { return _nodeAttributesDef; }

/**
 * 返回边的属性定义。
 * @return 边的属性定义。
 */
const AttributesDef &DataGraph::edgeAttributesDef() const { return _edgeAttributesDef; }

/**
 * 根据一组图匹配结果生成子图。
 * @param matches 图匹配结果的向量。
 * @return 表示子图的新 DataGraph 对象。
 * 将匹配子图格式转化为数据图格式
 */
DataGraph DataGraph::createSubGraph(const std::vector<GraphMatch> &matches) const
{
    DataGraph g;
    g._nodeAttributesDef = this->_nodeAttributesDef;
    g._edgeAttributesDef = this->_edgeAttributesDef;

    unordered_map<int, int> edgeMap;
    unordered_map<int, int> nodeMap;

    for (const GraphMatch &gm : matches) {
        const vector<int> &edges = gm.edges();
        for (int e : edges) {
            const Edge &edge = this->edges()[e];
            int u = edge.source();
            int v = edge.dest();

            if (edgeMap.find(e) == edgeMap.end()) {
                int e2 = edgeMap.size();
                edgeMap[e] = e2;

                if (nodeMap.find(u) == nodeMap.end()) {
                    int u2 = nodeMap.size();
                    nodeMap[u] = u2;
                    g.addNode(this->getName(u), this->nodeAttributes()[u]);
                }
                if (nodeMap.find(v) == nodeMap.end()) {
                    int v2 = nodeMap.size();
                    nodeMap[v] = v2;
                    g.addNode(this->getName(v), this->nodeAttributes()[v]);
                }

                const string &name1 = g.getName(nodeMap[u]);
                const string &name2 = g.getName(nodeMap[v]);
                g.addEdge(name1, name2, edge.time(), this->edgeAttributes()[e]);
            }
        }
    }

    return g;
}

/**
 * 根据节点标识符返回节点名称。
 * @param v 节点的标识符。
 * @return 节点的名称。
 */
const string &DataGraph::getName(int v) const
{
    return _nodeNames[v];
}

/**
 * 返回所有节点的属性。
 * @return 节点属性的向量。
 */
const std::vector<Attributes> &DataGraph::nodeAttributes() const
{
    return _nodeAttributes;
}

/**
 * 返回所有边的属性。
 * @return 边属性的向量。
 */
const std::vector<Attributes> &DataGraph::edgeAttributes() const
{
    return _edgeAttributes;
}

/**
 * 显示节点信息。
 * 包括节点的标识符、名称和属性。
 * @param u 节点的标识符。
 */
void DataGraph::dispNode(int u) const
{
    cout << u << "(" << _nodeNames[u];
    const auto &values = _nodeAttributes[u].stringValues();
    for (const string &val : values)
        cout << "," << val;
    for (int val : _nodeAttributes[u].intValues())
        cout << "," << val;
    for (double val : _nodeAttributes[u].floatValues())
        cout << "," << val;
    cout << ")" << flush;
}

/**
 * 显示边的信息。
 * 包括源节点、目标节点、时间戳和属性。
 * @param e 边的索引。
 */
void DataGraph::dispEdge(int e) const
{
    const Edge &edge = this->edges()[e];
    dispNode(edge.source());
    cout << " -> ";
    dispNode(edge.dest());
    cout << " " << edge.time();
    const auto &values = _edgeAttributes[e].stringValues();
    for (const string &val : values)
        cout << "," << val;
    for (int val : _edgeAttributes[e].intValues())
        cout << "," << val;
    for (double val : _edgeAttributes[e].floatValues())
        cout << "," << val;
    cout << flush;
}

/**
 * 显示图中所有边的信息。
 */
void DataGraph::disp() const
{
    int m = this->numEdges();
    for (int e = 0; e < m; e++) {
        dispEdge(e);
        cout << endl;
    }
    if (m == 0)
        cout << "[无边]" << endl;
}

/**
 * 根据时间戳更新边的顺序。
 * 清除旧的边属性，并按时间顺序重建。
 */
void DataGraph::updateOrderedEdges() const
{
    cout << "正在更新边的顺序。" << endl;

    Graph::updateOrderedEdges(); // 确保基类先更新。

    _edgeAttributes.clear();

    for (auto &pair : _timeEdgeAttributesMap) {
        time_t dateTime = pair.first;
        const vector<Attributes> &attributes = _timeEdgeAttributesMap.find(dateTime)->second;
        for (const Attributes &a : attributes) {
            _edgeAttributes.push_back(a);
        }
    }
}