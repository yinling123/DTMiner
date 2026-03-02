/* 
 * 文件: Node.h
 * 作者: D3M430
 *
 * 创建时间: 2017年1月13日 上午11:43
 */

#ifndef NODE_H
#define NODE_H

#include <vector>

/**
 * 图中的基本节点类。
 * 包含该节点的所有边（包括出边和入边），并按时间顺序存储。
 */
class Node
{
public:    
    /**
     * 获取该节点的所有边（包括出边和入边），按时间顺序排列。
     * @return 包含所有边索引的向量。
     */
    std::vector<int> &edges() { return _edges; }

    /**
     * 获取该节点的出边，按时间顺序排列。
     * @return 包含出边索引的向量。
     */
    std::vector<int> &outEdges() { return _outEdges; }

    /**
     * 获取该节点的入边，按时间顺序排列。
     * @return 包含入边索引的向量。
     */
    std::vector<int> &inEdges() { return _inEdges; }

    /**
     * 获取该节点的所有边（包括出边和入边），按时间顺序排列（只读版本）。
     * @return 包含所有边索引的常量向量。
     */
    const std::vector<int> &edges() const { return _edges; }

    /**
     * 获取该节点的出边，按时间顺序排列（只读版本）。
     * @return 包含出边索引的常量向量。
     */
    const std::vector<int> &outEdges() const { return _outEdges; }

    /**
     * 获取该节点的入边，按时间顺序排列（只读版本）。
     * @return 包含入边索引的常量向量。
     */
    const std::vector<int> &inEdges() const { return _inEdges; }

private:
    std::vector<int> _edges;    // 存储该节点的所有边索引（包括出边和入边）。
    std::vector<int> _outEdges; // 存储该节点的出边索引。
    std::vector<int> _inEdges;  // 存储该节点的入边索引。
};

#endif /* NODE_H */