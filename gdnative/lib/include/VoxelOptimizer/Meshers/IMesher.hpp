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

#ifndef IMESHER_HPP
#define IMESHER_HPP

#include <map>

#include <VoxelOptimizer/Material.hpp>
#include <VoxelOptimizer/Loaders/VoxelMesh.hpp>
#include <VoxelOptimizer/Loaders/ILoader.hpp>

#include <VoxelOptimizer/Mesh.hpp>

namespace VoxelOptimizer
{
    class IMesher
    {
        public:
            IMesher() = default;

            /**
             * @brief Generates list of meshed chunks.
             * 
             * @param m: Voxel mesh to meshify.
             */
            virtual std::map<CVector, Mesh> GenerateMeshes(VoxelMesh m, Loader Loader) = 0;

            virtual ~IMesher() = default;
        protected:
            struct SVertex
            {
                CVector Pos;
                CVector UV;
                CVector Normal;
                int Material;
            };

            void ClearCache();

            void AddFace(Mesh Mesh, CVector v1, CVector v2, CVector v3, CVector v4, CVector Normal, int Color, int Material);
            void AddFace(Mesh Mesh, SVertex v1, SVertex v2, SVertex v3);

            int AddVertex(Mesh Mesh, CVector Vertex);
            int AddNormal(Mesh Mesh, CVector Normal);
            int AddUV(Mesh Mesh, CVector UV);

            std::map<CVector, int> m_Index;
            std::map<CVector, int> m_NormalIndex;
            std::map<CVector, int> m_UVIndex;
            std::map<int, GroupedFaces> m_FacesIndex;

            Loader m_Loader;
    };

    using Mesher = std::shared_ptr<IMesher>;
} // namespace VoxelOptimizer


#endif //IMESHER_HPP