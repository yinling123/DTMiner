#include "GraphReader.h"

DataGraph loadFile(const std::string &filename)
{

    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("无法打开文件: " + filename);
    }

    DataGraph g;
    std::string line;
    int edge_index = 0;
    std::unordered_map<int, int> node_id_map; // 原始ID到新ID的映射
    int next_node_id = 0;                     // 下一个可用的新节点ID

    // 单次扫描处理所有数据
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        int u, v;
        time_t t;
        if (!(iss >> u >> v >> t))
        {
            throw std::runtime_error("格式错误: " + line);
        }

        edge_index++;

        if (edge_index % 100000 == 0)
        {
            std::cout << "已读取 " << edge_index << " 条边" << std::endl;
        }

        // 动态分配节点ID
        auto map_node = [&](int raw_id) -> int
        {
            if (node_id_map.find(raw_id) == node_id_map.end())
            {
                node_id_map[raw_id] = next_node_id;
                g.Graph::addNode(next_node_id++);
            }
            return node_id_map[raw_id];
        };

        // 获取映射后的节点ID
        int new_u = map_node(u);
        int new_v = map_node(v);

        // 存储边信息（包含时间和原始ID）
        g.Graph::addEdge(new_u, new_v, t);

        // 边索引增加
    }

    return g;
}
