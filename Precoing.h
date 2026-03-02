#ifndef PRECOING_H
#define PRECOING_H

#include "DataGraph.h"
#include "Minfo.h"
#include <unordered_set>

class Precoing
{
public:
    std::vector<Minfo> getMinfo(DataGraph &h);

    void generateCode(std::vector<Minfo> &, DataGraph &h, int nodes[]);
};

/**
 * 获取对应查询图的模式结构，并且生成对应层级的情况，便于运行时判断
 */
std::vector<Minfo> Precoing::getMinfo(DataGraph &h)
{
    auto edges = h._edges;
    std::vector<Minfo> minfos;
    int nodeMax = 1;
    std::unordered_set<int> nodeSet;
    nodeSet.insert(edges[0].source());
    nodeSet.insert(edges[0].dest());
    minfos.push_back(Minfo{0, 0, 0, 0, true});

    for (int i = 1; i < edges.size(); i++)
    {
        auto edge = edges[i];
        Minfo minfo;
        /**
         * 按照时序去推断当前边的出入点，预处理下，这样就不需要运行时判断
         * 如果两点皆为被映射，即为跳边的情况：这个逻辑一致，都是双点判断情况'
         * 还需要考虑一种两个点都无，并且自环的情况;
         * 两点未映射，那肯定找不到点呀，后续再修改
         */
        if (nodeSet.find(edge.source()) == nodeSet.end() && nodeSet.find(edge.dest()) == nodeSet.end())
        {
            minfo.baseNode = edge.dest();
            minfo.constraintNode = edge.source();
            minfo.newNode = 2;
            minfo.io = -1;
            if (edge.source() != edge.dest())
            {
                minfo.mappedNodes = nodeMax + 2;
                nodeMax += 2;
                nodeSet.insert(edge.dest());
                nodeSet.insert(edge.source());
            }
            else
            {
                minfo.mappedNodes = nodeMax + 1;
                nodeMax += 1;
                nodeSet.insert(edge.dest());
            }
        }
        else if (nodeSet.find(edge.source()) == nodeSet.end())
        {
            minfo.baseNode = edge.dest();
            minfo.constraintNode = -edge.source();
            minfo.io = 0;
            minfo.mappedNodes = nodeMax + 1;
            minfo.newNode = 1;
            nodeMax++;
            nodeSet.insert(edge.source());
        }
        else if (nodeSet.find(edge.dest()) == nodeSet.end())
        {
            minfo.baseNode = edge.source();
            minfo.constraintNode = -edge.dest();
            minfo.io = 1;
            minfo.mappedNodes = nodeMax + 1;
            minfo.newNode = 1;
            nodeMax++;
            nodeSet.insert(edge.dest());
        }
        else
        {
            minfo.baseNode = edge.dest();
            // 当cons不为负数时，cons为源顶点，dst为出顶点
            minfo.constraintNode = edge.source();
            minfo.io = -1;
            minfo.mappedNodes = nodeMax + 1;
            minfo.newNode = 0;
        }
        minfos.push_back(minfo);
    }
    return minfos;
}

inline void Precoing::generateCode(std::vector<Minfo> &minfo, DataGraph &h, int nodes[])
{
    /**
     * 生成GraphSearch_t_cpp.sh代码
     */
    std::stringstream ss;
    std::ifstream in("GraphSearch_t_cpp.sh");
    ss << in.rdbuf();
    ss << "\tif (minfo -> newNode == 1)\n";
    ss << "\t{\n";
    ss << "\t\tswitch (minfo -> mappedNodes)\n";
    ss << "\t\t{\n";
    for (int i = 5; i >= 2; i--)
    {
        if (i != 2)
        {
            ss << "\t\tcase " << i << ":\n";
            ss << "\t\t\tchecked = checked && (_h2gNodes[" << nodes[i - 1] << "] != node);\n";
        }
        else if (i == 2)
        {
            ss << "\t\tcase 2:\n";
            ss << "\t\t\tchecked = checked && (_h2gNodes[" << nodes[0] << "] != node) && (_h2gNodes[" << nodes[1] << "] != node);\n";
        }
    }
    ss << "\t\t}\n";
    ss << "\t}\n";
    ss << "\telse if(minfo -> newNode == 0)\n";
    ss << "\t{\n";
    ss << "\t\tswitch(minfo -> constraintNode)\n";
    ss << "\t\t{\n";
    for (int i = 0; i < 5; i++)
    {
        ss << "\t\tcase " << i << ":\n";
        ss << "\t\t\tchecked = (_h2gNodes[" << i << "] == node);\n";
        ss << "\t\t\tbreak;\n";
    }
    ss << "\t\t}\n";
    ss << "\t}\n";
    ss << "\telse if(minfo -> newNode == 2)\n";
    ss << "\t{\n";
    ss << "\t\tint node0 = g_u;\n";
    ss << "\t\tint node1 = g_v;\n";
    ss << "\t\tbool flag = (node0 != node1) && (g_u != g_v) || (node0 == node1) && (g_u == g_v);\n";
    ss << "\t\tif(flag)\n";
    ss << "\t\t{\n";
    ss << "\t\t\t return false;\n";
    ss << "\t\t}\n";
    ss << "\t\tswitch (minfo -> mappedNodes - 1)\n";
    ss << "\t\t{\n";
    for (int i = 5; i >= 2; i--)
    {
        if (i != 2)
        {
            ss << "\t\tcase " << i << ":\n";
            ss << "\t\t\tchecked = checked && (_h2gNodes[" << nodes[i - 1] << "] != node0);\n";
        }
        else if (i == 2)
        {
            ss << "\t\tcase 2:\n";
            ss << "\t\t\tchecked = checked && (_h2gNodes[" << nodes[0] << "] != node0) && (_h2gNodes[" << nodes[1] << "] != node0);\n";
        }
    }
    ss << "\t\t}\n";
    ss << "\t\tif(node0 != node1);\n";
    ss << "\t\t{\n";
    ss << "\t\t\tswitch (minfo -> mappedNodes)\n";
    ss << "\t\t\t{\n";
    for (int i = 5; i >= 2; i--)
    {

        if (i != 2)
        {
            ss << "\t\t\tcase " << i << ":\n";
            ss << "\t\t\t\tchecked = checked && (_h2gNodes[" << nodes[i - 1] << "] != node1);\n";
        }
        else if (i == 2)
        {
            ss << "\t\t\tcase 2:\n";
            ss << "\t\t\t\tchecked = checked && (_h2gNodes[" << nodes[0] << "] != node1) && (_h2gNodes[" << nodes[1] << "] != node1);\n";
        }
    }
    ss << "\t\t\t}\n";
    ss << "\t\t}\n";
    ss << "\t}\n";

    ss << "\treturn checked;\n";
    ss << "}\n";

    std::ofstream out("GraphSearch.sh");
    out << ss.str();
    out.close();
}

#endif