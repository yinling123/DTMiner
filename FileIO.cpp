#include "FileIO.h"
#include "LabeledWeightedGraph.h"
#include "FastReader.h"
#include "GraphMatch.h"
#include "SearchConfig.h"
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <exception>
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using namespace std;

// 定义常量字符串，用于配置文件解析
const std::string FileIO::STREAM_SECTION("stream"); // 流式数据部分
const std::string FileIO::IS_STREAMING("isStreaming"); // 是否流式处理
const std::string FileIO::STREAM_DUR("duration"); // 流持续时间
const std::string FileIO::STREAM_DELAY("delay"); // 流延迟
const std::string FileIO::STREAM_WIN("window"); // 流窗口
const std::string FileIO::DELTA("delta"); // 时间增量
const std::string FileIO::NUM_NODES("nodes"); // 节点数量
const std::string FileIO::NUM_LINKS("links"); // 边数量
const std::string FileIO::START_DATE("startDate"); // 开始日期
const std::string FileIO::END_DATE("endDate"); // 结束日期
const std::string FileIO::QUERIES_SECTION("queries"); // 查询部分
const std::string FileIO::SEL_SUBGRAPHS_SECTION("selected_subgraphs"); // 选定子图部分
const std::string FileIO::FIND_SUBGRAPHS_SECTION("find_subgraphs"); // 查找子图部分
const std::string FileIO::QUERY("query"); // 查询
const std::string FileIO::NODES("nodes"); // 节点
const std::string FileIO::LINKS("links"); // 边

/**
 * 加载边数据文件并构建图。
 * @param fname 文件名。
 * @return 构建的图对象。
 */
Graph FileIO::loadEdges(const std::string& fname)
{
    Graph g(0); // 初始化空图
    ifstream ifs(fname); // 打开文件
    time_t time = 0; // 时间戳初始化
    while(ifs.good()) // 逐行读取文件
    {        
        string line;
        std::getline(ifs, line); // 读取一行
        if(line.size() == 0 || line[0] == '#') // 跳过空行或注释行            
            continue;
        stringstream ss(line);       
        int u=0, v=0;
        ss >> u; // 解析起始节点
        ss >> v; // 解析目标节点
        g.addEdge(u,v,time); // 添加边到图中
        time++; // 增加时间戳
    }
    ifs.close(); // 关闭文件
    return g; // 返回图对象
}

/**
 * 加载SNAP格式的时间图数据。
 * @param fname 文件名。
 * @param g 图对象。
 */
void FileIO::loadSNAP(const string &fname, CertGraph &g)
{
    ifstream ifs(fname); // 打开文件
    string edgeType = ""; // 边类型初始化
    while(ifs.good()) // 逐行读取文件
    {
        string line;
        std::getline(ifs, line); // 读取一行
        if(line.size() == 0) // 跳过空行
            continue;
        stringstream ss(line);
        string uName, vName;
        ss >> uName; // 解析起始节点名称
        ss >> vName; // 解析目标节点名称
        time_t dateTime;
        ss >> dateTime; // 解析时间戳
        if(dateTime < 0) // 检查时间戳是否有效
            throw "时间值在时间SNAP数据中应为正整数 >= 0。";
        g.addEdge(uName, vName, dateTime, edgeType); // 添加边到图中
    }
    ifs.close(); // 关闭文件
}

/**
 * 加载带标签的边数据文件并构建图。
 * @param fname 文件名。
 * @return 构建的图对象。
 */
Graph FileIO::loadLabeledEdges(const string &fname)
{
    unordered_map<string,int> nameMap; // 节点名称映射表
    Graph g(0); // 初始化空图
    ifstream ifs(fname); // 打开文件
    time_t time = 0; // 时间戳初始化
    while(ifs.good()) // 逐行读取文件
    {        
        string line;
        std::getline(ifs, line); // 读取一行
        if(line.size() == 0 || line[0] == '#') // 跳过空行或注释行            
            continue;
        stringstream ss(line);       
        string uName, vName;
        ss >> uName; // 解析起始节点名称
        ss >> vName; // 解析目标节点名称
        if(nameMap.find(uName)==nameMap.end()) // 如果节点未映射
            nameMap[uName] = nameMap.size()-1; // 分配新ID
        if(nameMap.find(vName)==nameMap.end())
            nameMap[vName] = nameMap.size()-1;
        int u = nameMap[uName]; // 获取起始节点ID
        int v = nameMap[vName]; // 获取目标节点ID        
        if(u != v) // 避免自环
        {
            g.addEdge(u,v,time); // 添加边到图中
            time++; // 增加时间戳
        }
    }
    ifs.close(); // 关闭文件
    return g; // 返回图对象
}

/**
 * 加载Pajek格式的带标签加权图。
 * @param fname 文件名。
 * @return 构建的带标签加权图对象。
 */
