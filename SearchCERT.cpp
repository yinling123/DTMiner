// #include "SearchCERT.h"
// #include "FastReader.h"
// #include "FileIO.h"
// #include "GraphFilter.h"
// #include "GraphSearch.h"
// #include "MatchCriteria_CERT.h"
// #include "Tools.h"
// #include <fstream>
// #include <iostream>
// #include <map>
// #include <sstream>
// #include <ctime>

// using namespace std;

// /**
//  * 构造函数，初始化 SearchCERT 对象。
//  * @param dataGraph 数据图对象。
//  * @param delta 时间窗口的增量（秒）。
//  * @param limit 匹配子图的最大数量限制。
//  * @param stdOut 是否将结果输出到标准输出。
//  * @param unordered 是否搜索无序子图。
//  * @param saveAllGraphs 是否保存所有匹配子图。
//  * @param saveAllEdges 是否保存所有边信息。
//  * @param outFolder 输出文件夹路径。
//  */
// SearchCERT::SearchCERT(const CertGraph &dataGraph, int delta, int limit, bool stdOut, bool unordered, bool saveAllGraphs, bool saveAllEdges, const string &outFolder) : _g2(dataGraph.windowDuration())
// {
//     _dataGraph = &dataGraph;
//     _delta = delta;
//     _limit = limit;
//     _stdOut = stdOut;
//     _unordered = unordered;
//     _saveAllGraphs = saveAllGraphs;
//     _saveAllEdges = saveAllEdges;
//     _outFolder = outFolder;
// }

// /**
//  * 执行子图搜索并保存结果。
//  * @param hFname 查询图文件名。
//  * @param outGraphFname 输出图文件名。
//  * @param nodeCountFname 节点计数文件名。
//  * @param saveQueryGraph 是否保存查询图。
//  * @return 匹配子图的数量。
//  */
// int SearchCERT::search(const string &hFname, const string &outGraphFname, const string &nodeCountFname, bool saveQueryGraph)
// {
//     cout << "加载 GDF 查询图: " << hFname << endl;
//     CertGraph h = FileIO::loadCertGDF(hFname); // 加载查询图
//     h.disp(); // 显示查询图信息

//     // 确定数据图中边的时间范围
//     time_t start = _dataGraph->edges().front().time();
//     time_t end = _dataGraph->edges().back().time();

//     // 定义 CERT 图的匹配标准
//     MatchCriteria_CERT criteria;

//     cout << "基于搜索条件过滤图" << endl;
//     _g2 = CertGraph(_dataGraph->windowDuration()); // 清空旧图
//     GraphFilter::filter(*_dataGraph, h, criteria, _g2); // 过滤数据图
//     cout << "新图大小: " << _g2.numNodes() << " 个节点, " << _g2.numEdges() << " 条边" << endl;

//     clock_t t1 = clock(); // 开始计时

//     GraphSearch search; // 创建图搜索对象
//     if (_unordered)
//     {
//         cout << "搜索 CERT 图以查找匹配的无序子图" << endl;
//         _subgraphs = search.findAllSubgraphs(_g2, h, criteria, _limit); // 查找无序子图
//     }
//     else
//     {
//         cout << "搜索 CERT 图以查找匹配的有序子图" << endl;
//         _subgraphs = search.findOrderedSubgraphs(_g2, h, criteria, _limit, _delta); // 查找有序子图
//     }

//     clock_t t2 = clock(); // 结束计时
//     cout << endl;
//     cout << ">>>>>>> 图搜索耗时 = " << (double)(t2 - t1) / CLOCKS_PER_SEC << " 秒" << endl;
//     if (_subgraphs.empty())
//         cout << ">>>>>>> 未找到匹配的子图！" << endl;
//     else
//         cout << ">>>>>>> 找到的子图数量: " << _subgraphs.size() << endl;
//     cout << endl;

//     // 如果需要，将结果输出到标准输出
//     if (_stdOut)
//     {
//         for (int sgi = 0; sgi < _subgraphs.size(); sgi++)
//         {
//             cout << "子图 #" << sgi + 1 << endl;
//             cout << "   结果:" << endl;
//             const vector<int> &edges = _subgraphs[sgi].edges();
//             for (int edgeIndex : edges)
//             {
//                 cout << "      "; // 添加缩进以便显示
//                 _g2.disp(edgeIndex);
//             }
//         }
//     }

