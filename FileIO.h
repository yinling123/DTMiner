/**
 * 文件:   FileIO.h
 * 作者: D3M430
 *
 * 创建时间: 2017年1月19日 上午8:06
 */

#ifndef FILEIO_H
#define FILEIO_H

#include "DataGraph.h"
#include "Graph.h"
#include "LabeledWeightedGraph.h"
#include "CertGraph.h"
#include "GraphMatch.h"
#include "Roles.h"
#include "SearchConfig.h"
#include <time.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

/**
 * @class FileIO
 * @brief 用于处理图数据文件输入/输出操作的工具类。
 * 
 * 该类提供了静态方法，用于加载和保存各种格式的图数据，以及文件操作和元数据管理的实用功能。
 */
class FileIO
{
public:
    /**
     * @brief 加载基本图，每行包含源和目标索引（索引从0开始）。
     * @param fname 要加载的文件名。
     * @return 包含加载边的 Graph 对象。
     */
    static Graph loadEdges(const std::string &fname);

    /**
     * @brief 将 SNAP 时间网络数据加载到 CertGraph 中。
     * @param fname 要加载的文件名。
     * @param g 要填充数据的 CertGraph 对象。
     */
    static void loadSNAP(const std::string &fname, CertGraph &g);

    /**
     * @brief 加载基本图，每行包含源和目标标签。
     * @param fname 要加载的文件名。
     * @return 包含加载边的 Graph 对象。
     */
    static Graph loadLabeledEdges(const std::string &fname);

    /**
     * @brief 加载 Pajek 格式的加权带标签图。
     * @param fname 要加载的文件名。
     * @return 包含加载图的 LabeledWeightedGraph 对象。
     */
    static LabeledWeightedGraph loadPajek(const std::string &fname);

    /**
     * @brief 从 GDF 文件格式加载带有元数据的 CERT 图，并将其追加到现有的 CertGraph 中。
     * @param fname 要加载的文件名。
     * @param g 要填充数据的 CertGraph 对象。
     */
    static void loadCertGDF(const std::string &fname, CertGraph &g);

    /**
     * @brief 从 GDF 文件格式加载带有元数据的 CERT 图。
     * @param fname 要加载的文件名。
     * @return 包含加载图的 CertGraph 对象。
     */
    static CertGraph loadCertGDF(const std::string &fname);

    /**
     * @brief 从指定文件夹加载所有 *.gdf 文件作为单个 CERT 图。
     * @param folder 包含 GDF 文件的文件夹路径。
     * @return 包含合并图的 CertGraph 对象。
     */
    static CertGraph loadCertGDFs(const std::string &folder);

    /**
     * @brief 以 GDF 文件格式保存带有元数据的 CERT 图。
     * @param g 要保存的 CertGraph 对象。
     * @param fname 要保存到的文件名。
     */
    static void saveCertGDF(const CertGraph &g, const std::string &fname);

    /**
     * @brief 以 GDF 文件格式保存带有元数据的 CERT 图，并附加额外的边和节点值。
     * @param g 要保存的 CertGraph 对象。
     * @param fname 要保存到的文件名。
     * @param extraEdgeValues 额外的边值。
     * @param extraNodeValues 额外的节点值。
     */
    static void saveCertGDF(const CertGraph &g, const std::string &fname,
        const std::map<std::string,std::vector<int>> &extraEdgeValues,
        const std::map<std::string,std::vector<int>> &extraNodeValues);

    /**
     * @brief 从 GDF 文件格式加载通用属性图。
     * @param fname 要加载的文件名。
     * @return 包含加载图的 DataGraph 对象。
     */
    static DataGraph loadGenericGDF(const std::string &fname);

    /**
     * @brief 以标准 GDF 文件格式保存属性图。
     * @param g 要保存的图。
     * @param fname 要保存到的 GDF 文件名。
     */
    static void saveGenericGDF(const DataGraph &g, const std::string &fname);

    /**
     * @brief 保存一个 CSV 文件，记录每个节点在匹配子图中的出现次数（仅保存计数 > 0 的节点）。
     * @param g 包含子图的图。
     * @param subgraphs 每个子图的边索引列表。
     * @param h 查询图。
     * @param timeCounts 节点在不同时间片中匹配到的子图数量。
     * @param numTimeSlices 时间片的数量。
     * @param startTime 日期范围的起始时间。
     * @param endTime 日期范围的结束时间。
     * @param fname 要保存结果的 CSV 文件名。
     */
    static void saveNodeCount(const LabeledWeightedGraph &g, const std::vector<GraphMatch> &subgraphs, 
			      const LabeledWeightedGraph &h, const std::unordered_map<std::string,std::vector<int>> &timeCounts, 
			      int numTimeSlices, time_t startTime, time_t endTime, const std::string &fname);

    /**
     * @brief 基于原始查询图保存 GDF 图，权重基于匹配到的节点和边的数量。
     * @param query 要保存的查询图。
     * @param subgraphs 匹配的子图列表。
     * @param fname 要保存结果的 GDF 文件名。
     */
    static void saveQueryGraph(const CertGraph &query, const std::vector<GraphMatch> &subgraphs, const std::string &fname);

