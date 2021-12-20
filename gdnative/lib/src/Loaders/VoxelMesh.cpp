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

#include <map>
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
    const CVector CVoxelMesh::CHUNK_SIZE = CVector(16, 16, 16);

    CVoxel::CVoxel()
    {
        Normals[Direction::UP] = FACE_UP;
        Normals[Direction::DOWN] = FACE_DOWN;
        Normals[Direction::LEFT] = FACE_LEFT;
        Normals[Direction::RIGHT] = FACE_RIGHT;
        Normals[Direction::FORWARD] = FACE_FORWARD;
        Normals[Direction::BACKWARD] = FACE_BACKWARD;
    }

    void CVoxelMesh::SetVoxel(const CVector &Pos, int Material, int Color, bool Transparent)
    {
        std::lock_guard<std::recursive_mutex> lock(m_Lock);
        Voxel Tmp = Voxel(new CVoxel());
        Tmp->Pos = Pos;
        Tmp->Material = Material;
        Tmp->Color = Color;
        Tmp->Transparent = Transparent;

        m_Voxels.insert({Pos, Tmp});

        SetNormal(Pos, CVoxel::FACE_UP);
        SetNormal(Pos, CVoxel::FACE_DOWN);

        SetNormal(Pos, CVoxel::FACE_LEFT);
        SetNormal(Pos, CVoxel::FACE_RIGHT);

        SetNormal(Pos, CVoxel::FACE_FORWARD);
        SetNormal(Pos, CVoxel::FACE_BACKWARD);

        m_BlockCount++;

        MarkChunk(Pos, Transparent ? Tmp : nullptr);
    }

    void CVoxelMesh::RemoveVoxel(const CVector &Pos)
    {
        std::lock_guard<std::recursive_mutex> lock(m_Lock);
        auto IT = m_Voxels.find(Pos);
        if(IT != m_Voxels.end())
        {
            SetNormal(Pos, CVoxel::FACE_UP, false);
            SetNormal(Pos, CVoxel::FACE_DOWN, false);

            SetNormal(Pos, CVoxel::FACE_LEFT, false);
            SetNormal(Pos, CVoxel::FACE_RIGHT, false);

            SetNormal(Pos, CVoxel::FACE_FORWARD, false);
            SetNormal(Pos, CVoxel::FACE_BACKWARD, false);

            m_Voxels.erase(IT);
            MarkChunk(Pos);
        }
    }
    
    void CVoxelMesh::Clear()
    {
        std::lock_guard<std::recursive_mutex> lock(m_Lock);
        //TODO: Fix for V-Edit
        // m_Voxels.clear();

        // if(m_RemeshAll)
        //     InsertMarkedChunk(m_BBox);
        // else
        // {
        //     for (auto &&c : m_Chunks)
        //         InsertMarkedChunk(c.second);
        // }
    }

    Voxel CVoxelMesh::GetVoxel(const CVector &Pos)
    {
        std::lock_guard<std::recursive_mutex> lock(m_Lock);
        auto IT = m_Voxels.find(Pos);
        if(IT == m_Voxels.end())
            return nullptr;

        return IT->second;
    }

    Voxel CVoxelMesh::GetVoxel(const CVector &Pos, bool OpaqueOnly)
    {
        auto voxel = GetVoxel(Pos);
        if(voxel)
        {
            if(OpaqueOnly && voxel->Transparent)
                voxel = nullptr;
            else if(!OpaqueOnly && !voxel->Transparent)
                voxel = nullptr;
        }

        return voxel;
    }

    void CVoxelMesh::SetNormal(const CVector &Pos, const CVector &Neighbor, bool IsInvisible)
    {
        static const std::map<CVector, std::pair<CVoxel::Direction, CVoxel::Direction>> NEIGHBOR_INDEX = {
            {CVoxel::FACE_UP, {CVoxel::Direction::UP, CVoxel::Direction::DOWN}},
            {CVoxel::FACE_DOWN, {CVoxel::Direction::DOWN, CVoxel::Direction::UP}},

            {CVoxel::FACE_LEFT, {CVoxel::Direction::LEFT, CVoxel::Direction::RIGHT}},
            {CVoxel::FACE_RIGHT, {CVoxel::Direction::RIGHT, CVoxel::Direction::LEFT}},

            {CVoxel::FACE_FORWARD, {CVoxel::Direction::FORWARD, CVoxel::Direction::BACKWARD}},
            {CVoxel::FACE_BACKWARD, {CVoxel::Direction::BACKWARD, CVoxel::Direction::FORWARD}},
        };

        Voxel cur = GetVoxel(Pos);
        if(cur == nullptr)
            return;

        Voxel neighbor = GetVoxel(Pos + Neighbor);
        auto directions = NEIGHBOR_INDEX.at(Neighbor);

        // 1. Both opaque touching faces invisible
        // 2. One transparent touching faces visible
        // 3. Both transparent different transparency faces visible
        // 4. Both transparent same transparency and color faces invisible
        // 5. Both transparent different transparency and same color faces visible

        if(neighbor)
        {
            bool hideFaces = false;

            if(!neighbor->Transparent && !cur->Transparent)
                hideFaces = true;
            else if(neighbor->Transparent && !cur->Transparent || !neighbor->Transparent && cur->Transparent)
                hideFaces = false;
            else if(neighbor->Transparent && cur->Transparent)
            {
                if(neighbor->Material != cur->Material)
                    hideFaces = false;
                else if(neighbor->Color != cur->Color)
                    hideFaces = false;
                else
                    hideFaces = true;
            }

            if(hideFaces)
            {
                neighbor->Normals[directions.second] = IsInvisible ? CVoxel::FACE_ZERO : (Neighbor * -1.f);
                cur->Normals[directions.first] = IsInvisible ? CVoxel::FACE_ZERO : Neighbor;
            }
            else
                cur->Normals[directions.first] = Neighbor;
        }
        else
            cur->Normals[directions.first] = Neighbor;
    }

    void CVoxelMesh::MarkChunk(const CVector &Pos, Voxel voxel)
    {
        auto ChunkPos = (Pos / CHUNK_SIZE).Floor() * CHUNK_SIZE;
        Chunk chunk;

        // Search for chunk to mark for remeshing.
        auto IT = m_Chunks.find(ChunkPos);
        if(IT != m_Chunks.end())
            chunk = IT->second;
        else
        {
            //Create new chunk
            CBBox BBox = CBBox(ChunkPos, ChunkPos + CHUNK_SIZE);
            chunk = Chunk(new SChunk());
            chunk->BBox = BBox;

            m_Chunks[ChunkPos] = chunk;
        }

        if(voxel)
        {
            std::vector<Chunk> chunks = {chunk, m_GlobalChunk};
            for (auto &&c : chunks)
            {         
                auto matColorVec = CVector(voxel->Color, voxel->Material, 0);
                auto tIT = c->Transparent.find(matColorVec);
                if(tIT != c->Transparent.end())
                {
                    tIT->second.Beg = tIT->second.Beg.Min(Pos);
                    tIT->second.End = tIT->second.End.Max(Pos + CVector(1, 1, 1));
                }
                else
                    c->Transparent[matColorVec] = CBBox(Pos, Pos);
            }
        }

        if(m_RemeshAll)
            chunk = m_GlobalChunk;

        InsertMarkedChunk(chunk);
    }

    void CVoxelMesh::InsertMarkedChunk(Chunk chunk)
    {
        auto IT = m_ChunksToRemesh.find(chunk->BBox.Beg);
        if(IT == m_ChunksToRemesh.end())
            m_ChunksToRemesh[chunk->BBox.Beg] = chunk;
    }
} // namespace VoxelOptimizer
