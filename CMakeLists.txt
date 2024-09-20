cmake_minimum_required(VERSION 2.8)

project(project_hw)

set(CMAKE_C_STANDARD 99)

set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
set(ALG_DIR ${PROJECT_SOURCE_DIR}/algorithm)

# 设置头文件目录变量
set(HEADER_DIRS
    ${LIB_DIR}
    ${ALG_DIR}
)

# 添加源文件变量
set(SOURCES
    ${PROJECT_SOURCE_DIR}/main.c
    ${ALG_DIR}/algorithm.c
)

# 查找 ALG_DIR 目录下的所有 .c 文件并添加到 SOURCES 列表
file(GLOB ALG_SOURCES ${ALG_DIR}/*.c)

# 将所有 ALG_SOURCES 文件添加到 SOURCES 变量
list(APPEND SOURCES ${ALG_SOURCES})

# 包含头文件目录
include_directories(${HEADER_DIRS})

# 生成可执行文件
add_executable(project_hw  ${SOURCES})
target_link_libraries(project_hw PRIVATE ${PROJECT_SOURCE_DIR}/lib/libseek_model.so)

# 添加编译选项和宏定义
target_compile_options(project_hw PRIVATE -g -std=c99)
target_compile_definitions(project_hw PRIVATE _GNU_SOURCE)
