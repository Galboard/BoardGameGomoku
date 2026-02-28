#!/bin/bash

# 1. 告诉脚本遇到错误就停止执行
set -e

# 2. 准备构建目录
echo ">>> 正在清理并准备 build 目录..."
mkdir -p build
cd build

# 3. 指定 ARM 交叉编译器并运行 CMake
# 注意：如果你的板子是 32 位，请把 aarch64-linux-gnu- 替换为 arm-linux-gnueabihf-
echo ">>> 正在生成 CMake 配置..."
cmake -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
      -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
      ..

# 4. 执行编译（-j4 表示使用 4 个线程并行编译，能加快速度）
echo ">>> 正在编译可执行文件..."
make -j4

echo ">>> 编译完成！可执行文件已在 build 目录下生成。"