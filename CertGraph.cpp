#include "CertGraph.h"
#include "GraphMatch.h"
#include <iostream>
#include <unordered_set>

using namespace std;

// 静态方法：获取用户的角色信息
string CertGraph::getUserRole(const string &user, const Roles &roles) {
    string userRole = user;
    userRole.push_back('_'); // 添加下划线分隔符
    userRole.append(roles.getRole(user)); // 拼接角色信息
    return userRole; // 返回完整的用户角色字符串
}

// 添加一个带标签的节点
void CertGraph::addLabeledNode(int v, std::string label) {
    // 调用基类方法添加节点
    LabeledWeightedGraph::addLabeledNode(v, label);
    // 扩展其他变量
    int n = this->nodes().size();
    if (n > this->_nodeTypes.size()) {        
        _nodeTypes.resize(n); // 调整节点类型数组大小
        _nodeOutEdgeTypes.resize(n); // 调整出边类型数组大小
        _nodeInEdgeTypes.resize(n); // 调整入边类型数组大小
    }
}

// 添加一条边（无权重）
void CertGraph::addEdge(int u, int v, time_t dateTime) {
    // 调用基类方法添加边
    LabeledWeightedGraph::addEdge(u, v, dateTime);    
    this->_timeEdgeTypeMap[dateTime].push_back(""); // 默认边类型为空
}

// 添加一条带权重的边
void CertGraph::addWeightedEdge(int u, int v, time_t dateTime, double weight) {
    // 调用基类方法添加带权重的边
    LabeledWeightedGraph::addWeightedEdge(u, v, dateTime, weight);
    this->_timeEdgeTypeMap[dateTime].push_back(""); // 默认边类型为空
}

// 复制一条边及其相关信息
void CertGraph::copyEdge(int edgeIndex, const Graph &g) {
    CertGraph &cg = (CertGraph&)g; // 将传入图转换为CertGraph类型
    const Edge &edge = g.edges()[edgeIndex]; // 获取边信息
    int u = edge.source(), v = edge.dest(); // 获取源节点和目标节点
    // 如果当前图中节点不足，则复制所有节点
    if (this->nodes().size() < g.nodes().size()) {
        int n = g.nodes().size();
        for (int i = this->nodes().size(); i < n; i++) {
            this->addTypedNode(cg.getLabel(i), cg.getNodeType(i)); // 添加带类型的节点
        }
    }
    this->addEdge(u, v, edge.time(), cg.getEdgeType(edgeIndex)); // 添加边
}

// 根据指定的边列表创建子图
CertGraph CertGraph::createSubGraph(const vector<int> &edges) {    
    CertGraph g(this->windowDuration()); // 创建一个新的CertGraph对象
    
    // 确定使用的节点，并在它们之间创建边
    unordered_map<int, int> nodeMap; // 映射原始节点到新节点
    for (int e : edges) {
        const Edge &edge = this->edges()[e]; // 获取边信息
        int u = edge.source(), v = edge.dest(); // 获取源节点和目标节点
        // 如果源节点尚未映射，则添加新节点
        if (nodeMap.find(u) == nodeMap.end()) {
            int u2 = nodeMap.size();
            nodeMap[u] = u2;            
            g.addTypedNode(this->getLabel(u), this->getNodeType(u));
        }
        // 如果目标节点尚未映射，则添加新节点
        if (nodeMap.find(v) == nodeMap.end()) {
            int v2 = nodeMap.size();
            nodeMap[v] = v2;
            g.addTypedNode(this->getLabel(v), this->getNodeType(v));
        }
        // 添加边到子图
        g.addEdge(nodeMap[u], nodeMap[v], edge.time(), this->getEdgeType(e));
    }
    return g; // 返回创建的子图
}