LabeledWeightedGraph FileIO::loadPajek(const string& fname)
{
    LabeledWeightedGraph g(0); // 初始化空图
    ifstream ifs(fname);    
    time_t time = 0; // 时间戳初始化
    string line;
    string blank;
    ifs >> blank; // 跳过文件头
    int n;
    ifs >> n; // 读取节点数量
    while(ifs.good()) // 逐行读取节点数据
    {
        std::getline(ifs, line);
        if(line[0] == '*') // 跳过"*Arcs"标记
            break;
        stringstream ss(line);
        int v;
        ss >> v;
        v--; // 节点ID从0开始
        string label;
        ss >> label; // 读取节点标签
        try
        {
            g.addLabeledNode(v, label); // 添加带标签节点到图中
        }
        catch(exception &e) // 异常处理
        {
            cout << "异常: " << e.what() << endl;
            cout << "行: " << line << endl;
        }
    }    
    while(ifs.good()) // 逐行读取边数据
    {
        std::getline(ifs, line);
        if(line.size() <= 1) // 跳过空行
            break;
        stringstream ss(line);
        int u, v;
        double w;
        ss >> u; // 解析起始节点ID
        ss >> v; // 解析目标节点ID
        ss >> w; // 解析权重
        u--; v--; // 节点ID从0开始
        try
        {
            g.addWeightedEdge(u, v, time, w); // 添加加权边到图中
            time++; // 增加时间戳
        }
        catch(exception &e) // 异常处理
        {
            cout << "异常: " << e.what() << endl;
            cout << "行: " << line << endl;
        }
    }
    ifs.close(); // 关闭文件
    return g; // 返回图对象
}

/**
 * 加载CERT GDF格式的图数据。
 * @param fname 文件名。
 * @param g 图对象。
 */
void FileIO::loadCertGDF(const std::string &fname, CertGraph &g)
{
    int lineNum = 0; // 行号计数器
    bool nodeData = false, edgeData = false; // 标记当前数据部分
    FastReader csv(fname); // 使用FastReader读取CSV文件
    while(csv.good()) // 逐行读取文件
    {
        char **vars = csv.next(); // 获取当前行数据
        int nc = csv.rowSize(); // 获取列数
        lineNum++;
        try
        {
            if(nc > 1) // 忽略空行
            {
                if(vars[0][0] == '\0' || vars[0][0] == '#') // 跳过注释行
                    continue;

                if(nodeData) // 处理节点数据
                {
                    if(strncmp(vars[0], "edgedef>", 8) == 0) // 切换到边数据部分
                    {
                        cout << "读取边数据" << endl;
                        nodeData = false;
                        edgeData = true;
                        continue;
                    }
                    g.addTypedNode(vars[0],vars[1]); // 添加节点到图中
                    
                    if(nc == 3 && vars[2][0] != '\0') // 处理度限制
                    {
                        FileIO::addNodeRestrictions(vars[0], vars[2], g);
                    }
                }
                else if(edgeData) // 处理边数据
                {		    
                    if(nc != 4) // 检查列数是否正确
                    {
                        cerr << "错误: 行有 " << nc << " 列，而不是4列。" << endl;
                        throw "读取GDF文件时出错。边数据应有4列。";                    
                    }
                    time_t dateTime = (long)(atof(vars[3])); // 解析时间戳
                    g.addEdge(vars[0], vars[1], dateTime, vars[2]); // 添加边到图中
                }
                else // 查找节点数据部分
                {
                    if(strncmp(vars[0], "nodedef>", 8) == 0) // 切换到节点数据部分
                    {
                        cout << "读取节点数据" << endl;
                        nodeData = true;
                        edgeData = false;
                        continue;
                    }
                    else
                        throw "读取GDF文件时出错。未找到nodedef>部分。";
                }
            }
        }
        catch(exception &e) // 异常处理
        {
            cerr << "第 #" << lineNum << " 行出现问题" << endl;
            cerr << "问题行: " << endl;
            csv.dispRow();
            cerr << e.what() << endl;
            throw e.what();            
        }
    }
    csv.close(); // 关闭文件
    if(edgeData == false) // 检查是否找到边数据部分
        throw "读取GDF文件时出错。未找到edgedef>部分。";
}

/**
 * 加载指定文件夹中的所有CERT GDF文件并构建图。
 * @param folder 文件夹路径。
 * @return 构建的图对象。
 */
CertGraph FileIO::loadCertGDFs(const std::string &folder)
{
    vector<string> fnames = FileIO::getFileNames(folder); // 获取文件夹中的文件名列表
    CertGraph g(0); // 初始化空图
    for(const string &fname : fnames) // 遍历文件名
    {
        if(fname.size() > 4 && fname.substr(fname.size()-4,4).compare(".gdf")==0) // 检查扩展名
        {
            cout << "添加边来自: " << fname << endl;
            FileIO::loadCertGDF(fname, g); // 加载单个GDF文件
            cout << "   总节点数: " << g.numNodes() << endl;
            cout << "   总边数: " << g.numEdges() << endl;
        }
        else
            cout << "忽略文件: " << fname << endl;
    }
    return g; // 返回图对象
}

