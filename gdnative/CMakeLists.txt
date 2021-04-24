cmake_minimum_required(VERSION 3.3.0)
project(godotvoxeloptimizer VERSION 0.1.0 LANGUAGES CXX C)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)


set(SRCS "${PROJECT_SOURCE_DIR}/main.cpp"
         "${PROJECT_SOURCE_DIR}/src/GodotVoxelOptimizer.cpp")
set(LIBS "godot-cpp.linux.debug.64"
         "voxeloptimizer")

include_directories("${PROJECT_SOURCE_DIR}/../godot-cpp/include"
                    "${PROJECT_SOURCE_DIR}/../godot-cpp/include/core"
                    "${PROJECT_SOURCE_DIR}/../godot-cpp/include/gen"
                    "${PROJECT_SOURCE_DIR}/../godot-cpp/godot-headers"
                    "${PROJECT_SOURCE_DIR}/lib/include"
                    "${PROJECT_SOURCE_DIR}/include")

link_directories("${PROJECT_SOURCE_DIR}/../godot-cpp/bin")


add_library(${PROJECT_NAME} SHARED ${SRCS})
target_link_libraries(${PROJECT_NAME} ${LIBS})