//     // 如果需要，保存所有匹配子图
//     if (_saveAllGraphs)
//     {
//         cout << "将结果保存到以下文件夹: " << _outFolder << endl;
//         try
//         {
//             for (int sgi = 0; sgi < _subgraphs.size(); sgi++)
//             {
//                 CertGraph g3 = _g2.createSubGraph(_subgraphs[sgi].edges());

//                 stringstream ss;
//                 ss << _outFolder << "results" << sgi + 1 << ".gdf";
//                 FileIO::saveCertGDF(g3, ss.str());
//             }
//         }
//         catch (exception &e)
//         {
//             cerr << "保存结果时遇到问题。请确保输出文件夹存在并具有写权限。" << endl;
//             cerr << e.what() << endl;
//         }
//         catch (const char *msg)
//         {
//             cerr << "保存结果时遇到问题。请确保输出文件夹存在并具有写权限。" << endl;
//             cerr << msg << endl;
//         }
//         catch (...)
//         {
//             cerr << "保存结果时遇到问题。请确保输出文件夹存在并具有写权限。" << endl;
//         }
//     }

//     // 如果需要，计算时间片节点计数并保存
//     if (!nodeCountFname.empty())
//     {
//         cout << "搜索时间片中的子图匹配" << endl;
//         const int numSlices = 5;
//         unordered_map<string, vector<int>> timeCounts = calcTemporalCounts(_g2, h, criteria, start, end, numSlices);
//         cout << "保存节点计数到 " << nodeCountFname << endl;
//         FileIO::saveNodeCount(_g2, _subgraphs, h, timeCounts, numSlices, start, end, nodeCountFname);
//     }

//     // 如果需要，保存单个聚合图
//     if (!outGraphFname.empty())
//     {
//         cout << "从所有匹配子图创建单个图。" << endl;
//         vector<int> edgeCounts;
//         bool ignoreDir = true;
//         CertGraph g3(0);
//         if (_saveAllEdges)
//         {
//             vector<int> tempNodeCounts; // 临时节点计数，实际未使用
//             g3 = _g2.createSubGraph(_subgraphs, edgeCounts, tempNodeCounts);
//         }
//         else
//         {
//             g3 = _g2.createAggregateSubGraph(ignoreDir, _subgraphs, edgeCounts);
//         }
//         vector<int> nodeCounts(g3.nodes().size(), 1); // 每个节点计数为 1
//         map<string, vector<int>> extraEdgeValues, extraNodeValues;
//         extraEdgeValues["count"] = edgeCounts;
//         extraNodeValues["count"] = nodeCounts;
//         cout << "将单个图保存到文件: " << outGraphFname << endl;
//         FileIO::saveCertGDF(g3, outGraphFname, extraEdgeValues, extraNodeValues);
//     }

//     // 如果需要，保存查询图
//     if (saveQueryGraph)
//     {
//         string outQueryFname = _outFolder + FileIO::getFname(hFname);
//         cout << "保存带节点/边计数的查询图副本到 " << outQueryFname << endl;
//         FileIO::saveQueryGraph(h, _subgraphs, outQueryFname);
//     }
//     cout << endl;

//     return _subgraphs.size(); // 返回匹配子图的数量
// }

// /**
//  * 计算时间片中的节点计数。
//  * @param g 数据图。
//  * @param h 查询图。
//  * @param criteria 匹配标准。
//  * @param start 时间范围的起始时间。
//  * @param end 时间范围的结束时间。
//  * @param numSlices 时间片的数量。
//  * @return 节点计数的映射表。
//  */
// unordered_map<string, vector<int>> SearchCERT::calcTemporalCounts(const CertGraph &g, const CertGraph &h, const MatchCriteria_CERT &criteria, time_t start, time_t end, int numSlices)
// {
//     unordered_map<string, vector<int>> results;

//     time_t sliceDur = (end - start) / numSlices; // 每个时间片的持续时间
//     for (int i = 0; i < numSlices; i++)
//     {
//         time_t t0 = start + i * sliceDur;
//         time_t t1 = t0 + sliceDur;
//         unordered_map<string, int> sliceCounts = calcTemporalSlice(g, h, criteria, t0, t1);
//         for (auto pair : sliceCounts)
//         {
//             const string &name = pair.first;
//             int count = pair.second;
//             vector<int> &counts = results[name];
//             if (counts.size() < numSlices)
//                 counts.resize(numSlices, 0);
//             counts[i] = count;
//         }
//     }
//     return results;
// }

