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

#include <VoxelOptimizer/Meshers/IMesher.hpp>

namespace VoxelOptimizer
{
    int IMesher::AddVertex(Mesh Mesh, CVector Vertex)
    {
        int Ret = 0;
        // auto End = Mesh->Vertices.data() + Mesh->Vertices.size();
        auto IT = m_Index.find(Vertex.hash()); //std::find(Mesh->Vertices.data(), End, Vertex);

        if(IT != m_Index.end())
            Ret = IT->second;
        else
        {
            Mesh->Vertices.push_back(Vertex);
            Ret = Mesh->Vertices.size();

            m_Index.insert({Vertex.hash(), Ret});
        }

        return Ret;
    }

    int IMesher::AddNormal(Mesh Mesh, CVector Normal)
    {
        int Ret = 0;
        // Normal = CVector(Normal.x, Normal.z, Normal.y);

        // auto End = Mesh->Vertices.data() + Mesh->Vertices.size();
        auto IT = m_NormalIndex.find(Normal.hash()); //std::find(Mesh->Vertices.data(), End, Vertex);

        if(IT != m_NormalIndex.end())
            Ret = IT->second;
        else
        {
            Mesh->Normals.push_back(Normal);
            Ret = Mesh->Normals.size();

            m_NormalIndex.insert({Normal.hash(), Ret});
        }

        return Ret;
    }
} // namespace VoxelOptimizer
