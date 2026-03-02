#!/bin/bash

# 数据集和对应的 delta
DATASETS=(
    "/home/share/chenh/hyb/dataset/wiki-talk-temporal.txt"
    "/home/share/chenh/hyb/dataset/sx-stackoverflow.txt"
    "/home/share/chenh/hyb/dataset/temporal-reddit-reply.txt"
    "/home/share/chenh/hyb/dataset/ethereum.txt"
)

DELTAS=("24h" "24h" "10h" "1h")

MOTIF_DIR="/home/chenh/hyb/project/version24/block_based_temporal_subgraph/motifs"
OUTPUT_PREFIX="out"

# 遍历每个数据集
for i in "${!DATASETS[@]}"; do
    GRAPH_PATH="${DATASETS[i]}"
    DELTA="${DELTAS[i]}"

    echo "============== Running on dataset: $(basename "$GRAPH_PATH")  Δ=$DELTA =============="

    # 直接给目录，内部程序自己枚举 motif
    time ./graph_search -g "$GRAPH_PATH" -qf "$MOTIF_DIR" -o "${OUTPUT_PREFIX}_$(basename "$GRAPH_PATH")" -delta "$DELTA"

    echo "Sleeping for 10 seconds before next dataset..."
    sleep 10
done
