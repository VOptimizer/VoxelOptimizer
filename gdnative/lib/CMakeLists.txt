cmake_minimum_required(VERSION 3.3.0)
project(voxeloptimizer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SRCS "${PROJECT_SOURCE_DIR}/src/VoxelLoader.cpp"
         "${PROJECT_SOURCE_DIR}/src/SimpleMesher.cpp"
         "${PROJECT_SOURCE_DIR}/src/OptimizeMesher.cpp"
         "${PROJECT_SOURCE_DIR}/src/WavefrontObjExporter.cpp")
#set(LIBS "")

include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(${PROJECT_NAME} STATIC ${SRCS})
#target_link_libraries(${PROJECT_NAME} ${LIBS})