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

#include <VoxelOptimizer/Loaders/VoxelMesh.hpp>

namespace VoxelOptimizer
{
    const CVector CVoxel::FACE_UP = CVector(0, 0, 1);
    const CVector CVoxel::FACE_DOWN = CVector(0, 0, -1);
    const CVector CVoxel::FACE_LEFT = CVector(-1, 0, 0);
    const CVector CVoxel::FACE_RIGHT = CVector(1, 0, 0);
    const CVector CVoxel::FACE_FORWARD = CVector(0, 1, 0);
    const CVector CVoxel::FACE_BACKWARD = CVector(0, -1, 0);
    const CVector CVoxel::FACE_ZERO = CVector(0, 0, 0);

    CVoxel::CVoxel()
    {
        Normals[Direction::UP] = FACE_UP;
        Normals[Direction::DOWN] = FACE_DOWN;
        Normals[Direction::LEFT] = FACE_LEFT;
        Normals[Direction::RIGHT] = FACE_RIGHT;
        Normals[Direction::FORWARD] = FACE_FORWARD;
        Normals[Direction::BACKWARD] = FACE_BACKWARD;
    }

    void CVoxelModel::SetVoxel(CVector Pos, int Material)
    {
        Voxel Tmp = Voxel(new CVoxel());
        Tmp->Pos = Pos;
        Tmp->Material = Material;

        Voxel Up, Down, Left, Right, Forward, Backward;

        if(Pos.z + 1 != m_Size.z)
            Up = m_Voxels[Pos.x + m_Size.x * Pos.y + m_Size.x * m_Size.y * (Pos.z + 1)];

        if(Pos.z != 0)
            Down = m_Voxels[Pos.x + m_Size.x * Pos.y + m_Size.x * m_Size.y * (Pos.z - 1)];

        if(Pos.x != 0)
            Left = m_Voxels[(Pos.x - 1) + m_Size.x * Pos.y + m_Size.x * m_Size.y * Pos.z];

        if(Pos.x + 1 != m_Size.x)
            Right = m_Voxels[(Pos.x + 1) + m_Size.x * Pos.y + m_Size.x * m_Size.y * Pos.z];

        if(Pos.y + 1 != m_Size.y)
            Forward = m_Voxels[Pos.x + m_Size.x * (Pos.y + 1) + m_Size.x * m_Size.y * Pos.z];

        if(Pos.y != 0)
            Backward = m_Voxels[Pos.x + m_Size.x * (Pos.y - 1) + m_Size.x * m_Size.y * Pos.z];

        SetNormal(Tmp, Up, CVoxel::Direction::UP, CVoxel::Direction::DOWN, CVoxel::FACE_UP);
        SetNormal(Tmp, Down, CVoxel::Direction::DOWN, CVoxel::Direction::UP, CVoxel::FACE_DOWN);

        SetNormal(Tmp, Left, CVoxel::Direction::LEFT, CVoxel::Direction::RIGHT, CVoxel::FACE_LEFT);
        SetNormal(Tmp, Right, CVoxel::Direction::RIGHT, CVoxel::Direction::LEFT, CVoxel::FACE_RIGHT);

        SetNormal(Tmp, Forward, CVoxel::Direction::FORWARD, CVoxel::Direction::BACKWARD, CVoxel::FACE_FORWARD);
        SetNormal(Tmp, Backward, CVoxel::Direction::BACKWARD, CVoxel::Direction::FORWARD, CVoxel::FACE_BACKWARD);

        m_Voxels[Pos.x + m_Size.x * Pos.y + m_Size.x * m_Size.y * Pos.z] = Tmp;
    }

    Voxel CVoxelModel::GetVoxel(CVector Pos)
    {
        size_t ArrPos = (size_t)Pos.x + (size_t)m_Size.x * (size_t)Pos.y + (size_t)m_Size.x * (size_t)m_Size.y * (size_t)Pos.z;

        if(ArrPos > m_Voxels.size() - 1)
            return nullptr;

        return m_Voxels[ArrPos];
    }
    
    void CVoxelModel::SetNormal(Voxel Cur, Voxel Neighbor, CVoxel::Direction CurDir, CVoxel::Direction NeighborDir, CVector Val)
    {
        if(Neighbor)
        {
            Neighbor->Normals[NeighborDir] = CVoxel::FACE_ZERO;
            Cur->Normals[CurDir] = CVoxel::FACE_ZERO;
        }
        else
            Cur->Normals[CurDir] = Val;
    }
} // namespace VoxelOptimizer
