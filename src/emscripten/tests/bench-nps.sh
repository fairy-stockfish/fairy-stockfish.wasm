#!/bin/bash

this_dir=$(dirname "${BASH_SOURCE[0]}")

uci_exe_default="node $this_dir/../public/uci.js"
uci_exe=${uci_exe:-$uci_exe_default}

# 200 knodes/s
nps_target=${nps_target:-200000}

# Hash = 128MB
# Threads = 1
# Depth = 22
bench_command=${bench_command:-"bench xiangqi 128 1 22 current depth NNUE"}

echo -n "Running bench ... "

nps=$(eval "$uci_exe $bench_command" | grep "Nodes/second" | awk '{ print $NF }')

echo -n "(nps, nps_target) = ($nps, $nps_target) ... "

if [ "$nps" -ge "$nps_target" ]; then
  echo "[ok]"
else
  echo "[fail]"
  exit 1
fi
