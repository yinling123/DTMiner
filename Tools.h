#include <string>
#include <unordered_map>
#include <vector>
#include "GraphMatch.h"
#include "CertGraph.h"

/**
 * 通用工具类，提供多种实用功能。
 */
class Tools
{
public:
    /**
     * 将给定字符串按指定分隔符拆分为多个子字符串。
     * @param str 输入的字符串。
     * @param delim 分隔符字符。
     * @return 返回包含拆分子字符串的向量。
     */
    static std::vector<std::string> split(const std::string &str, char delim);

    /**
     * 以最合适的单位显示持续时间（如秒、分钟、小时、天等）。
     * 输出格式：Xw Yd Zh Am Bs（周、天、小时、分钟、秒）。
     * @param duration 持续时间（秒）。
     */
    static void dispDuration(int duration);

    /**
     * 将时间戳转换为日期字符串，格式为 MM/DD/YYYY 或 MM-DD-YYYY。
     * @param date 时间戳（自 Unix 纪元以来的秒数）。
     * @param useHyphen 是否使用连字符（-）作为分隔符。默认为 false（使用斜杠 /）。
     * @return 返回格式化的日期字符串。
     */
    static std::string getDate(time_t date, bool useHyphen = false);

    /**
     * 统计匹配子图中特定类型节点的出现次数。
     * @param type 要统计的节点类型。
     * @param subgraphs 包含匹配子图的向量。
     * @param g 子图所属的图对象。
     * @return 返回节点索引到出现次数的映射表。
     */
    static std::unordered_map<int, int> count(std::string type, const std::vector<GraphMatch> &subgraphs, const CertGraph &g);

    /**
     * 根据节点计数计算给定节点的排名。
     * 排名规则：1 表示计数值最多的节点，依次递减。
     * 如果无法在计数表中找到节点，则返回 0。
     * @param v 要查找排名的节点索引。
     * @param nodeCounts 节点索引到计数值的映射表。
     * @return 返回节点的排名（从 1 开始）。如果节点不存在于计数表中，则返回 0。
     */
    static int findRanking(int v, const std::unordered_map<int, int> &nodeCounts);
};