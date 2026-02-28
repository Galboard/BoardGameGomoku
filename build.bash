#!/bin/bash

# 1. 告诉脚本遇到错误就停止执行
set -e

# 2. 彻底清理并准备 build 目录（非常关键，防止旧的 64 位缓存捣乱）
echo ">>> 正在清理旧的 build 目录..."
rm -rf build
mkdir -p build
cd build

# 3. 指定 32 位 ARM 交叉编译器并运行 CMake
echo ">>> 正在生成 32 位 ARM 的 CMake 配置..."
cmake -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \
      -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++ \
      ..

# 4. 执行编译（-j4 表示使用 4 个线程并行编译）
echo ">>> 正在编译可执行文件..."
make -j4

echo ">>> 编译完成！32 位可执行文件已在 build 目录下生成。"