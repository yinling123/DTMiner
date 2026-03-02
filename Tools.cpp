#include "Tools.h"
#include <iostream>
#include <sstream>
#include <time.h>

using namespace std;

/**
 * 将字符串按指定分隔符拆分为多个子字符串。
 * @param str 输入的字符串。
 * @param delim 分隔符字符。
 * @return 返回包含拆分子字符串的向量。
 */
vector<string> Tools::split(const std::string &str, char delim)
{
    vector<string> vars; // 存储拆分结果的向量
    if (str.empty()) // 如果输入字符串为空，直接返回空向量
        return vars;

    size_t pos = 0; // 当前搜索位置
    while (pos != string::npos) // 循环直到找不到分隔符
    {
        size_t pos2 = str.find(delim, pos); // 查找下一个分隔符位置
        if (pos2 == string::npos) // 如果未找到分隔符
        {
            vars.push_back(str.substr(pos)); // 添加剩余部分到结果
            break;
        }
        else
        {
            vars.push_back(str.substr(pos, pos2 - pos)); // 添加当前部分到结果
            pos = pos2 + 1; // 更新搜索位置
        }
    }
    return vars; // 返回拆分结果
}

/**
 * 格式化并输出持续时间（以秒为单位）。
 * 输出格式：Xw Yd Zh Am Bs（周、天、小时、分钟、秒）。
 * @param duration 持续时间（秒）。
 */
void Tools::dispDuration(int duration)
{
    int minute = 60; // 一分钟的秒数
    int hour = 60 * minute; // 一小时的秒数
    int day = 24 * hour; // 一天的秒数
    int week = 7 * day; // 一周的秒数

    if (duration >= week) // 计算周数
    {
        cout << duration / week << "w "; // 输出周数
        duration -= (duration / week) * week; // 减去已计算的部分
    }
    if (duration >= day) // 计算天数
    {
        cout << duration / day << "d "; // 输出天数
        duration -= (duration / day) * day; // 减去已计算的部分
    }
    if (duration >= hour) // 计算小时数
    {
        cout << duration / hour << "h "; // 输出小时数
        duration -= (duration / hour) * hour; // 减去已计算的部分
    }
    if (duration >= minute) // 计算分钟数
    {
        cout << duration / minute << "m "; // 输出分钟数
        duration -= (duration / minute) * minute; // 减去已计算的部分
    }
    if (duration > 0) // 剩余秒数
    {
        cout << duration << "s "; // 输出秒数
    }
}

/**
 * 将时间戳转换为日期字符串。
 * @param date 时间戳（time_t 类型）。
 * @param useHyphen 是否使用连字符（-）作为分隔符。如果为 false，则使用斜杠（/）。
 * @return 返回格式化的日期字符串（MM/DD/YYYY 或 MM-DD-YYYY）。
 */
string Tools::getDate(time_t date, bool useHyphen)
{
    struct tm *timeStruct; // 时间结构体指针
    timeStruct = gmtime(&date); // 将时间戳转换为 UTC 时间

    int month = timeStruct->tm_mon + 1; // 月份从 0 开始，需加 1
    int day = timeStruct->tm_mday; // 日
    int year = timeStruct->tm_year + 1900; // 年份从 1900 开始，需加 1900

    char sep = '/'; // 默认分隔符为斜杠
    if (useHyphen)
        sep = '-'; // 如果需要，改为连字符

    stringstream ss; // 字符串流
    ss << month << sep << day << sep << year; // 组装日期字符串
    return ss.str(); // 返回结果
}

/*string Tools::getFname(const string &path)
{
    int pos = path.rfind('/'); // 查找最后一个斜杠位置
    if (pos == string::npos)
        pos = path.rfind('\\'); // 如果未找到斜杠，查找反斜杠
    if (pos == string::npos)
        return path; // 如果未找到路径分隔符，返回原始路径
    return path.substr(pos + 1); // 返回文件名部分
}*/

/**
 * 统计子图中特定类型节点的出现次数。
 * @param type 节点类型。
 * @param subgraphs 包含匹配子图的向量。
 * @param g 图对象（CertGraph 类型）。
 * @return 返回节点 ID 到出现次数的映射表。
 */
unordered_map<int, int> Tools::count(string type, const vector<GraphMatch> &subgraphs, const CertGraph &g)
{
    unordered_map<int, int> nodeCounts; // 节点计数映射表
    for (const GraphMatch &sg : subgraphs) // 遍历每个子图
    {
        for (int v : sg.nodes()) // 遍历子图中的每个节点
        {
            if (g.getNodeType(v) == type) // 如果节点类型匹配
            {
                if (nodeCounts.find(v) == nodeCounts.end()) // 如果节点尚未记录
                    nodeCounts[v] = 1; // 初始化计数为 1
                else
                    nodeCounts[v]++; // 否则计数加 1
            }
        }
    }
    return nodeCounts; // 返回统计结果
}

/**
 * 计算给定节点在节点计数表中的排名。
 * 排名规则：计数值越大，排名越靠前。
 * @param v 节点 ID。
 * @param nodeCounts 节点计数映射表。
 * @return 返回节点的排名（从 1 开始）。如果节点不存在于计数表中，则返回 0。
 */
int Tools::findRanking(int v, const unordered_map<int, int> &nodeCounts)
{
    auto iter = nodeCounts.find(v); // 查找节点
    if (iter == nodeCounts.end()) // 如果节点不存在
        return 0; // 返回 0

    int vCount = iter->second; // 获取节点的计数值
    int ranking = 1; // 默认排名为 1（最高）
    // 遍历所有节点计数，计算排名
    for (auto pair : nodeCounts)
    {
        int count = pair.second; // 获取当前节点的计数值
        if (count > vCount) // 如果当前节点计数更高
            ranking++; // 排名加 1
    }
    return ranking; // 返回最终排名
}