// 根据多个子图匹配结果创建子图，并统计边和节点的数量
CertGraph CertGraph::createSubGraph(const vector<GraphMatch> &subGraphs,
    vector<int> &edgeCounts, vector<int> &nodeCounts) {
    edgeCounts.clear(); // 清空边计数
    nodeCounts.clear(); // 清空节点计数    

    // 用于跟踪节点对之间的唯一边
    unordered_map<int, unordered_map<int, unordered_set<int>>> uniqueEdges;
    
    CertGraph g(this->windowDuration()); // 创建一个新的CertGraph对象
    
    // 确定使用的节点，并在它们之间创建边
    unordered_map<int, int> edgeMap; // 边索引映射
    unordered_map<int, int> nodeMap; // 节点索引映射
    for (const GraphMatch &gm : subGraphs) {
        const vector<int> &edges = gm.edges(); // 获取匹配的边列表
        for (int e : edges) {
            const Edge &edge = this->edges()[e]; // 获取边信息
            int u = edge.source(), v = edge.dest(); // 获取源节点和目标节点
            // 如果边尚未使用，则添加新边
            if (edgeMap.find(e) == edgeMap.end()) {
                edgeMap[e] = edgeCounts.size();
                edgeCounts.push_back(0); // 初始化边计数
                // 如果源节点尚未映射，则添加新节点
                if (nodeMap.find(u) == nodeMap.end()) {
                    int u2 = nodeMap.size();
                    nodeMap[u] = u2;            
                    g.addTypedNode(this->getLabel(u), this->getNodeType(u));
                    nodeCounts.push_back(0); // 初始化节点计数
                }
                // 如果目标节点尚未映射，则添加新节点
                if (nodeMap.find(v) == nodeMap.end()) {
                    int v2 = nodeMap.size();
                    nodeMap[v] = v2;
                    g.addTypedNode(this->getLabel(v), this->getNodeType(v));
                    nodeCounts.push_back(0); // 初始化节点计数
                }
                // 添加边到子图
                g.addEdge(nodeMap[u], nodeMap[v], edge.time(), this->getEdgeType(e));		
                nodeCounts[nodeMap[u]]++; // 更新源节点计数
                nodeCounts[nodeMap[v]]++; // 更新目标节点计数
            }
            edgeCounts[edgeMap[e]]++; // 更新边计数
        }
    }
    return g; // 返回创建的子图
}

// 根据多个子图匹配结果创建聚合子图
CertGraph CertGraph::createAggregateSubGraph(bool ignoreDir, const vector<GraphMatch> &subGraphs,
    vector<int> &edgeCounts) {
    edgeCounts.clear(); // 清空边计数

    // 用于跟踪节点对之间的唯一边
    unordered_map<int, unordered_map<int, unordered_set<int>>> uniqueEdges;
    // 映射节点对到新的边索引
    unordered_map<int, unordered_map<int, int>> edgeMap;

    CertGraph g(this->windowDuration()); // 创建一个新的CertGraph对象
    
    // 确定使用的节点，并在它们之间创建边
    unordered_map<int, int> nodeMap; // 节点索引映射
    for (const GraphMatch &gm : subGraphs) {
        const vector<int> &edges = gm.edges(); // 获取匹配的边列表
        for (int e : edges) {
            const Edge &edge = this->edges()[e]; // 获取边信息
            int u = edge.source(), v = edge.dest(); // 获取源节点和目标节点
            // 如果忽略方向且u > v，则交换u和v以避免重复计数
            if (ignoreDir && u > v) {
                swap(u, v);
            }
            // 如果节点对之间尚无边，则添加新边
            if (edgeMap[u].find(v) == edgeMap[u].end()) {
                edgeMap[u][v] = edgeCounts.size();
                edgeCounts.push_back(0); // 初始化边计数
                // 如果源节点尚未映射，则添加新节点
                if (nodeMap.find(u) == nodeMap.end()) {
                    int u2 = nodeMap.size();
                    nodeMap[u] = u2;            
                    g.addTypedNode(this->getLabel(u), this->getNodeType(u));
                }
                // 如果目标节点尚未映射，则添加新节点
                if (nodeMap.find(v) == nodeMap.end()) {
                    int v2 = nodeMap.size();
                    nodeMap[v] = v2;
                    g.addTypedNode(this->getLabel(v), this->getNodeType(v));
                }
                // 添加边到子图（边类型为空）
                g.addEdge(nodeMap[u], nodeMap[v], edge.time(), ""); 
            }
            // 确保边计数基于唯一边的数量
            int u2 = nodeMap[u], v2 = nodeMap[v];
            int e2 = edgeMap[u][v];
            uniqueEdges[u2][v2].insert(e);	    
            edgeCounts[e2] = uniqueEdges[u2][v2].size();	    
        }
    }
    return g; // 返回创建的聚合子图
}

