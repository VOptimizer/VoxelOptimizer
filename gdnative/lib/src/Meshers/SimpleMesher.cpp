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

#include <VoxelOptimizer/Meshers/SimpleMesher.hpp>
#include <algorithm>

namespace VoxelOptimizer
{
    Mesh CSimpleMesher::GenerateMesh(VoxelModel m, CMagicaVoxelLoader::ColorPalette Palette)
    {
        Mesh Ret = Mesh(new SMesh());

        auto Voxels = m->GetVoxels();
        CVector BoxCenter = m->GetSize() / 2;

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
                    CVector v1, v2, v3, v4, Normal = v->Normals[i];
                    std::swap(Normal.y, Normal.z);
                    
                    switch (i)
                    {
                        case CVoxel::Direction::UP:
                        case CVoxel::Direction::DOWN:
                        {
                            v1 = CVector(v->Pos.x - 0.5, v->Pos.z + v->Normals[i].z / 2.0, v->Pos.y + 0.5) - BoxCenter;
                            v2 = CVector(v->Pos.x - 0.5, v->Pos.z + v->Normals[i].z / 2.0, v->Pos.y - 0.5) - BoxCenter;
                            v3 = CVector(v->Pos.x + 0.5, v->Pos.z + v->Normals[i].z / 2.0, v->Pos.y - 0.5) - BoxCenter;
                            v4 = CVector(v->Pos.x + 0.5, v->Pos.z + v->Normals[i].z / 2.0, v->Pos.y + 0.5) - BoxCenter;

                            I1 = AddVertex(Ret, v1);
                            I2 = AddVertex(Ret, v2);
                            I3 = AddVertex(Ret, v3);
                            I4 = AddVertex(Ret, v4);
                        }break;

                        case CVoxel::Direction::LEFT:
                        case CVoxel::Direction::RIGHT:
                        {
                            v1 = CVector(v->Pos.x + v->Normals[i].x / 2.0, v->Pos.z - 0.5, v->Pos.y - 0.5) - BoxCenter;
                            v2 = CVector(v->Pos.x + v->Normals[i].x / 2.0, v->Pos.z - 0.5, v->Pos.y + 0.5) - BoxCenter;
                            v3 = CVector(v->Pos.x + v->Normals[i].x / 2.0, v->Pos.z + 0.5, v->Pos.y + 0.5) - BoxCenter;
                            v4 = CVector(v->Pos.x + v->Normals[i].x / 2.0, v->Pos.z + 0.5, v->Pos.y - 0.5) - BoxCenter;

                            I1 = AddVertex(Ret, v1);
                            I2 = AddVertex(Ret, v2);
                            I3 = AddVertex(Ret, v3);
                            I4 = AddVertex(Ret, v4);
                        }break;

                        case CVoxel::Direction::FORWARD:
                        case CVoxel::Direction::BACKWARD:
                        {
                            v1 = CVector(v->Pos.x - 0.5, v->Pos.z + 0.5, v->Pos.y + v->Normals[i].y / 2.0) - BoxCenter;
                            v2 = CVector(v->Pos.x - 0.5, v->Pos.z - 0.5, v->Pos.y + v->Normals[i].y / 2.0) - BoxCenter;
                            v3 = CVector(v->Pos.x + 0.5, v->Pos.z - 0.5, v->Pos.y + v->Normals[i].y / 2.0) - BoxCenter;
                            v4 = CVector(v->Pos.x + 0.5, v->Pos.z + 0.5, v->Pos.y + v->Normals[i].y / 2.0) - BoxCenter;

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
                        

                    CVector FaceNormal = (v2 - v1).Cross(v3 - v1).Normalize(); 
                    int NormalIdx = AddNormal(Ret, Normal);
                    
                    //(v1 * v2).Cross(v1 * v3).Normalize();
                    // CVector Centroid((v1.x + v2.x + v3.x) / 3.f, (v1.y + v2.y + v3.y) / 3.f, (v1.z + v2.z + v3.z) / 3.f);
                    // float FaceDir = FaceNormal.Dot((v2 - Centroid).Normalize());

                    if(FaceNormal == Normal)
                    {
                        Faces->Indices.push_back(CVector(I1, NormalIdx, 0));
                        Faces->Indices.push_back(CVector(I2, NormalIdx, 0));
                        Faces->Indices.push_back(CVector(I3, NormalIdx, 0));

                        Faces->Indices.push_back(CVector(I1, NormalIdx, 0));
                        Faces->Indices.push_back(CVector(I3, NormalIdx, 0));
                        Faces->Indices.push_back(CVector(I4, NormalIdx, 0));
                    }
                    else
                    {
                        Faces->Indices.push_back(CVector(I3, NormalIdx, 0));
                        Faces->Indices.push_back(CVector(I2, NormalIdx, 0));
                        Faces->Indices.push_back(CVector(I1, NormalIdx, 0));

                        Faces->Indices.push_back(CVector(I4, NormalIdx, 0));
                        Faces->Indices.push_back(CVector(I3, NormalIdx, 0));
                        Faces->Indices.push_back(CVector(I1, NormalIdx, 0));
                    }
                }
            }
        }

        m_Index.clear();
        m_FacesIndex.clear();
        return Ret;
    }
} // namespace VoxelOptimizer
