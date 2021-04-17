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
#include <memory>
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
            int Material;

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

    class CVoxelModel
    {
        public:
            CVoxelModel() = default;

            /**
             * @brief Sets the size of the voxel space.
             */
            inline void SetSize(CVector Size)
            {
                m_Size = Size;

                m_Voxels.clear();
                m_Voxels.resize(m_Size.x * m_Size.y * m_Size.z);
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
             * List of voxels. The size of the list is always the size of the voxel space.
             * Voxels which are null are empty space.
             * 
             * @return Returns the list of voxels.
             */
            inline std::vector<Voxel> GetVoxels() const
            {
                return m_Voxels;
            }

            /**
             * @brief Sets a voxel with an given material index.
             * 
             * @param Pos: The position of the voxel inside the voxel space.
             * @param Material: Material index of the voxels material.
             */
            void SetVoxel(CVector Pos, int Material);

            /**
             * @return Gets a voxel on a given position.
             */
            Voxel GetVoxel(CVector Pos);

            ~CVoxelModel() = default;
        private:
            void SetNormal(Voxel Cur, Voxel Neighbor, CVoxel::Direction CurDir, CVoxel::Direction NeighborDir, CVector Val);

            CVector m_Size;
            std::vector<Voxel> m_Voxels;
    };

    using VoxelModel = std::shared_ptr<CVoxelModel>;
} // namespace VoxelOptimizer


#endif //VOXELMESH_HPP