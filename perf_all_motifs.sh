#!/bin/bash

GRAPH_PATH="/home/share/chenh/hyb/dataset/ethereum.txt"
MOTIF_DIR="/home/chenh/hyb/project/version24/block_based_temporal_subgraph/motifs"
OUTPUT_PREFIX="out"
DELTA="1h"
TIMEOUT_DURATION="24h"
EXIT_LOG="exit.txt"
PERF_METRICS="cache-references,cache-misses,LLC-load-misses,LLC-store-misses,branch-misses"
PERF_DIR="perf_result_5/ethereum"

mkdir -p "$PERF_DIR"  # 创建目录（如果不存在）

for motif_file in "$MOTIF_DIR"/M*.txt; do
    motif_name=$(basename "$motif_file" .txt)
    output_file="${OUTPUT_PREFIX}_${motif_name}.gdf"
    perf_output="${PERF_DIR}/${motif_name}.txt"

    echo "Running with motif: $motif_file"

    if ! timeout "$TIMEOUT_DURATION" sudo perf stat -e $PERF_METRICS -o "$perf_output" -- \
        ./graph_search -g "$GRAPH_PATH" -q "$motif_file" -o "$output_file" -delta "$DELTA"; then

        echo "Timeout or failure on: $motif_file"
        echo "$(basename "$motif_file")" >> "$EXIT_LOG"
        continue
    fi

    echo "Sleeping for 60 seconds before next task..."
    sleep 60
done
