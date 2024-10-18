cmake_minimum_required(VERSION 2.8)

project(project_hw LANGUAGES CXX C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17") # 设置c++的编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99") # 设置c的编译选项

set(CMAKE_C_STANDARD 99)

set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
set(ALG_DIR ${PROJECT_SOURCE_DIR}/algorithm)
set(LKH_DIR ${PROJECT_SOURCE_DIR}/lib/LKH/SRC)

# 设置头文件目录变量
set(HEADER_DIRS

    # ${LIB_DIR}
    ${ALG_DIR}
    ${LKH_DIR}/INCLUDE
)

# 添加源文件变量
set(SOURCES
    ${PROJECT_SOURCE_DIR}/main.c
    ${ALG_DIR}/algorithm.c
    ${ALG_DIR}/greedy.cpp
)

# 查找 ALG_DIR 目录下的所有 .c 文件并添加到 SOURCES 列表
file(GLOB ALG_SOURCES ${ALG_DIR}/*.c)

file(GLOB LKH_SOURCES ${ALG_DIR}/*.c ${LKH_DIR}/*.c)
list(REMOVE_ITEM LKH_SOURCES ${LKH_DIR}/LKHmain.c)

# 将所有 ALG_SOURCES 文件添加到 SOURCES 变量
list(APPEND SOURCES ${ALG_SOURCES} ${LKH_SOURCES})

# 包含头文件目录
include_directories(${HEADER_DIRS})

# 生成可执行文件
add_executable(project_hw ${SOURCES})
target_link_libraries(project_hw PRIVATE ${PROJECT_SOURCE_DIR}/lib/libseek_model.so)

add_library(project_hw_dl SHARED ${SOURCES})

# target_link_libraries(project_hw PRIVATE ${PROJECT_SOURCE_DIR}/lib/libseek_model.so)

# 添加编译选项和宏定义
target_compile_options(project_hw PRIVATE -g)
target_compile_definitions(project_hw PRIVATE _GNU_SOURCE)
