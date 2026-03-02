/* 
 * 文件:   Edge.h
 * 作者: D3M430
 *
 * 创建时间: 2017年1月13日 下午2:36
 */

#ifndef EDGE_H
#define EDGE_H

#include <time.h>

/**
 * 定义图中的基本有向边。
 */
class Edge
{
public:
    /**
     * 构造函数
     * @param index 边的索引（也隐含了边的发生顺序）。
     * @param source 源节点的索引（从0开始）。
     * @param dest 目标节点的索引（从0开始）。
     * @param dateTime 边的时间戳（如果不可用，可以使用顺序整数代替）。
     */
    Edge(int index, int source, int dest, time_t dateTime) {
        _index = index;  // 设置边的索引。全局边列表索引
        _source = source; // 设置源节点的索引。
        _dest = dest;     // 设置目标节点的索引。
        _time = dateTime; // 设置边的时间戳。
    }

    /**
     * 返回边的索引。
     * @return 边的索引。
     */
    int index() const { return _index; }

    /**
     * 返回源节点的索引。
     * @return 源节点的索引。
     */
    int source() const { return _source; }

    /**
     * 返回目标节点的索引。
     * @return 目标节点的索引。
     */
    int dest() const { return _dest; }

    /**
     * 返回边的时间戳。
     * @return 边的时间戳。
     */
    time_t time() const { return _time; }

private:
    int _index;  // 边的索引。
    int _source; // 源节点的索引。
    int _dest;   // 目标节点的索引。
    time_t _time; // 边的时间戳。
};

#endif /* EDGE_H */