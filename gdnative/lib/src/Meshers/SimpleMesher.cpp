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
    Mesh CSimpleMesher::GenerateMesh(VoxelMesh m, Loader Loader)
    {
        Mesh Ret = Mesh(new SMesh());
        Ret->Texture = Loader->GetColorPalette();
        m_Loader = Loader;

        auto Voxels = m->GetVoxels();
        auto BBox = m->GetBBox();
        CVector Beg = BBox.Beg;
        std::swap(Beg.y, Beg.z);

        CVector BoxCenter = BBox.GetSize() / 2;
        std::swap(BoxCenter.y, BoxCenter.z);

        for (auto &&pair : Voxels)
        {          
            Voxel v = pair.second;

            if(v && v->IsVisible())
            {
                for (char i = 0; i < v->Normals.size(); i++)
                {
                    // Invisible
                    if(v->Normals[i] == CVector(0, 0, 0))
                        continue;

                    CVector v1, v2, v3, v4, Normal = v->Normals[i];
                    std::swap(Normal.y, Normal.z);
                    
                    switch (i)
                    {
                        case CVoxel::Direction::UP:
                        case CVoxel::Direction::DOWN:
                        {
                            float PosZ = v->Normals[i].z < 0 ? 0 : v->Normals[i].z;

                            v1 = CVector(v->Pos.x, v->Pos.z + PosZ, v->Pos.y + 1.f) - BoxCenter;
                            v2 = CVector(v->Pos.x, v->Pos.z + PosZ, v->Pos.y) - BoxCenter;
                            v3 = CVector(v->Pos.x + 1.f, v->Pos.z + PosZ, v->Pos.y) - BoxCenter;
                            v4 = CVector(v->Pos.x + 1.f, v->Pos.z + PosZ, v->Pos.y + 1.f) - BoxCenter;
                        }break;

                        case CVoxel::Direction::LEFT:
                        case CVoxel::Direction::RIGHT:
                        {
                            float Posx = v->Normals[i].x < 0 ? 0 : v->Normals[i].x;

                            v1 = CVector(v->Pos.x + Posx, v->Pos.z, v->Pos.y) - BoxCenter;
                            v2 = CVector(v->Pos.x + Posx, v->Pos.z, v->Pos.y + 1.f) - BoxCenter;
                            v3 = CVector(v->Pos.x + Posx, v->Pos.z + 1.f, v->Pos.y + 1.f) - BoxCenter;
                            v4 = CVector(v->Pos.x + Posx, v->Pos.z + 1.f, v->Pos.y) - BoxCenter;
                        }break;

                        case CVoxel::Direction::FORWARD:
                        case CVoxel::Direction::BACKWARD:
                        {
                            float PosY = v->Normals[i].y < 0 ? 0 : v->Normals[i].y;

                            v1 = CVector(v->Pos.x, v->Pos.z + 1.f, v->Pos.y + PosY) - BoxCenter;
                            v2 = CVector(v->Pos.x, v->Pos.z, v->Pos.y + PosY) - BoxCenter;
                            v3 = CVector(v->Pos.x + 1.f, v->Pos.z, v->Pos.y + PosY) - BoxCenter;
                            v4 = CVector(v->Pos.x + 1.f, v->Pos.z + 1.f, v->Pos.y + PosY) - BoxCenter;
                        }break;
                    }

                    AddFace(Ret, v1 - Beg, v2 - Beg, v3 - Beg, v4 - Beg, Normal, v->Color, v->Material);
                }
            }
        }

        ClearCache();
        return Ret;
    }
} // namespace VoxelOptimizer
