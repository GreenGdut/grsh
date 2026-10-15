#!/bin/bash
set -e

cd "$(dirname "$0")"

# 仅在传入 clean 参数时清理
if [[ "${1:-}" == "clean" ]]; then
    echo "cleaning..."
    rm -rf build bin
fi

echo "configuring..."
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$PWD/bin"

echo "building..."
cmake --build build

echo "build complete!"