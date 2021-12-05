/*
 * MIT License
 *
 * Copyright (c) 2021 Christian Tost
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef VOXELOPTIMIZER_HPP
#define VOXELOPTIMIZER_HPP

#include <VoxelOptimizer/BBox.hpp>
#include <VoxelOptimizer/Color.hpp>
#include <VoxelOptimizer/Exceptions.hpp>
#include <VoxelOptimizer/Material.hpp>
#include <VoxelOptimizer/Mesh.hpp>
#include <VoxelOptimizer/Vector.hpp>

#include <VoxelOptimizer/Exporters/IExporter.hpp>
#include <VoxelOptimizer/Exporters/SpriteStackingExporter.hpp>
#include <VoxelOptimizer/Exporters/WavefrontObjExporter.hpp>
#include <VoxelOptimizer/Exporters/GLTFExporter.hpp>
#include <VoxelOptimizer/Exporters/GodotSceneExporter.hpp>
#include <VoxelOptimizer/Exporters/PLYExporter.hpp>
#include <VoxelOptimizer/Loaders/ILoader.hpp>
#include <VoxelOptimizer/Loaders/GoxelLoader.hpp>
#include <VoxelOptimizer/Loaders/MagicaVoxelLoader.hpp>
#include <VoxelOptimizer/Loaders/VoxelMesh.hpp>
#include <VoxelOptimizer/Loaders/KenshapeLoader.hpp>
#include <VoxelOptimizer/Loaders/QubicleBinaryLoader.hpp>
#include <VoxelOptimizer/Loaders/QubicleBinaryTreeLoader.hpp>
#include <VoxelOptimizer/Loaders/QubicleExchangeLoader.hpp>
#include <VoxelOptimizer/Loaders/QubicleLoader.hpp>
#include <VoxelOptimizer/Meshers/GreedyMesher.hpp>
#include <VoxelOptimizer/Meshers/SimpleMesher.hpp>
#include <VoxelOptimizer/Meshers/MarchingCubesMesher.hpp>
// #include <VoxelOptimizer/Meshers/VerticesReducer.hpp>

#endif //VOXELOPTIMIZER_HPP