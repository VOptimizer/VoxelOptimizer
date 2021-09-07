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

#ifndef VOXELMESH_HPP
#define VOXELMESH_HPP

#include <array>
#include <VoxelOptimizer/BBox.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <VoxelOptimizer/Vector.hpp>

namespace VoxelOptimizer
{
    class CVoxel
    {
        public:
            enum Direction
            {
                UP,
                DOWN,
                LEFT,
                RIGHT,
                FORWARD,
                BACKWARD
            };

            // Normal face directions inside the voxel space.
            const static CVector FACE_UP; // (0, 0, 1);
            const static CVector FACE_DOWN; // (0, 0, -1);
            const static CVector FACE_LEFT; // (-1, 0, 0);
            const static CVector FACE_RIGHT; // (1, 0, 0);
            const static CVector FACE_FORWARD; // (0, 1, 0);
            const static CVector FACE_BACKWARD; // (0, -1, 0);
            const static CVector FACE_ZERO; // (0, 0, 0);

            CVoxel();

            CVector Pos;
            int Material;   //!< Index of the material.
            int Color;      //!< Index of the color.
            bool Transparent;

            // A normal of (0, 0, 0) means invisible face.
            std::array<CVector, 6> Normals;

            /**
             * @return Returns true if at least one normal is not (0, 0, 0).
             */
            inline bool IsVisible() const
            {
                for (auto &&n : Normals)
                {
                    if(!n.IsZero())
                        return true;
                }
                
                return false;
            }

            ~CVoxel() = default;
    };

    using Voxel = std::shared_ptr<CVoxel>;

    class CVoxelMesh
    {
        public:
            CVoxelMesh() : m_RemeshAll(false), m_BlockCount(0) { }

            /**
             * @brief Sets the size of the voxel space.
             */
            inline void SetSize(const CVector &Size)
            {
                m_Size = Size;

                // m_Voxels.clear();
                // m_Voxels.resize(m_Size.x * m_Size.y * m_Size.z);
            }

            /**
             * The voxel space begins always at (0, 0, 0) which is the bottom, front left corner.
             * The z axis is the Up axis and y the forward axis.
             * 
             * @return Returns the voxel space size of a voxel mesh.
             */
            inline CVector GetSize() const
            {
                return m_Size;
            }

            /**
             * @brief Gets the bounding box.
             */
            inline CBBox GetBBox() const
            {
                return m_BBox;
            }
            
            /**
             * @brief Sets the bounding box.
             */
            inline void SetBBox(CBBox BBox)
            {
                m_BBox = BBox;
            }
            
            /**
             * List of voxels. The size of the list is always the size of the voxel space.
             * Voxels which are null are empty space.
             * 
             * @return Returns the list of voxels.
             */
            inline std::map<CVector, Voxel> GetVoxels() const
            {
                std::lock_guard<std::recursive_mutex> lock(m_Lock);
                return m_Voxels;
            }

            inline std::vector<CBBox> GetChunksToRemesh()
            {
                std::lock_guard<std::recursive_mutex> lock(m_Lock);
                std::vector<CBBox> Ret(m_ChunksToRemesh.size(), CBBox());
                size_t Pos = 0;

                for (auto &&c : m_ChunksToRemesh)
                {
                    Ret[Pos] = c.second;
                    Pos++;
                }

                m_ChunksToRemesh.clear();

                return Ret;
            }

            /**
             * @brief Sets a voxel with an given material index.
             * 
             * @param Pos: The position of the voxel inside the voxel space.
             * @param Material: Material index of the voxels material.
             * @param Color: Color index.
             * @param Transparent: Is the block transparent?
             */
            void SetVoxel(const CVector &Pos, int Material, int Color, bool Transparent);

            /**
             * @brief Removes a voxel on a given position
             * 
             * @param Pos: Position of the voxel to remove.
             */
            void RemoveVoxel(const CVector &Pos);

            /**
             * @brief Clears the mesh
             */
            void Clear();

            /**
             * @return Gets a voxel on a given position.
             */
            Voxel GetVoxel(const CVector &Pos);

            /**
             * @brief Gets the count of all setted blocks.
             */
            inline size_t GetBlockCount() const
            {
                return m_BlockCount;
            }

            /**
             * @brief If true, the mesh will be always completely remeshed, regardless of the chunks.
             */
            inline void RemeshAlways(bool val)
            {
                m_RemeshAll = val;
            }
            
            ~CVoxelMesh() = default;
        private:   
            const static CVector CHUNK_SIZE;

            void SetNormal(const CVector &Pos, const CVector &Neighbor, bool IsInvisible = true);
            void MarkChunk(const CVector &Pos);
            void InsertMarkedChunk(const CBBox &BBox);

            CVector m_Size;
            CBBox m_BBox;
            std::map<CVector, Voxel> m_Voxels;
            std::map<CVector, CBBox> m_Chunks;
            std::map<CVector, CBBox> m_ChunksToRemesh;

            bool m_RemeshAll;
            size_t m_BlockCount;
            mutable std::recursive_mutex m_Lock;
    };

    using VoxelMesh = std::shared_ptr<CVoxelMesh>;
} // namespace VoxelOptimizer


#endif //VOXELMESH_HPP