/**
 * 加载单个CERT GDF文件并构建图。
 * @param fname 文件名。
 * @return 构建的图对象。
 */
CertGraph FileIO::loadCertGDF(const std::string &fname)
{    
    CertGraph g(0); // 初始化空图
    FileIO::loadCertGDF(fname, g); // 调用加载函数
    return g; // 返回图对象
}

/**
 * 添加节点的度限制。
 * @param nodeID 节点ID。
 * @param restrictions 限制字符串。
 * @param g 图对象。
 */
void FileIO::addNodeRestrictions(const std::string &nodeID, const std::string &restrictions, CertGraph &g)
{
    cout << "节点限制文本: " << nodeID << ": " << restrictions << endl;
    int v = g.getIndex(nodeID); // 获取节点索引
    stringstream ss;
    ss << restrictions;
    while(ss.good()) // 解析限制字符串
    {        
        string var;
        if(ss.str().find('&') != string::npos)
            std::getline(ss, var, '&'); // 按"&"分割
        else
            std::getline(ss, var);

        bool isOutDeg = false; // 是否为出度限制
        int startIndex = 0;
        if(var.size() >= 4 && var.substr(0,4) == "OUT:") // 出度限制
        {
            isOutDeg = true;
            startIndex = 4;
        }
        else if(var.size() >= 3 && var.substr(0,3) == "IN:") // 入度限制
        {
            isOutDeg = false;
            startIndex = 3;
        }
        else if(var.size() >= 6 && var.substr(0,6) == "REGEX:") // 正则表达式限制
        {
            try
            {
                cout << "添加正则表达式限制: \"" << var.substr(6) << "\"" << endl;
                regex rx(var.substr(6));
                g.addRegex(v, rx);
            }
            catch(...)
            {
                cout << "创建正则表达式时出现问题: ";
                cout << "\"" << var.substr(6) << "\"" << endl;
                cout << "确保输入是有效的POSIX正则表达式。" << endl;
            }
            continue;
        }
        else if(var == "NameMatch") // 名称匹配限制
        {  
            g.setNeedsNameMatch(v);
            cout << "要求节点名称精确匹配: " << nodeID << endl;
            continue;
        }
        else
            throw "未知的节点限制。选项包括 'OUT:', 'IN:', 'REGEX:' 和 'NameMatch'";
        
        bool isLessThan = false; // 是否为小于限制
        string inequalityStr = var.substr(startIndex);
        cout << "添加节点限制: " << nodeID << ": " << inequalityStr << endl;
        int i = inequalityStr.find('<');
        if(i != string::npos)
        {
            isLessThan = true;
        }
        else
        {
            i = inequalityStr.find('>');
            if(i != string::npos)
            {
                isLessThan = false;
            }
            else
                throw "限制不等式未找到。必须是 '>' 或 '<'。";
        }
        
        string type = inequalityStr.substr(0,i); // 边类型
        int value = atoi(inequalityStr.substr(i+1).c_str()); // 限制值
        
        DegRestriction restrict(type, isOutDeg, isLessThan, value); // 创建限制对象
        g.addDegRestriction(v, restrict); // 添加限制到图中
    }
}

/**
 * 保存CertGraph到GDF文件（无额外数据）。
 * @param g 图对象。
 * @param fname 文件名。
 */
void FileIO::saveCertGDF(const CertGraph &g, const string &fname)
{
    map<string,vector<int>> blank; // 空的额外数据
    saveCertGDF(g, fname, blank, blank); // 调用带额外数据的重载函数
}

/**
 * 保存CertGraph到GDF文件（支持额外数据）。
 * @param g 图对象。
 * @param fname 文件名。
 * @param extraEdgeValues 额外边属性值。
 * @param extraNodeValues 额外节点属性值。
 */
void FileIO::saveCertGDF(const CertGraph &g, const string &fname,
        const map<string,vector<int>> &extraEdgeValues,
        const map<string,vector<int>> &extraNodeValues)
{
    ofstream ofs(fname.c_str()); // 打开输出文件
    
    // 写入节点数据部分
    ofs << "nodedef>name VARCHAR,type VARCHAR"; // 定义节点属性
    for(const auto &pair : extraNodeValues) // 添加额外节点属性
    {
        ofs << "," << pair.first << " DOUBLE";
    }
    ofs << endl;
    int n = g.nodes().size(); // 获取节点数量
    for(int v=0; v<n; v++) // 遍历所有节点
    {
        ofs << g.getLabel(v) << "," << g.getNodeType(v); // 写入节点名称和类型
        for(const auto &pair : extraNodeValues) // 写入额外节点属性值
        {
            ofs << "," << pair.second[v];
        }
        ofs << endl;
    }
    
    // 写入边数据部分
    ofs << "edgedef>node1 VARCHAR,node2 VARCHAR,type VARCHAR,date DOUBLE"; // 定义边属性
    for(const auto &pair : extraEdgeValues) // 添加额外边属性
    {
        ofs << "," << pair.first << " DOUBLE";
    }
    ofs << endl;
    int m = g.numEdges(); // 获取边数量
    for(int e=0; e<m; e++) // 遍历所有边
    {
        const Edge &edge = g.edges()[e]; // 获取边信息
        ofs << g.getLabel(edge.source()) << "," << g.getLabel(edge.dest()) << "," 
            << g.getEdgeType(e) << "," << edge.time(); // 写入边的起始节点、目标节点、类型和时间
        for(const auto &pair : extraEdgeValues) // 写入额外边属性值
        {
            ofs << "," << pair.second[e];
        }
        ofs << endl;
    }
    ofs.close(); // 关闭文件
}