// /**
//  * 计算单个时间片中的节点计数。
//  * @param g 数据图。
//  * @param h 查询图。
//  * @param criteria 匹配标准。
//  * @param start 时间片的起始时间。
//  * @param end 时间片的结束时间。
//  * @return 节点计数的映射表。
//  */
// unordered_map<string, int> SearchCERT::calcTemporalSlice(const CertGraph &g, const CertGraph &h, const MatchCriteria_CERT &criteria, time_t start, time_t end)
// {
//     cout << "基于时间范围切片过滤图" << endl;
//     CertGraph g2(g.windowDuration());
//     GraphFilter::filter(g, start, end, g2);
//     cout << "新图大小: " << g2.numNodes() << " 个节点, " << g2.numEdges() << " 条边" << endl;

//     GraphSearch search;
//     vector<GraphMatch> subgraphs;
//     if (_unordered)
//     {
//         cout << "搜索时间片以查找匹配的无序子图" << endl;
//         subgraphs = search.findAllSubgraphs(g2, h, criteria, _limit);
//     }
//     else
//     {
//         cout << "搜索时间片以查找匹配的有序子图" << endl;
//         subgraphs = search.findOrderedSubgraphs(g2, h, criteria, _limit, _delta);
//     }

//     cout << subgraphs.size() << " 个匹配子图被找到" << endl;

//     unordered_map<string, int> results;
//     for (const GraphMatch &sg : subgraphs)
//     {
//         for (int u : sg.nodes())
//         {
//             const string &name = g2.getLabel(u);
//             if (results.find(name) == results.end())
//                 results[name] = 0;
//             results[name]++;
//         }
//     }

//     cout << results.size() << " 个匹配节点" << endl;

//     return results;
// }

// /**
//  * 执行 motif 测试。
//  * @param motifFnames motif 文件名列表。
//  * @param answersFname 答案文件名。
//  */
// void SearchCERT::motifTest(const vector<string> &motifFnames, const string &answersFname)
// {
//     // 从文件加载答案
//     vector<unordered_set<string>> answers;
//     FastReader csv(answersFname);
//     while (csv.good())
//     {
//         char **vars = csv.next();
//         int nc = csv.rowSize();
//         if (nc == 0 || (nc == 1 && vars[0][0] == '\0'))
//             continue;
//         unordered_set<string> nodeNames;
//         for (int i = 0; i < nc; i++)
//             nodeNames.insert(vars[i]);
//         answers.push_back(nodeNames);
//     }
//     csv.close();

//     // 执行 motif 测试
//     motifTest(motifFnames, answers);
// }

// /**
//  * 执行 motif 测试。
//  * @param motifFnames motif 文件名列表。
//  * @param answers 答案集合。
//  */
// void SearchCERT::motifTest(const vector<string> &motifFnames, const vector<unordered_set<string>> &answers)
// {
//     // 根据日期范围和 delta 创建输出文件名
//     stringstream ss;
//     ss << "test_" << Tools::getDate(_dataGraph->windowStart(), true) << "_";
//     ss << Tools::getDate(_dataGraph->windowEnd(), true) << "_delta" << _delta << ".csv";
//     string outFname = ss.str();

//     cout << "保存实验结果到: " << outFname << endl;
//     ofstream ofs(outFname);
//     ofs << "Motif,Delta(hr),Start Date,End Date,# Subgraphs,# Users,# PCs";
//     for (int uc = 0; uc < answers.size(); uc++)
//     {
//         ofs << ",Use Case " << uc + 1;
//     }
//     ofs << endl;

//     for (int hi = 0; hi < motifFnames.size(); hi++)
//     {
//         const string &hFname = motifFnames[hi];
//         cout << "加载 GDF 查询图: " << hFname << endl;
//         CertGraph h = FileIO::loadCertGDF(hFname);
//         h.disp();

//         // 确定边的时间范围
//         time_t start = _dataGraph->edges().front().time();
//         time_t end = _dataGraph->edges().back().time();

//         // 定义 CERT 图的匹配标准
//         MatchCriteria_CERT criteria;

//         cout << "基于搜索条件过滤图" << endl;
//         CertGraph g2(_dataGraph->windowDuration());
//         GraphFilter::filter(*_dataGraph, h, criteria, g2);
//         cout << "新图大小: " << g2.numNodes() << " 个节点, " << g2.numEdges() << " 条边" << endl;

