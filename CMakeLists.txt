# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(fio_benchmark_v2)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 创建可执行文件
add_executable(fio_v2.0
        main.cpp
        fio.cpp
)

# 如果有额外的链接库，可以在这里添加
# target_link_libraries(fio_v2.0 pthread)