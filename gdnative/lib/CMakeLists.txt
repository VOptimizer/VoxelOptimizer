cmake_minimum_required(VERSION 3.3.0)
project(voxeloptimizer VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SRCS "${PROJECT_SOURCE_DIR}/src/Loaders/MagicaVoxelLoader.cpp"
         "${PROJECT_SOURCE_DIR}/src/Loaders/VoxelMesh.cpp"
         "${PROJECT_SOURCE_DIR}/src/Meshers/SimpleMesher.cpp"
         "${PROJECT_SOURCE_DIR}/src/Meshers/GreedyMesher.cpp"
         "${PROJECT_SOURCE_DIR}/src/Meshers/IMesher.cpp"
         "${PROJECT_SOURCE_DIR}/src/Meshers/Slicer/Slicer.cpp"
         "${PROJECT_SOURCE_DIR}/src/Exporters/WavefrontObjExporter.cpp"
         "${PROJECT_SOURCE_DIR}/src/Exporters/IExporter.cpp"
         "${PROJECT_SOURCE_DIR}/src/Exporters/SpriteStackingExporter.cpp"
         "${PROJECT_SOURCE_DIR}/src/Exporters/glTF/GLTFExporter.cpp"
         "${PROJECT_SOURCE_DIR}/src/stb_image_write.c")
#set(LIBS "")

include_directories("${PROJECT_SOURCE_DIR}/include"
                    "${PROJECT_SOURCE_DIR}/third_party")

add_library(${PROJECT_NAME} STATIC ${SRCS})
#target_link_libraries(${PROJECT_NAME} ${LIBS})