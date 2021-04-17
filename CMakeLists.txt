cmake_minimum_required(VERSION 3.3.0)
project(VoxelOptimizer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRCS "${PROJECT_SOURCE_DIR}/main.cpp")
#set(LIBS "")

include_directories("${PROJECT_SOURCE_DIR}/gdnative/lib/include")

add_subdirectory("${PROJECT_SOURCE_DIR}/gdnative/lib")

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} voxeloptimizer)