/**
 * 加载通用GDF格式的图数据。
 * @param fname 文件名。
 * @return 构建的DataGraph对象。
 * 一次性加载完成点信息和边信息
 */
DataGraph FileIO::loadGenericGDF(const string &fname)
{
    DataGraph g; // 初始化空图
    int lineNum = 0; // 行号计数器
    bool isNodeData = false, isEdgeData = false; // 标记当前数据部分
    int sourceCol = -1, destCol = -1, timeCol = -1, nameCol = -1; // 列索引
    int numNodeCols = 0, numEdgeCols = 0; // 节点和边的列数

    FastReader csv(fname); // 使用FastReader读取CSV文件
    // 进行每行读取，分别读取点数据和边数据
    while(csv.good()) // 逐行读取文件
    {
        char **vars = csv.next(); // 获取当前行数据
        int numVars = csv.rowSize(); // 获取列数
        lineNum++;
        if(lineNum % 100000 == 0) // 每10万行打印一次进度
            cout << "解析第 #" << lineNum << " 行" << endl;
        if(numVars == 0 || (numVars == 1 && (vars[0][0] == '\0' || vars[0][0] == '#'))) // 跳过空行或注释行
            continue;

        try
        {
            if(!isNodeData && !isEdgeData) // 查找节点数据部分
            {
                if(strncmp(vars[0],"nodedef>",8)==0) // 找到节点定义部分
                {
                    cout << "读取节点数据" << endl;
                    isNodeData = true;
                    AttributesDef def = getAttributesDef(isNodeData, vars, numVars); // 获取节点属性定义
                    for(int i=0; i<def.attributeNames().size(); i++)
                    {
                        if(def.attributeNames()[i] == "name") // 找到"name"列
                            nameCol = i;
                    }
                    if(nameCol < 0)
                        throw "在\"nodedef>\"中缺少\"name\"列";
                    g.setNodeAttributesDef(def); // 设置节点属性定义
                    numNodeCols = def.attributeNames().size();
                    continue;
                }
                else
                    throw "读取GDF文件时出错。未找到nodedef>部分。";
            }

            if(isNodeData) // 处理节点数据
            {
                if(strncmp(vars[0],"edgedef>",8)==0) // 切换到边数据部分
                {
                    cout << "读取边数据" << endl;
                    isNodeData = false;
                    isEdgeData = true;

                    AttributesDef def = getAttributesDef(isNodeData, vars, numVars); // 获取边属性定义
                    const auto &names = def.attributeNames();
                    for(int i=0; i<names.size(); i++)
                    {
                        const string &name = names[i];
                        if(name == "node1")
                            sourceCol = i;
                        else if(name == "node2")
                            destCol = i;
                        else if(name == "date" || name == "time")
                            timeCol = i;
                    }
                    if(sourceCol < 0)
                        throw "在\"edgedef>\"中缺少\"node1\"列";
                    if(destCol < 0)
                        throw "在\"edgedef>\"中缺少\"node2\"列";
                    if(timeCol < 0)
                        throw "在\"edgedef>\"中缺少\"time\"列";
                    numEdgeCols = names.size();
                    g.setEdgeAttributesDef(def); // 设置边属性定义
                    continue;
                }
                if(numVars != numNodeCols) // 检查列数是否匹配
                    throw "节点部分列数不匹配。";

                Attributes a;
                FileIO::addAttributeValues(g.nodeAttributesDef(), vars, a); // 解析节点属性值
                g.addNode(vars[nameCol], a); // 添加节点到图中
            }

            if(isEdgeData) // 处理边数据
            {
                if(numVars != numEdgeCols) // 检查列数是否匹配
                    throw "边部分列数不匹配。";

                Attributes a;
                FileIO::addAttributeValues(g.edgeAttributesDef(), vars, a); // 解析边属性值
                g.addEdge(vars[sourceCol], vars[destCol], atoi(vars[timeCol]), a); // 添加边到图中
            }
        }
        catch(exception &e) // 异常处理
        {
            cerr << "第 #" << lineNum << " 行出现问题" << endl;
            cerr << "问题行: ";
            csv.dispRow();
            cerr << e.what() << endl;
            throw e.what();
        }
    }
    csv.close(); // 关闭文件
    if(isEdgeData == false) // 检查是否找到边数据部分
        throw "读取GDF文件时出错。未找到edgedef>部分。";

    return g; // 返回图对象
}

