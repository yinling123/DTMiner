// /* 
//  * File:   SearchCERT.h
//  * Author: D3M430
//  *
//  * Created on May 10, 2017, 11:36 AM
//  */

// #ifndef SEARCHCERT_H
// #define	SEARCHCERT_H

// #include "CertGraph.h"
// #include "MatchCriteria_CERT.h"
// #include <string>
// #include <unordered_map>
// #include <unordered_set>
// #include <utility>
// #include <vector>

// /**
//  * 该类提供了在 CERT 数据集上执行子图搜索的功能。
//  */
// class SearchCERT
// {
// public:
//     /**
//      * 构造函数初始化搜索功能，并从命令行参数中获取选项。
//      * @param dataGraph 要搜索的大图。
//      * @param delta 匹配边之间允许的最大时间差（秒）。
//      * @param limit 要查找的子图的最大数量。（设置为 INT_MAX 表示不限制数量）。
//      * @param stdOut 如果为 true，则将匹配信息输出到标准输出控制台。
//      * @param unordered 如果为 true，则执行无序子图匹配，忽略边的顺序。
//      *                  如果为 false，则要求边的顺序与查询图一致。
//      * @param saveAllGraphs 如果为 true，则将每个匹配子图保存为单独的 GDF 文件（不推荐）。
//      * @param saveAllEdges 如果为 true，则保存每条单独的边，而不是合并它们。
//      * @param outFolder 保存输出文件的位置。
//      */
//     SearchCERT(const CertGraph &dataGraph, int delta, int limit, bool stdOut, bool unordered, bool saveAllGraphs, bool saveAllEdges, const std::string &outFolder);

//     /**
//      * 对存储在给定文件名中的查询图执行子图搜索，并保存必要的结果。
//      * @param hFname 存储查询图的 GDF 文件名。
//      * @param outGraphFname 保存合并结果图的 GDF 文件名。
//      * @param nodeCountFname 保存节点计数的 CSV 文件名（每个节点被找到的匹配子图数量）。
//      * @param saveQueryGraph 如果为 true，则还保存查询图的副本，并附加节点匹配数量的信息。
//      * @return 找到的匹配子图数量。
//      */
//     int search(const std::string &hFname, const std::string &outGraphFname, const std::string &nodeCountFname, bool saveQueryGraph);    

//     /**
//      * 计算给定时间片内每个节点被找到的匹配子图数量。
//      * @param g 要搜索的图。
//      * @param h 要查找的图。
//      * @param criteria 边/节点属性的匹配标准。
//      * @param start 搜索的起始时间。
//      * @param end 搜索的结束时间。
//      * @param numSlices 将时间范围分割的时间片数量。
//      * @return 节点名称到每个时间片计数列表的映射。
//      */
//     std::unordered_map<std::string, std::vector<int>> calcTemporalCounts(const CertGraph &g, const CertGraph &h, const MatchCriteria_CERT &criteria, time_t start, time_t end, int numSlices);

//     /**
//      * 计算特定时间片内每个节点被找到的匹配子图数量。
//      * @param g 要搜索的图。
//      * @param h 要查找的图。
//      * @param criteria 边/节点属性的匹配标准。
//      * @param start 搜索的起始时间。
//      * @param end 搜索的结束时间。
//      * @return 节点名称到该时间片内匹配子图数量的映射。
//      */
//     std::unordered_map<std::string, int> calcTemporalSlice(const CertGraph &g, const CertGraph &h, const MatchCriteria_CERT &criteria, time_t start, time_t end);

//     /**
//      * 使用给定的 motif/查询图和答案执行 motif 测试。
//      */
//     void motifTest(const std::vector<std::string> &motifFnames, const std::string &answersFname);

//     /**
//      * 使用给定的 motif/查询图和答案执行 motif 测试。
//      */
//     void motifTest(const std::vector<std::string> &motifFnames, const std::vector<std::unordered_set<std::string>> &answers);

//     /**
//      * 获取最近匹配的子图引用。
//      */
//     const std::vector<GraphMatch> &subgraphs() const { return _subgraphs; }

//     /**
//      * 查找给定节点所在的子图，并返回这些子图的所有边（无论节点是否在边上）。
//      */
//     std::vector<std::pair<std::string,std::string>> findSubgraphEdges(const std::vector<std::string> &nodeIDs) const;

//     /** 获取实际用于搜索的过滤图引用 */
//     //const CertGraph &filteredGraph() const { return _g2; }

// private:
//     const CertGraph *_dataGraph; // 原始数据图的指针
//     CertGraph _g2;               // 过滤后的图
//     std::vector<GraphMatch> _subgraphs; // 最近匹配的子图集合
//     int _delta, _limit;          // 时间窗口增量和子图数量限制
//     bool _stdOut, _unordered, _saveAllGraphs, _saveAllEdges; // 输出选项和保存选项
//     std::string _outFolder;      // 输出文件夹路径
// };

// #endif	/* SEARCHCERT_H */