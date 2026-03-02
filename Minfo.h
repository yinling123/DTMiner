#ifndef MINFO_H
#define MINFO_H

/**
 * 存储每层级的信息
 * @baseNode 存储当前层的节点
 * @constraintNode 存储当前层的约束点 为-1表示不存在约束，因为是新加边；不为-1表示需要满足当前边另一个点要满足对应要求
 * @io 存储当前层的新增点的出入方向 0表示当前baseNode的入边索引；1表示当前baseNode的出边索引；-1表示需要进行自动判断，
 * 因为当前点不是新点或者作为起始边，选择边索引量比较小的二分
 * @mappedNodes 存储当前层已经映射的点的数量
 * @newNode 判断当前层级是否新增顶点
 */

class alignas(64) Minfo
{

public:
    int baseNode;
    int constraintNode;
    int io;
    int mappedNodes;
    int newNode;
};

#endif