/**
 * 保存DataGraph到通用GDF文件。
 * @param g 图对象。
 * @param fname 文件名。
 */
void FileIO::saveGenericGDF(const DataGraph &g, const string &fname)
{
    ofstream ofs(fname); // 打开输出文件

    // 写入节点头部
    ofs << "nodedef>";
    ofs << "name VARCHAR";
    FileIO::saveAttributesHeader(g.nodeAttributesDef(), ofs); // 写入节点属性头部

    const AttributesDef &nodeDef = g.nodeAttributesDef();
    const AttributesDef &edgeDef = g.edgeAttributesDef();
    int numNodeAtts = nodeDef.attributeNames().size();
    int numEdgeAtts = edgeDef.attributeNames().size();

    // 写入节点数据
    int n = g.nodes().size();
    for(int u=0; u<n; u++) // 遍历所有节点
    {
        const Attributes &a = g.nodeAttributes()[u];
        int si=0, ii=0, fi=0; // 字符串、整数、浮点数值索引
        for(int i=0; i<numNodeAtts; i++) // 遍历所有属性
        {
            AttributeType attType = nodeDef.attributeTypes()[i];
            if(attType == AttributeType::DATA) // 数据属性
            {
                DataType dataType = nodeDef.dataTypes()[i];
                if(dataType == DataType::STRING)
                    ofs << a.stringValues()[si++];
                else if(dataType == DataType::INT)
                    ofs << a.intValues()[ii++];
                else if(dataType == DataType::FLOAT)
                    ofs << a.floatValues()[fi++];
                else
                    throw "无法保存节点属性。未知的数据类型。";
            }
            else if(attType == AttributeType::NODE_ID) // 节点ID属性
            {
                ofs << g.getName(u);
            }
            else
            {
                throw "节点部分发现不合适的属性类型。";
            }
            if(i < numNodeAtts-1)
                ofs << ",";
        }
        ofs << endl;
    }

    // 写入边头部
    ofs << "edgedef>";
    FileIO::saveAttributesHeader(g.edgeAttributesDef(), ofs); // 写入边属性头部

    // 写入边数据
    int m = g.numEdges();
    for(int e=0; e<m; e++) // 遍历所有边
    {
        const Edge &edge = g.edges()[e];
        const Attributes &a = g.edgeAttributes()[e];
        int si=0, ii=0, fi=0; // 字符串、整数、浮点数值索引
        for(int i=0; i<numEdgeAtts; i++) // 遍历所有属性
        {
            AttributeType attType = edgeDef.attributeTypes()[i];
            if(attType == AttributeType::DATA) // 数据属性
            {
                DataType dataType = edgeDef.dataTypes()[i];
                if(dataType == DataType::STRING)
                    ofs << a.stringValues()[si++];
                else if(dataType == DataType::INT)
                    ofs << a.intValues()[ii++];
                else if(dataType == DataType::FLOAT)
                    ofs << a.floatValues()[fi++];
                else
                    throw "无法保存边属性。未知的数据类型。";
            }
            else if(attType == AttributeType::SOURCE_ID) // 起始节点ID属性
            {
                ofs << g.getName(edge.source());
            }
            else if(attType == AttributeType::DEST_ID) // 目标节点ID属性
            {
                ofs << g.getName(edge.dest());
            }
            else if(attType == AttributeType::TIME) // 时间属性
            {
                ofs << edge.time();
            }
            else
            {
                throw "边部分发现不合适的属性类型。";
            }
            if(i < numEdgeAtts-1)
                ofs << ",";
        }
        ofs << endl;
    }

    ofs.close(); // 关闭文件
}

/**
 * 保存节点计数结果到CSV文件。
 * @param g 图对象。
 * @param subgraphs 子图匹配结果。
 * @param h 查询图。
 * @param timeCounts 时间计数。
 * @param numTimeSlices 时间片数量。
 * @param startTime 开始时间。
 * @param endTime 结束时间。
 * @param fname 文件名。
 */