//         clock_t t1 = clock();
//         GraphSearch search;
//         vector<GraphMatch> subgraphs;
//         if (_unordered)
//         {
//             cout << "搜索 CERT 图以查找匹配的无序子图" << endl;
//             subgraphs = search.findAllSubgraphs(g2, h, criteria, _limit);
//         }
//         else
//         {
//             cout << "搜索 CERT 图以查找匹配的有序子图" << endl;
//             subgraphs = search.findOrderedSubgraphs(g2, h, criteria, _limit, _delta);
//         }
//         clock_t t2 = clock();

//         cout << endl;
//         cout << ">>>>>>> 图搜索耗时 = " << (double)(t2 - t1) / CLOCKS_PER_SEC << " 秒" << endl;
//         if (subgraphs.empty())
//             cout << ">>>>>>> 未找到匹配的子图！" << endl;
//         else
//             cout << ">>>>>>> 找到的子图数量: " << subgraphs.size() << endl;
//         cout << endl;

//         // 统计用户和 PC 的数量
//         cout << "统计类型为 " << CertGraph::USER_NODE << " 的节点数量" << endl;
//         unordered_map<int, int> userCounts = Tools::count(CertGraph::USER_NODE, subgraphs, *_dataGraph);
//         cout << "找到的数量 = " << userCounts.size() << endl;
//         cout << "统计类型为 " << CertGraph::PC_NODE << " 的节点数量" << endl;
//         unordered_map<int, int> pcCounts = Tools::count(CertGraph::PC_NODE, subgraphs, *_dataGraph);
//         cout << "找到的数量 = " << pcCounts.size() << endl;

//         int hours = (int)(_delta / 3600);
//         ofs << FileIO::getFname(hFname) << "," << hours << "," << Tools::getDate(start) << "," << Tools::getDate(end);
//         ofs << "," << subgraphs.size() << "," << userCounts.size() << "," << pcCounts.size();

//         cout << "确定答案的排名" << endl;
//         for (const unordered_set<string> &nodeLabels : answers)
//         {
//             int bestRanking = INT_MAX;
//             for (const string &label : nodeLabels)
//             {
//                 if (!_dataGraph->hasLabeledNode(label))
//                     continue;

//                 int v = _dataGraph->getIndex(label);
//                 int ranking = Tools::findRanking(v, userCounts);
//                 if (!ranking)
//                     ranking = Tools::findRanking(v, pcCounts);
//                 if (ranking > 0 && ranking < bestRanking)
//                     bestRanking = ranking;
//             }
//             if (bestRanking == INT_MAX)
//                 ofs << ",NA";
//             else
//                 ofs << "," << bestRanking;
//         }
//         ofs << endl;
//     }

//     cout << "完成结果文件" << endl;
//     ofs.close();
// }

// /**
//  * 查找子图中的边。
//  * @param nodeIDs 节点 ID 列表。
//  * @return 边的源节点和目标节点的名称对。
//  */
// vector<pair<string, string>> SearchCERT::findSubgraphEdges(const vector<string> &nodeIDs) const
// {
//     // 获取节点索引
//     vector<int> nodes;
//     for (const string &id : nodeIDs)
//     {
//         if (_g2.hasLabeledNode(id))
//         {
//             int index = _g2.getIndex(id);
//             nodes.push_back(index);
//         }
//     }

//     // 查找包含节点的子图，并添加其边
//     unordered_set<int> edges;
//     for (const GraphMatch &gm : _subgraphs)
//     {
//         for (int u : nodes)
//         {
//             if (gm.hasNode(u))
//             {
//                 for (int e : gm.edges())
//                 {
//                     const Edge &edge = _g2.edges()[e];
//                     if (edge.index() != e)
//                         throw "边索引不匹配！";
//                 }
//                 edges.insert(gm.edges().begin(), gm.edges().end());
//                 break;
//             }
//         }
//     }

//     // 将边转换为名称对
//     vector<pair<string, string>> edgeVec;
//     for (int e : edges)
//     {
//         const Edge &edge = _g2.edges()[e];
//         const string &source = _g2.getLabel(edge.source());
//         const string &dest = _g2.getLabel(edge.dest());
//         pair<string, string> nodePair(source, dest);
//         edgeVec.push_back(nodePair);
//     }
//     return edgeVec;
// }