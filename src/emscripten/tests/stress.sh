#!/bin/bash

this_dir=$(dirname "${BASH_SOURCE[0]}")

n=${n:-20}
bench_command=${bench_command:-"bench xiangqi 128 2 15 default depth NNUE"}
nps_target=${nps_target:-"100000"}

echo -n "Running 0/$n ... "

export bench_command
export nps_target
for (( i=0; i < n; i++ )); do
  if ! bash "$this_dir"/bench-nps.sh > /dev/null 2>&1; then
    echo "[fail]"
    exit 1
  fi
  printf "\r"
  echo -n "Running $((i + 1))/$n ... "
done

echo "[ok]"