void FileIO::saveNodeCount(const LabeledWeightedGraph &g, const vector<GraphMatch> &subgraphs, 
                           const LabeledWeightedGraph &h, const unordered_map<string,vector<int>> &timeCounts, 
                           int numTimeSlices, time_t startTime, time_t endTime, const string &fname)
{
    unordered_map<int,unordered_map<int,int>> nodeCounts; // 节点计数
    for(GraphMatch gm : subgraphs) // 遍历所有子图匹配结果
    {
        for(int g_u : gm.nodes()) // 遍历子图中的节点
        {
            int h_u = gm.getQueryNode(g_u); // 获取查询图中的对应节点
            if(nodeCounts[g_u].find(h_u) == nodeCounts[g_u].end())
                nodeCounts[g_u][h_u] = 1; // 初始化计数
            else
                nodeCounts[g_u][h_u]++; // 增加计数
        }
    }

    ofstream ofs(fname); // 打开输出文件

    // 写入表头
    ofs << "节点名称, 查询节点, 子图计数";
    time_t sliceDur = (endTime - startTime)/numTimeSlices; // 计算时间片长度
    for(int i=0; i<numTimeSlices; i++) // 遍历所有时间片
    {
        time_t sliceStart = startTime + sliceDur*i;
        time_t sliceEnd = sliceStart + sliceDur;
        ofs << ",时间计数 " << sliceStart << " - " << sliceEnd;
    }
    ofs << endl;

    // 写入数据
    for(auto pair : nodeCounts) // 遍历所有节点计数
    {
        int g_u = pair.first;
        for(auto countPair : pair.second)
        {
            int h_u = countPair.first;
            int count = countPair.second;
            const string &label = g.getLabel(g_u);
            ofs << label << "," << h.getLabel(h_u) << "," << count;
            if(timeCounts.find(label) == timeCounts.end()) // 如果没有时间计数
            {
                for(int i=0; i<numTimeSlices; i++)
                    ofs << ",0";
            }
            else // 输出时间计数
            {
                const vector<int> &uTimeCounts = timeCounts.find(label)->second;
                for(int count : uTimeCounts)
                {
                    ofs << "," << count;
                }
            }
            ofs << endl;
        }
    }
    ofs.close(); // 关闭文件
}

/**
 * 保存查询图及其匹配结果到GDF文件。
 * @param query 查询图。
 * @param subgraphs 子图匹配结果。
 * @param fname 文件名。
 */
void FileIO::saveQueryGraph(const CertGraph &query, const vector<GraphMatch> &subgraphs, const string &fname)
{
    int h_n = query.numNodes(), h_m = query.numEdges(); // 获取查询图的节点和边数量

    // 统计子图匹配结果中节点和边的映射关系
    vector<unordered_set<int>> nodeMap(h_n), edgeMap(h_m);
    for(const GraphMatch &subgraph : subgraphs)
    {
        for(int g_u : subgraph.nodes()) // 遍历子图中的节点
        {
            int h_u = subgraph.getQueryNode(g_u); // 获取查询图中的对应节点
            nodeMap[h_u].insert(g_u);
        }
        for(int g_e : subgraph.edges()) // 遍历子图中的边
        {
            int h_e = subgraph.getQueryEdge(g_e); // 获取查询图中的对应边
            edgeMap[h_e].insert(g_e);
        }
    }

    // 将计数作为额外信息存储
    map<string,vector<int>> nodeData, edgeData;
    vector<int> nodeCounts(h_n), edgeCounts(h_m);
    for(int h_v=0; h_v<h_n; h_v++) // 遍历查询图中的节点
    {
        nodeCounts[h_v] = nodeMap[h_v].size(); // 统计唯一节点数量
    }
    for(int h_e=0; h_e<h_m; h_e++) // 遍历查询图中的边
    {
        edgeCounts[h_e] = edgeMap[h_e].size(); // 统计唯一边数量
    }
    string COUNT_LABEL = "count";
    nodeData[COUNT_LABEL] = nodeCounts;
    edgeData[COUNT_LABEL] = edgeCounts;

    // 保存查询图
    FileIO::saveCertGDF(query, fname, edgeData, nodeData);
}

/**
 * 获取指定文件夹中的所有文件名。
 * @param folder 文件夹路径。
 * @return 文件名列表。
 */
vector<string> FileIO::getFileNames(const string &folder)
{
    string folderName = folder;
    // 将路径中的反斜杠替换为正斜杠
    for(char &c : folderName)
    {
        if(c == '\\')
            c = '/';
    }
    // 确保路径以正斜杠结尾
    if(folderName.back() != '/')
        folderName.push_back('/');
    
    vector<string> out; // 存储文件名
    struct dirent *ent; // 目录项指针
    DIR *dir = opendir(folderName.c_str()); // 打开目录
    if(dir != 0) // 如果目录打开成功
    {
        while(true) 
        {
            ent = readdir(dir); // 读取目录项
            if(ent == 0) // 如果没有更多项，退出循环
                break;
            const string fname = ent->d_name; // 获取文件名
            if (fname[0] == '.') // 跳过隐藏文件
                continue;
            const string fullFname = folderName + fname; // 构建完整路径
            out.push_back(fullFname); // 添加到结果列表
        }    
        closedir(dir); // 关闭目录
    }
    return out; // 返回文件名列表
}

/**
 * 获取指定文件夹中具有特定扩展名的所有文件名。
 * @param folder 文件夹路径。
 * @param ext 文件扩展名。
 * @return 符合条件的文件名列表。
 */
vector<string> FileIO::getFileNames(const string &folder, const string &ext)
{
    vector<string> fnames = FileIO::getFileNames(folder); // 获取所有文件名
    vector<string> outFnames; // 存储符合条件的文件名
    for(const string &fname : fnames) // 遍历文件名
    {
        int n = fname.size();
        int n2 = ext.size();
        if(n > n2) // 检查文件名长度是否大于扩展名长度
        {
            if(fname.substr(n-n2) == ext) // 检查文件扩展名是否匹配
                outFnames.push_back(fname);       
        }
    }
    return outFnames; // 返回符合条件的文件名列表
}

