#ifndef STATS__H
#define STATS__H

#include <vector>

/**
 * 计算并存储与搜索结果相关的统计信息。
 */
class Stats
{
public:
    /**
     * 构造函数，用于根据给定的整数向量计算均值和标准差。
     * @param counts 包含整数的向量，表示需要统计的数据。
     */
    Stats(const std::vector<int> &counts);

    /**
     * 获取计算得到的均值。
     * @return 返回均值（double 类型）。
     */
    double mean() const { return _mean; }

    /**
     * 获取计算得到的标准差。
     * @return 返回标准差（double 类型）。
     */
    double stdDev() const { return _stdDev; }

private:
    double _mean;  // 存储计算得到的均值
    double _stdDev; // 存储计算得到的标准差
};

#endif