// 添加一个带类型的节点
void CertGraph::addTypedNode(const std::string &name, const std::string &type) {
    if (!this->hasLabeledNode(name)) { // 如果节点不存在
        LabeledWeightedGraph::addLabeledNode(nodes().size(), name); // 调用基类方法添加节点
        int n = this->nodes().size(); // 获取当前节点数量
        int v = this->getIndex(name); // 获取节点索引
        if (v >= this->_nodeTypes.size()) {        
            _nodeTypes.resize(v + 1); // 调整节点类型数组大小
            _nodeOutEdgeTypes.resize(v + 1); // 调整出边类型数组大小
            _nodeInEdgeTypes.resize(v + 1); // 调整入边类型数组大小
        }
        _nodeTypes[v] = type; // 设置节点类型
    }
}

// 添加一条边（通过节点名称）
void CertGraph::addEdge(const std::string &source, const std::string &dest, time_t dateTime, const std::string &type) {
    if (!this->hasLabeledNode(source)) // 如果源节点不存在，则添加
        this->addLabeledNode(nodes().size(), source);
    if (!this->hasLabeledNode(dest)) // 如果目标节点不存在，则添加
        this->addLabeledNode(nodes().size(), dest);
    
    int u = this->getIndex(source); // 获取源节点索引
    int v = this->getIndex(dest); // 获取目标节点索引
    this->addEdge(u, v, dateTime, type); // 调用内部方法添加边
}

// 添加一条带类型的边（通过节点索引）
void CertGraph::addEdge(int u, int v, time_t dateTime, const std::string &type) {    
    LabeledWeightedGraph::addEdge(u, v, dateTime); // 调用基类方法添加边
    _timeEdgeTypeMap[dateTime].push_back(type); // 记录边类型
    
    // 获取该边类型的出度和入度
    vector<int> &outDegs = _typeOutDegs[type];
    vector<int> &inDegs = _typeInDegs[type];
    
    // 如果需要，调整映射大小
    int n = this->nodes().size();
    if (outDegs.size() < n) {
        outDegs.resize(n, 0);
        inDegs.resize(n, 0);
    }
    
    // 更新度数
    outDegs[u]++;
    inDegs[v]++;
    
    // 更新相邻边类型的集合
    if (u >= _nodeOutEdgeTypes.size())
        _nodeOutEdgeTypes.resize(u + 1);
    _nodeOutEdgeTypes[u].insert(type);
    if (v >= _nodeInEdgeTypes.size())
        _nodeInEdgeTypes.resize(v + 1);
    _nodeInEdgeTypes[v].insert(type);
}

// 获取指定边的类型
const std::string &CertGraph::getEdgeType(int edgeIndex) const {
    if (!_edgesReady) { // 如果边未排序，则更新
        this->updateOrderedEdges();
    }
    return _edgeTypes[edgeIndex]; // 返回边类型
}

// 获取指定节点的出度（针对特定边类型）
int CertGraph::getOutDeg(int v, const std::string &edgeType) const {
    const auto &pair = _typeOutDegs.find(edgeType); // 查找边类型
    if (pair == _typeOutDegs.end()) // 如果未找到
        return 0;
    else if (pair->second.size() <= v) // 如果节点索引超出范围
        return 0;
    else
        return pair->second[v]; // 返回出度
}

// 获取指定节点的入度（针对特定边类型）
int CertGraph::getInDeg(int v, const std::string &edgeType) const {
    const auto &pair = _typeInDegs.find(edgeType); // 查找边类型
    if (pair == _typeInDegs.end()) // 如果未找到
        return 0;
    else if (pair->second.size() <= v) // 如果节点索引超出范围
        return 0;
    else
        return pair->second[v]; // 返回入度
}

// 获取指定节点的出边类型集合
const unordered_set<string> &CertGraph::getOutEdgeTypes(int v) const { 
    if (v >= _nodeOutEdgeTypes.size()) { // 如果节点索引超出范围
        cout << "Missing out edge types for vertex " << v << endl;
        throw "Missing out edge types for vertex.";
    }
    return _nodeOutEdgeTypes[v]; // 返回出边类型集合
}