    /**
     * @brief 保存描述大图当前状态的 JSON 文件。
     * @param g 要保存信息的大图。
     * @param rangeStart 数据范围的起始时间。
     * @param rangeEnd 数据范围的结束时间。
     * @param queryCounts 每个查询图找到的匹配子图数量。
     * @param config 当前的搜索和流式配置。
     * @param selEdges 要在 GraphInfo 文件中标记的边。
     * @param jsonFname 要保存信息的 JSON 文件名。
     */
    static void saveGraphInfo(const CertGraph &g, time_t rangeStart, time_t rangeEnd, const std::unordered_map<std::string,int> &queryCounts, const SearchConfig &config, const std::vector<std::pair<std::string,std::string>> &selEdges, const std::string &jsonFname);

    /**
     * @brief 从 JSON 文件加载配置设置。
     * @param jsonFname 要加载的文件名。
     * @param config 要填充的 SearchConfig 对象。
     */
    static void loadConfig(const std::string &jsonFname, SearchConfig &config);

    /**
     * @brief 返回指定文件夹中的所有文件名。
     * @param folder 文件夹路径。
     * @return 文件名列表。
     */
    static std::vector<std::string> getFileNames(const std::string &folder);

    /**
     * @brief 返回指定文件夹中与给定扩展名匹配的所有文件名。
     * @param folder 文件夹路径。
     * @param ext 要过滤的文件扩展名。
     * @return 文件名列表。
     */
    static std::vector<std::string> getFileNames(const std::string &folder, const std::string &ext);

    /**
     * @brief 返回文件的最后修改日期（自纪元以来的秒数）。
     * @param fname 文件名。
     * @return 最后修改日期。
     */
    static time_t getFileDate(const std::string &fname);

    /**
     * @brief 提取路径中的文件名部分。
     * @param path 完整文件路径。
     * @return 文件名。
     */
    static std::string getFname(const std::string &path);

    /**
     * @brief 检查文件是否存在并可打开。
     * @param fname 文件名。
     * @return 如果文件存在则返回 true，否则返回 false。
     */
    static bool fileExists(const std::string &fname);

    /**
     * @brief 根据查询文件名和输出文件夹生成单个输出图的文件名。
     * @param fname 查询文件名。
     * @param outFolder 输出文件夹。
     * @return 生成的文件名。
     */
    static std::string makeOutGraphFname(const std::string &fname, const std::string &outFolder);

    /**
     * @brief 根据查询文件名和输出文件夹生成节点计数 CSV 文件的文件名。
     * @param fname 查询文件名。
     * @param outFolder 输出文件夹。
     * @return 生成的文件名。
     */
    static std::string makeNodeCountFname(const std::string &fname, const std::string &outFolder);

private:
    /**
     * @brief 根据限制字符串向 CertGraph 添加节点限制。
     * @param nodeID 节点 ID。
     * @param restrictions 限制字符串。
     * @param g 要修改的 CertGraph 对象。
     */
    static void addNodeRestrictions(const std::string &nodeID, const std::string &restrictions, CertGraph &g);

    /**
     * @brief 从变量数组中提取属性名称。
     * @param vars 变量数组。
     * @param numVars 变量数量。
     * @return 属性名称列表。
     */
    static std::vector<std::string> getAttributeNames(char **vars, int numVars);

    /**
     * @brief 根据变量数组生成 AttributesDef 对象。
     * @param isNodeData 表示数据是否为节点。
     * @param vars 变量数组。
     * @param numVars 变量数量。
     * @return AttributesDef 对象。
     */
    static AttributesDef getAttributesDef(bool isNodeData, char **vars, int numVars);

    /**
     * @brief 将属性头部分保存到文件中。
     * @param def AttributesDef 对象。
     * @param ofs 输出文件流。
     */
    static void saveAttributesHeader(const AttributesDef &def, std::ofstream &ofs);

    /**
     * @brief 根据定义和变量向 Attributes 对象添加属性值。
     * @param def AttributesDef 对象。
     * @param vars 变量数组。
     * @param a 要填充的 Attributes 对象。
     */
    static void addAttributeValues(const AttributesDef &def, char **vars, Attributes &a);

    // 用于加载/保存 JSON 文件的常量
    static const std::string STREAM_SECTION; ///< 流设置的节名称。
    static const std::string IS_STREAMING;   ///< 流启用标志的键。
    static const std::string STREAM_DUR;     ///< 流持续时间的键。
    static const std::string STREAM_DELAY;   ///< 流延迟的键。
    static const std::string STREAM_WIN;     ///< 流窗口大小的键。
    static const std::string DELTA;          ///< Delta 值的键。
    static const std::string NUM_NODES;      ///< 节点数量的键。
    static const std::string NUM_LINKS;      ///< 链接数量的键。
    static const std::string START_DATE;     ///< 开始日期的键。
    static const std::string END_DATE;       ///< 结束日期的键。
    static const std::string QUERIES_SECTION;///< 查询的节名称。
    static const std::string SEL_SUBGRAPHS_SECTION; ///< 选定子图的节名称。
    static const std::string FIND_SUBGRAPHS_SECTION; ///< 查找子图的节名称。
    static const std::string QUERY;          ///< 查询的键。
    static const std::string NODES;          ///< 节点的键。
    static const std::string LINKS;          ///< 链接的键。
};

#endif /* FILEIO_H */