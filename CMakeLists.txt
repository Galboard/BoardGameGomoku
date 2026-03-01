cmake_minimum_required(VERSION 3.20)

project(gomoku)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 1. 添加源文件
add_executable(gomoku main.cpp)

target_sources(gomoku PRIVATE
    src/lcd.cpp
)

# 2. 包含头文件目录
target_include_directories(gomoku PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 3. 开启静态链接（关键！）
# 这会向链接器传递 -static 参数，把所有的依赖（包括 C/C++ 标准库）全部打包进可执行文件
target_link_options(gomoku PRIVATE -static)