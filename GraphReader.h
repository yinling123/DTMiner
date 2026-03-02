#ifndef GRAPH_DATA_LOADER_H
#define GRAPH_DATA_LOADER_H

#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include "Edge.h"
#include "Node.h"
#include "DataGraph.h"

DataGraph loadFile(const std::string &filename);

// 使用示例保持不变
#endif