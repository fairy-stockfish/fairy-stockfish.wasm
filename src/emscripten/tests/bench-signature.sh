#!/bin/bash

this_dir=$(dirname "${BASH_SOURCE[0]}")

uci_exe_default="node $this_dir/../public/uci.js"
uci_exe=${uci_exe:-$uci_exe_default}

if [ -z "$sign_ref" ]; then
  sign_ref=$(git log | grep "bench:" | head -n 1 | awk '{print $NF}')
fi

echo -n "Running bench ... "

sign=$(eval "$uci_exe bench xiangqi 16 1 13 default depth NNUE" | grep "Nodes searched" | awk '{ print $NF }')

echo -n "(sign, sign_ref) = ($sign, $sign_ref) ... "

if [ "$sign" == "$sign_ref" ]; then
  echo "[ok]"
else
  echo "[fail]"
  exit 1
fi
