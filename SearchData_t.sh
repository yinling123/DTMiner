#ifndef SEARCHDATA_H
#define SEARCHDATA_H

#include <time.h>
#include <vector>
#include <stack>
#include "ManualStack.h"
#include "SegMap.h"

struct alignas(64) SearchData
{
    SearchData() = default;

    // 显式拷贝构造函数
    // SearchData(const SearchData &other)
    //     : _h2gNodes(other._h2gNodes),
    //       map(other.map),
    //       _sg_edgeStack(other._sg_edgeStack), // 确保 ManualStack 有正确的拷贝构造
    //       candidates(other.candidates),
    //       h_i(other.h_i),
    //       _firstEdgeTime(other._firstEdgeTime)
    // {
    //     // 深拷贝 arr 指向的内容（如果需要）
    //     for (int i = 0; i < 8; ++i)
    //     {
    //         if (other.arr[i])
    //         {
    //             // 假设 arr[i] 指向的 vector 是只读的，可共享
    //             arr[i] = other.arr[i];
    //         }
    //         else
    //         {
    //             arr[i] = nullptr;
    //         }
    //     }
    // }

    // SearchData(const SearchData &) = delete;
    // SearchData &operator=(const SearchData &) = delete;

    // // /**
    // //  * 移动构造函数
    // // */
    // SearchData(SearchData &&data)
    // {
    //     _firstEdgeTime = data._firstEdgeTime;
    //     _h2gNodes = std::move(data._h2gNodes);
    //     _sg_edgeStack = std::move(data._sg_edgeStack);
    //     candidates = std::move(data.candidates);
    //     // g_i = data.g_i;
    //     h_i = data.h_i;
    //     map = std::move(data.map);
    //     arr[0] = data.arr[0];
    //     arr[1] = data.arr[1];
    //     arr[2] = data.arr[2];
    //     arr[3] = data.arr[3];
    //     arr[4] = data.arr[4];
    // }

    // /**
    //  * 移动赋值函数
    //  */
    // SearchData &operator=(SearchData &&data)
    // {
    //     _firstEdgeTime = data._firstEdgeTime;
    //     _h2gNodes = std::move(data._h2gNodes);
    //     _sg_edgeStack = std::move(data._sg_edgeStack);
    //     candidates = std::move(data.candidates);
    //     // g_i = data.g_i;
    //     h_i = data.h_i;
    //     map = std::move(data.map);
    //     arr[0] = data.arr[0];
    //     arr[1] = data.arr[1];
    //     arr[2] = data.arr[2];
    //     arr[3] = data.arr[3];
    //     arr[4] = data.arr[4];

    //     return *this;
    // }

    // std::vector<int> _h2gNodes;
    # int _h2gNodes[5];
    // std::vector<int> _g2hNodes; // 节点映射表
    // std::vector<int> _numSearchEdgesForNode; // 每个节点的搜索边计数
    // std::vector<std::pair<int, int>> map;
    // std::stack<int> _sg_edgeStack; // 存储搜索路径中的边索引，回溯需要
    // ManualStack<int> _sg_edgeStack;
    // std::vector<int> _allEdges; // 所有边的索引列表
    // std::vector<std::pair<int, int>> candidates; // 当前层的候选边列表，边数量-1
    # std::pair<int, int> candidates[5];
    // 存储当前层级的搜索数组
    const std::vector<int> *arr[5] = {nullptr};
    // 存储motif的开始匹配点
    int h_i = 0;
    time_t _firstEdgeTime; // 第一条匹配边的时间

    // char ch[24];
    // 存储图边的当前匹配点
    // int g_i = -1;
