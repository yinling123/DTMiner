#include "CertGraph.h"

#ifndef TESTS__H
#define TESTS__H

/**
 * 提供用于测试搜索结果等功能的工具类。
 */
class Tests
{
public:
    /**
     * 统计图中重复边的数量。
     * @param g 输入的图对象（CertGraph 类型）。
     * @return 返回图中重复边的数量。
     */
    static int countDuplicateEdges(const CertGraph &g);
};

#endif