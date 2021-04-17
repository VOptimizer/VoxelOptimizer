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

#include <voxeloptimizer/SimpleMesher.hpp>
#include <algorithm>

namespace VoxelOptimizer
{
    SimpleMesh CSimpleMesher::GenerateMesh(CVoxelLoader::Model m, CVoxelLoader::ColorPalette Palette)
    {
        SimpleMesh Ret = SimpleMesh(new SSimpleMesh());

        auto Voxels = m->GetVoxels();
        for (auto &&v : Voxels)
        {          
            if(v && v->IsVisible())
            {
                for (char i = 0; i < v->Normals.size(); i++)
                {
                    // Invisible
                    if(v->Normals[i] == CVector(0, 0, 0))
                        continue;

                    int I1, I2, I3, I4;
                    CVector v1, v2, v3, v4, N = v->Normals[i];

                    switch (i)
                    {
                        case CVoxelLoader::CVoxel::Direction::UP:
                        case CVoxelLoader::CVoxel::Direction::DOWN:
                        {
                            v1 = CVector(v->Pos.x - 0.5, v->Pos.z + v->Normals[i].z / 2.0, v->Pos.y + 0.5);
                            v2 = CVector(v->Pos.x - 0.5, v->Pos.z + v->Normals[i].z / 2.0, v->Pos.y - 0.5);
                            v3 = CVector(v->Pos.x + 0.5, v->Pos.z + v->Normals[i].z / 2.0, v->Pos.y - 0.5);
                            v4 = CVector(v->Pos.x + 0.5, v->Pos.z + v->Normals[i].z / 2.0, v->Pos.y + 0.5);

                            N.y = N.z;
                            N.z = 0;

                            I1 = AddVertex(Ret, v1);
                            I2 = AddVertex(Ret, v2);
                            I3 = AddVertex(Ret, v3);
                            I4 = AddVertex(Ret, v4);
                        }break;

                        case CVoxelLoader::CVoxel::Direction::LEFT:
                        case CVoxelLoader::CVoxel::Direction::RIGHT:
                        {
                            v1 = CVector(v->Pos.x + v->Normals[i].x / 2.0, v->Pos.z - 0.5, v->Pos.y - 0.5);
                            v2 = CVector(v->Pos.x + v->Normals[i].x / 2.0, v->Pos.z - 0.5, v->Pos.y + 0.5);
                            v3 = CVector(v->Pos.x + v->Normals[i].x / 2.0, v->Pos.z + 0.5, v->Pos.y + 0.5);
                            v4 = CVector(v->Pos.x + v->Normals[i].x / 2.0, v->Pos.z + 0.5, v->Pos.y - 0.5);

                            I1 = AddVertex(Ret, v1);
                            I2 = AddVertex(Ret, v2);
                            I3 = AddVertex(Ret, v3);
                            I4 = AddVertex(Ret, v4);
                        }break;

                        case CVoxelLoader::CVoxel::Direction::FORWARD:
                        case CVoxelLoader::CVoxel::Direction::BACKWARD:
                        {
                            v1 = CVector(v->Pos.x - 0.5, v->Pos.z + 0.5, v->Pos.y + v->Normals[i].y / 2.0);
                            v2 = CVector(v->Pos.x - 0.5, v->Pos.z - 0.5, v->Pos.y + v->Normals[i].y / 2.0);
                            v3 = CVector(v->Pos.x + 0.5, v->Pos.z - 0.5, v->Pos.y + v->Normals[i].y / 2.0);
                            v4 = CVector(v->Pos.x + 0.5, v->Pos.z + 0.5, v->Pos.y + v->Normals[i].y / 2.0);

                            I1 = AddVertex(Ret, v1);
                            I2 = AddVertex(Ret, v2);
                            I3 = AddVertex(Ret, v3);
                            I4 = AddVertex(Ret, v4);
                        }break;
                    }

                    GroupedFaces Faces;

                    auto ITFaces = m_FacesIndex.find(v->Material);
                    if(ITFaces == m_FacesIndex.end())
                    {
                        Faces = GroupedFaces(new SGroupedFaces());
                        Ret->Faces.push_back(Faces);

                        Faces->Material.Diffuse = Palette[v->Material - 1];
                        m_FacesIndex.insert({v->Material, Faces});
                    }
                    else
                        Faces = ITFaces->second;
                        

                    CVector FaceNormal = (v2 - v1).Cross(v3 - v1).Normalize(); //(v1 * v2).Cross(v1 * v3).Normalize();
                    CVector Centroid((v1.x + v2.x + v3.x) / 3.f, (v1.y + v2.y + v3.y) / 3.f, (v1.z + v2.z + v3.z) / 3.f);
                    float FaceDir = FaceNormal.Dot((v2 - Centroid).Normalize());

                    if(FaceNormal == N)
                    {
                        Faces->Indices.push_back(I1);
                        Faces->Indices.push_back(I2);
                        Faces->Indices.push_back(I3);

                        Faces->Indices.push_back(I1);
                        Faces->Indices.push_back(I3);
                        Faces->Indices.push_back(I4);
                    }
                    else
                    {
                        Faces->Indices.push_back(I3);
                        Faces->Indices.push_back(I2);
                        Faces->Indices.push_back(I1);

                        Faces->Indices.push_back(I4);
                        Faces->Indices.push_back(I3);
                        Faces->Indices.push_back(I1);
                    }
                }
            }
        }

        m_Index.clear();
        m_FacesIndex.clear();
        return Ret;
    }

    int CSimpleMesher::AddVertex(SimpleMesh Mesh, CVector Vertex)
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
} // namespace VoxelOptimizer
