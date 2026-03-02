#!/bin/bash

DATASETS_DIR="/home/share/chenh/hyb/dataset"
MOTIF_DIR="/home/chenh/hyb/project/version24/block_based_temporal_subgraph/motifs"
OUTPUT_PREFIX="out"
TIMEOUT_DURATION="24h"
EXIT_LOG="exit.txt"

# 只保留 wiki 和 stackoverflow
GRAPH_FILES=("wiki-talk-temporal.txt" "sx-stackoverflow.txt")

# M1 到 M13
MOTIF_IDS=($(seq -f "M%g" 1 13))

for graph_file in "${GRAPH_FILES[@]}"; do
    GRAPH_PATH="$DATASETS_DIR/$graph_file"
    
    if [[ ! -f "$GRAPH_PATH" ]]; then
        echo "❌ Graph file not found: $GRAPH_PATH" | tee -a "$EXIT_LOG"
        continue
    fi

    dataset="${graph_file%.txt}"
    DELTA="24h"  # 固定为 24h

    for motif_id in "${MOTIF_IDS[@]}"; do
        motif_file="$MOTIF_DIR/${motif_id}.txt"

        if [[ ! -f "$motif_file" ]]; then
            echo "❌ Motif file not found: $motif_file" | tee -a "$EXIT_LOG"
            continue
        fi

        output_file="${OUTPUT_PREFIX}_${dataset}_${motif_id}.gdf"

        echo "▶ Running dataset: $dataset, graph: $graph_file, motif: $motif_id, DELTA=$DELTA"

        # 执行时屏蔽输出
        timeout "$TIMEOUT_DURATION" ./graph_search -g "$GRAPH_PATH" -q "$motif_file" -o "$output_file" -delta "$DELTA" > /dev/null 2>&1

        echo "💤 Sleeping for 10 seconds..."
        sleep 10
    done
done

echo "✅ All tasks completed. Failures logged in $EXIT_LOG"