// 获取指定节点的入边类型集合
const unordered_set<string> &CertGraph::getInEdgeTypes(int v) const { 
    if (v >= _nodeInEdgeTypes.size()) { // 如果节点索引超出范围
        cout << "Missing in edge types for vertex " << v << endl;
        throw "Missing in edge types for vertex.";
    }
    return _nodeInEdgeTypes[v]; // 返回入边类型集合
}

// 获取指定节点的度限制条件
const vector<DegRestriction> &CertGraph::getDegRestrictions(int v) const {
    return this->_degRestricts.find(v)->second; // 返回度限制条件列表
}

// 判断指定节点是否有度限制条件
bool CertGraph::hasDegRestrictions(int v) const {
    return this->_degRestricts.find(v) != _degRestricts.end(); // 返回是否存在限制
}

// 添加度限制条件
void CertGraph::addDegRestriction(int v, const DegRestriction &restrict) {
    this->_degRestricts[v].push_back(restrict); // 添加限制条件
}

// 添加正则表达式匹配规则
void CertGraph::addRegex(int v, const std::regex &regex) {
    _regexMatch[v] = regex; // 设置正则表达式
}

// 显示图的信息
void CertGraph::disp() const {
    cout << nodes().size() << " nodes" << endl; // 输出节点数量
    for (int i = 0; i < nodes().size(); i++) {        
        cout << "  " << this->getLabel(i); // 输出节点标签
        const string &type = this->getNodeType(i); // 获取节点类型
        if (!type.empty())
            cout << " (" << type << ")"; // 输出节点类型
        if (this->hasDegRestrictions(i)) { // 如果有度限制条件
            const vector<DegRestriction> &restricts = this->getDegRestrictions(i);
            for (const DegRestriction &restrict : restricts) {
                cout << ", "; 
                if (restrict.isOutDeg()) // 如果是出度限制
                    cout << "OUT:";
                else
                    cout << "IN:"; // 如果是入度限制
                cout << restrict.edgeType(); // 输出边类型
                if (restrict.isLessThan()) // 如果是小于限制
                    cout << "<";
                else
                    cout << ">"; // 如果是大于限制
                cout << restrict.value(); // 输出限制值
            }
        }
        if (this->needsNameMatch(i)) // 如果需要精确名称匹配
            cout << " [Needs Exact Name Match]";
        cout << endl;
    }
    cout << numEdges() << " edges:" << endl; // 输出边数量
    for (int i = 0; i < numEdges(); i++) {
        cout << "  ";
        disp(i); // 显示每条边的详细信息
    }
}

// 显示指定边的详细信息
void CertGraph::disp(int edgeIndex) const {
    const Edge &edge = edges()[edgeIndex]; // 获取边信息
    int u = edge.source(), v = edge.dest(); // 获取源节点和目标节点
    const string &source = this->getLabel(u); // 获取源节点标签
    const string &dest = this->getLabel(v); // 获取目标节点标签
    const string &sourceType = this->getNodeType(u); // 获取源节点类型
    const string &destType = this->getNodeType(v); // 获取目标节点类型
    const string &edgeType = this->getEdgeType(edgeIndex); // 获取边类型
    time_t dateTime = edge.time(); // 获取时间戳
    
    cout << source; // 输出源节点标签
    if (!sourceType.empty())
        cout << " (" << sourceType << ")"; // 输出源节点类型
    if (!edgeType.empty())
        cout << " --" << edgeType << "--> "; // 输出边类型
    else
        cout << " -> ";
    cout << dest; // 输出目标节点标签
    if (!destType.empty())
        cout << " (" << destType << ")"; // 输出目标节点类型
    
    struct tm *timeInfo = gmtime(&dateTime); // 转换时间为可读格式
    cout << " | " << asctime(timeInfo); // 输出时间
}

// 更新有序边列表
void CertGraph::updateOrderedEdges() const {
    LabeledWeightedGraph::updateOrderedEdges(); // 确保基类已更新
    
    _edgeTypes.clear(); // 清空旧的边类型列表
    
    // 按时间顺序构建边列表
    for (auto &pair : _timeEdgeTypeMap) {
        time_t dateTime = pair.first; // 获取时间戳
        const vector<string> &types = _timeEdgeTypeMap.find(dateTime)->second; // 获取边类型列表
        for (const string &type : types) {
            _edgeTypes.push_back(type); // 添加边类型到列表
        }
    }
}