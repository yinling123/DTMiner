# DTMiner (Final)

一个面向**时序子图匹配（Temporal Subgraph Matching）**的 C++ 项目实现，当前代码版本聚焦于基于分块与多线程/NUMA 优化的匹配流程。

## Features

- 时序有向图子图匹配
- 支持时间窗口约束（`-delta`）
- 支持批量查询文件（`-qf`）
- 多线程执行与 NUMA 亲和性相关优化
- 提供 motif 示例与批量运行脚本

## Repository Structure

```text
.
├── block_based_temporal_subgraph/   # 核心源码与脚本
│   ├── Makefile
│   ├── main.cpp
│   ├── CmdArgs.cpp/h
│   ├── GraphSearch.cpp/h
│   ├── motifs/                      # 示例查询图（M1~M13）
│   └── LICENSE.txt
├── readme.txt                       # 旧说明
└── README.md                        # 当前文档
```

## Requirements

- Linux (推荐)
- `g++`（支持 C++17）
- `make`
- `libnuma`（头文件和链接库）

## Build

进入核心目录并编译：

```bash
cd block_based_temporal_subgraph
make
```

编译成功后生成可执行文件：

- `graph_search`

## Important Build Notes

当前 `Makefile` 有以下硬编码配置（请按本机环境调整）：

- 编译器：`CXX = g++-9`
- NUMA 头文件路径：`-I/home/chenh/hyb/software/numactl-2.0.17-soft/include`
- NUMA 库路径：`-L/home/chenh/hyb/software/numactl-2.0.17-soft/lib`
- 链接库：`-lnuma`

如果你的系统已通过包管理器安装 `libnuma-dev`，通常可以改成系统默认路径。

## Input Format

当前主流程通过 `loadFile()` 读取图文件，格式为**每行一条边**：

```text
src_node_id dst_node_id timestamp
```

- 三列均为整数
- 支持注释行（以 `#` 开头）和空行
- 查询图与数据图都使用该格式

示例（`motifs/M1.txt`）：

```text
1 2 1
2 3 2
3 4 3
```

## Usage

```bash
./graph_search -g <graph_file> -q <query_file> [-delta <duration>] [-o <output_name>]
./graph_search -g <graph_file> -qf <query_folder> [-delta <duration>] [-o <output_name>]
```

参数说明：

- `-g`：数据图文件路径（必选）
- `-q`：单个查询图路径（必选，或用 `-qf`）
- `-qf`：查询图目录（当前会枚举 `.txt` 文件）
- `-delta`：时间窗口，支持 `s/m/h/d/w` 后缀（默认 `24h`）
- `-o`：输出文件名（当前版本仅解析参数，见下文说明）

示例：

```bash
./graph_search -g /path/to/data.txt -q motifs/M1.txt -delta 1h
./graph_search -g /path/to/data.txt -qf motifs -delta 24h
```

## Output

当前 `main.cpp` 默认会将统计结果追加写入：

- `res_final.txt`

典型字段顺序：

```text
<input_graph_name> <query_name> <time_ms> <match_count> <preprocess_ms> <peak_rss_kb>
```

说明：

- 代码中 `-o` 对应的 GDF 结果导出逻辑目前被注释（`saveGenericGDF` 相关段落未启用）。
- 如果你需要导出组合子图，可在 `main.cpp` 中恢复对应注释代码。

## Scripts

项目中包含若干批处理脚本（如 `run_all_motifs.sh`、`perf_all_motifs.sh`、`scalable.sh`），用于批量实验与性能统计。

注意：

- 这些脚本里的数据集路径是作者本地绝对路径，使用前请先修改。

## License

本项目核心代码目录包含许可证文件：

- `block_based_temporal_subgraph/LICENSE.txt`

请在分发或二次开发前阅读该文件条款。

## Citation

如果你的工作使用了该时序子图匹配方向的思路，可参考原始相关论文（见 `block_based_temporal_subgraph/README.TXT` 中给出的论文信息）。