/**
 * 获取文件的最后修改时间。
 * @param fname 文件名。
 * @return 文件的最后修改时间（时间戳）。
 */
time_t FileIO::getFileDate(const string &fname)
{
    struct stat fileStat; // 文件状态结构体
    lstat(fname.c_str(), &fileStat); // 获取文件状态
    return fileStat.st_mtime; // 返回最后修改时间
}

/**
 * 从完整路径中提取文件名。
 * @param path 文件路径。
 * @return 文件名。
 */
string FileIO::getFname(const std::string &path)
{
    size_t pos = path.rfind('/'); // 查找最后一个正斜杠
    if(pos == string::npos)
        pos = path.rfind('\\'); // 如果没有正斜杠，查找反斜杠
    if(pos != string::npos)
        return path.substr(pos+1); // 提取文件名
    return path; // 如果没有找到分隔符，返回原始路径
}

/**
 * 检查文件是否存在。
 * @param fname 文件名。
 * @return 如果文件存在返回 true，否则返回 false。
 */
bool FileIO::fileExists(const std::string &fname)
{
    ifstream ifs(fname); // 尝试打开文件
    return ifs.good(); // 如果文件流有效，说明文件存在
}

/**
 * 根据输入文件名和输出文件夹生成输出图文件名。
 * @param fname 输入文件名。
 * @param outFolder 输出文件夹。
 * @return 生成的输出图文件名。
 */
string FileIO::makeOutGraphFname(const std::string &fname, const std::string &outFolder)
{
    string outGraphFname = outFolder; // 初始化输出文件名
    string fname2 = getFname(fname); // 提取输入文件名
    outGraphFname.append(fname2.substr(0,fname2.size()-4)); // 去掉扩展名
    outGraphFname.append(".outgraph.gdf"); // 添加新的扩展名
    return outGraphFname; // 返回生成的文件名
}

/**
 * 根据输入文件名和输出文件夹生成节点计数文件名。
 * @param fname 输入文件名。
 * @param outFolder 输出文件夹。
 * @return 生成的节点计数文件名。
 */
string FileIO::makeNodeCountFname(const std::string &fname, const std::string &outFolder)
{
    string nodeCountFname = outFolder; // 初始化输出文件名
    string fname2 = getFname(fname); // 提取输入文件名
    nodeCountFname.append(fname2.substr(0,fname2.size()-4)); // 去掉扩展名
    nodeCountFname.append(".nodecount.csv"); // 添加新的扩展名
    return nodeCountFname; // 返回生成的文件名
}

/**
 * 从变量数组中提取属性名称。
 * @param vars 变量数组。
 * @param numVars 变量数量。
 * @return 属性名称列表。
 */
vector<string> FileIO::getAttributeNames(char **vars, int numVars)
{
    vector<string> names; // 存储属性名称
    if(numVars <= 0) // 如果没有变量，直接返回空列表
        return names;

    names.resize(numVars); // 初始化属性名称列表
    string front = vars[0]; // 获取第一个变量
    size_t pos1 = front.find('>'); // 查找 '>' 符号
    if(pos1 == string::npos)
        throw "无法在属性定义行中找到 '>'。";
    size_t pos2 = front.rfind(' '); // 查找最后一个空格
    if(pos2 == string::npos)
        throw "无法在属性定义行中找到空格。";
    pos1++;

    names[0] = front.substr(pos1, pos2-pos1); // 提取第一个属性名称
    for(int i=1; i<numVars; i++) // 遍历剩余变量
    {
        string var = vars[i];
        int pos = var.find(' '); // 查找空格
        if(pos == string::npos)
            throw "无法在属性行中找到空格。";
        names[i] = var.substr(0, pos); // 提取属性名称
    }
    return names; // 返回属性名称列表
}

/**
 * 根据变量数组生成属性定义。
 * @param isNodeData 是否是节点数据。
 * @param vars 变量数组。
 * @param numVars 变量数量。
 * @return 属性定义对象。
 */
