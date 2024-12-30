#!/usr/bin/env bash
set -ex

FILE="$1"
OUTPUT_BIN="main"

g++ \
  -lcurl \
  -lfmt \
  -o "$OUTPUT_BIN" \
  -I ./include \
  $(find ./src -name "*.cc") \
  "$FILE"

echo "Compiled $FILE -> $OUTPUT_BIN"
