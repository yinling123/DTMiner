#include "Stats.h"
#include <math.h>

using namespace std;

/**
 * 构造函数 Stats，用于计算给定整数数组的均值和标准差。
 * @param counts 包含整数的向量，表示需要统计的数据。
 */
Stats::Stats(const std::vector<int> &counts)
{
    int n = counts.size(); // 获取数据的数量

    // 计算均值
    double sum = 0.0; // 初始化总和为 0
    for(int count : counts) // 遍历所有数据
        sum += count; // 累加每个数据值
    _mean = sum / n; // 均值 = 总和 / 数据数量

    // 计算标准差
    sum = 0.0; // 重置总和为 0
    for(int count : counts) // 再次遍历所有数据
    {
        double diff = (double)count - _mean; // 计算当前值与均值的差值
        sum += diff * diff; // 累加差值的平方
    }
    _stdDev = sqrt(sum / (n-1)); // 标准差 = 平方和的平均值开根号（使用 n-1 表示样本标准差）
}