AttributesDef FileIO::getAttributesDef(bool isNodeData, char **vars, int numVars)
{
    AttributesDef a; // 初始化属性定义
    for(int i=0; i<numVars; i++) // 遍历变量
    {
        string var = vars[i];
        size_t pos1 = 0;
        if(i ==0) // 处理第一个变量
        {
            pos1 = var.find('>');
            if(pos1 == string::npos)
                pos1 = 0;
            else
            {
                do
                {
                    pos1++;
                } while(var[pos1] == ' ' || var[pos1] == '\t'); // 跳过空白字符
            }
        }
        size_t pos2 = var.find(' ', pos1); // 查找空格
        if(pos2 == string::npos)
            throw "无法在属性行中找到空格。";

        string name = var.substr(pos1,pos2-pos1); // 提取属性名称
        string typeStr = var.substr(pos2+1); // 提取数据类型字符串
        DataType dataType = DataType::STRING; // 默认数据类型为字符串
        if(typeStr == "INT" or typeStr == "INTEGER")
            dataType = DataType::INT; // 整数类型
        else if(typeStr == "FLOAT" or typeStr == "DOUBLE")
            dataType = DataType::FLOAT; // 浮点类型
        else if(typeStr != "VARCHAR")
        {
            cerr << "当前不支持使用此数据类型的图: \"" << typeStr << "\"" << endl;
            throw "GDF 文件中未知的数据类型。";
        }

        AttributeType attType = AttributeType::DATA; // 默认属性类型为数据
        if(name == "name" && isNodeData)
            attType = AttributeType::NODE_ID; // 节点 ID
        else if(name == "node1" && !isNodeData)
            attType = AttributeType::SOURCE_ID; // 起始节点 ID
        else if(name == "node2" && !isNodeData)
            attType = AttributeType::DEST_ID; // 目标节点 ID
        else if(name == "time" || name == "date" && !isNodeData)
            attType = AttributeType::TIME; // 时间属性

        a.addAttribute(attType, dataType, name); // 添加属性定义
    }
    return a; // 返回属性定义对象
}

/**
 * 保存属性头部信息到文件。
 * @param def 属性定义。
 * @param ofs 输出文件流。
 */
void FileIO::saveAttributesHeader(const AttributesDef& def, std::ofstream& ofs)
{
    int n = def.attributeNames().size(); // 获取属性数量
    for(int i=0; i<n; i++) // 遍历所有属性
    {
        const string &name = def.attributeNames()[i];        
        ofs << name << " "; // 写入属性名称
        DataType type = def.dataTypes()[i]; // 获取数据类型
        if(type == DataType::STRING)
            ofs << "VARCHAR"; // 字符串类型
        else if(type == DataType::INT)
            ofs << "INT"; // 整数类型
        else if(type == DataType::FLOAT)
            ofs << "FLOAT"; // 浮点类型
        else
            throw "保存属性头部时出现问题。属性定义中未知的数据类型。";
        if(i < n-1)
            ofs << ","; // 添加逗号分隔符
    }    
    ofs << endl; // 换行
}

/**
 * 根据属性定义添加属性值。
 * @param def 属性定义。
 * @param vars 变量数组。
 * @param a 属性对象。
 */
void FileIO::addAttributeValues(const AttributesDef &def, char **vars, Attributes &a)
{
    const auto &dataTypes = def.dataTypes(); // 获取数据类型列表
    const auto &attTypes = def.attributeTypes(); // 获取属性类型列表
    int numCols = attTypes.size(); // 获取列数
    for(int i=0; i<numCols; i++) // 遍历所有列
    {
        if(attTypes[i] == AttributeType::DATA) // 如果是数据属性
        {
            DataType type = dataTypes[i]; // 获取数据类型
            const char *var = vars[i]; // 获取变量值
            if(type == DataType::STRING) // 字符串类型
            {
                if(var[0] == '\0') // 如果为空
                {
                    StringRestrictions r;
                    r.allowAny(); // 允许任意值
                    a.addStringValue(r);
                }
                else
                    a.addStringValue(var); // 添加字符串值
            }
            else if(type == DataType::FLOAT) // 浮点类型
            {
                if(var[0] == '\0') // 如果为空
                {
                    FloatRestrictions r;
                    r.allowAny(); // 允许任意值
                    a.addFloatValue(r);
                }
                else if(var[0] == '>') // 最小值限制
                {
                    FloatRestrictions r;
                    float val = atof(var+1);
                    cout << "设置最小值: " << val << endl;
                    r.setMin(val);
                    a.addFloatValue(r);
                }
                else if(var[0] == '<') // 最大值限制
                {
                    FloatRestrictions r;
                    float val = atof(var+1);
                    cout << "设置最大值: " << val << endl;
                    r.setMax(val);
                    a.addFloatValue(r);
                }
                else                            
                    a.addFloatValue(atof(var)); // 添加浮点值
            }
            else if(type == DataType::INT) // 整数类型
            {
                if(var[0] == '\0') // 如果为空
                {
                    IntRestrictions r;
                    r.allowAny(); // 允许任意值
                    a.addIntValue(r);
                }
                else if(var[0] == '>') // 最小值限制
                {
                    IntRestrictions r;
                    int val = atoi(var+1);
                    cout << "设置最小值: " << val << endl;
                    r.setMin(val);
                    a.addIntValue(r);
                }
                else if(var[0] == '<') // 最大值限制
                {
                    IntRestrictions r;
                    int val = atoi(var+1);
                    cout << "设置最大值: " << val << endl;
                    r.setMax(val);
                    a.addIntValue(r);
                }
                else                            
                    a.addIntValue(atoi(var)); // 添加整数值
            }
            else
                throw "无法添加属性。未知的数据类型。";
        }
    }  
}