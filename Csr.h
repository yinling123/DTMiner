#ifndef CSR_H
#define CSR_H

#include <vector>
#include <map>
#include <unordered_map>
#include <time.h>
#include "Node.h"
#include "Edge.h"

/**
 * @brief 定义了一个标准的有向图类，支持按时间顺序存储边。
 */
class Csr
{
public:
    int _numEdges = 0;
    std::vector<int> in_vertex;
    std::vector<int> in_offset;
    std::vector<int> out_vertex;
    std::vector<int> out_offset;
    mutable std::vector<Edge> _edges;
};

#endif