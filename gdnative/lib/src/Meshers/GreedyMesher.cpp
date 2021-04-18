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

#include "Slicer/Slicer.hpp"
#include <VoxelOptimizer/Meshers/GreedyMesher.hpp>

namespace VoxelOptimizer
{
    Mesh CGreedyMesher::GenerateMesh(VoxelMesh m, CMagicaVoxelLoader::ColorPalette Palette)
    {
        Mesh Ret = Mesh(new SMesh());

        auto Size = m->GetSize();
        CVector BoxCenter = m->GetSize() / 2;

        CSlicer Slicer(m);

        // For all 3 axis (x, y, z)
        for (size_t Axis = 0; Axis < 3; Axis++)
        {
            Slicer.SetActiveAxis(Axis);

            int Axis1 = (Axis + 1) % 3; // 1 = 1 = y, 2 = 2 = z, 3 = 0 = x
            int Axis2 = (Axis + 2) % 3; // 2 = 2 = z, 3 = 0 = x, 4 = 1 = y
            int x[3] = {0};

            // Iterate over each slice of the 3d model.
            for (x[Axis] = -1; x[Axis] < Size.v[Axis];)
            {
                ++x[Axis];

                // Foreach slice go over a 2d plane. 
                for (int HeightAxis = 0; HeightAxis < Size.v[Axis2]; ++HeightAxis)
                {
                    for (int WidthAxis = 0; WidthAxis < Size.v[Axis1];)
                    {
                        CVector Pos;
                        Pos.v[Axis] = x[Axis] - 1;
                        Pos.v[Axis1] = WidthAxis;
                        Pos.v[Axis2] = HeightAxis;

                        if(Slicer.IsFace(Pos))
                        {
                            int w, h;
                            CVector Normal = Slicer.Normal();
                            int Material = Slicer.Material();

                            //Claculates the width of the rect.
                            for (w = 1; WidthAxis + w < Size.v[Axis1]; w++) 
                            {
                                CVector WPos;
                                WPos.v[Axis1] = w;

                                bool IsFace = Slicer.IsFace(Pos + WPos);
                                if(IsFace)
                                {
                                    CVector FaceNormal = Slicer.Normal();
                                    IsFace = Normal == FaceNormal && Material == Slicer.Material();
                                }

                                if(!IsFace)
                                    break;
                            }

                            bool done = false;
                            for (h = 1; HeightAxis + h < Size.v[Axis2]; h++)
                            {
                                // Check each block next to this quad
                                for (int k = 0; k < w; ++k)
                                {
                                    CVector QuadPos = Pos;
                                    QuadPos.v[Axis1] += k;
                                    QuadPos.v[Axis2] += h;

                                    bool IsFace = Slicer.IsFace(QuadPos);
                                    if(IsFace)
                                    {
                                        CVector FaceNormal = Slicer.Normal();
                                        IsFace = Normal == FaceNormal && Material == Slicer.Material();
                                    }

                                    // If there's a hole in the mask, exit
                                    if (!IsFace)
                                    {
                                        done = true;
                                        break;
                                    }
                                }

                                if (done)
                                    break;
                            }

                            x[Axis1] = WidthAxis;
                            x[Axis2] = HeightAxis;

                            int du[3] = {0};
                            du[Axis1] = w;

                            int dv[3] = {0};
                            dv[Axis2] = h;

                            int I1, I2, I3, I4;

                            CVector v1 = CVector(x[0], x[2], x[1]) - BoxCenter;
                            CVector v2 = CVector(x[0] + du[0], x[2] + du[2], x[1] + du[1]) - BoxCenter;
                            CVector v3 = CVector(x[0] + du[0] + dv[0], x[2] + du[2] + dv[2], x[1] + du[1] + dv[1]) - BoxCenter;
                            CVector v4 = CVector(x[0] + dv[0], x[2] + dv[2], x[1] + dv[1]) - BoxCenter;

                            I1 = AddVertex(Ret, v1);
                            I2 = AddVertex(Ret, v2);
                            I3 = AddVertex(Ret, v3);
                            I4 = AddVertex(Ret, v4);

                            GroupedFaces Faces;

                            Material -= 1;
                            auto ITFaces = m_FacesIndex.find(Material);
                            if(ITFaces == m_FacesIndex.end())
                            {
                                Faces = GroupedFaces(new SGroupedFaces());
                                Ret->Faces.push_back(Faces);

                                Faces->Material.Diffuse = Palette[Material];
                                m_FacesIndex.insert({Material, Faces});
                            }
                            else
                                Faces = ITFaces->second;

                            CVector FaceNormal = (v2 - v1).Cross(v3 - v1).Normalize();
                            
                            std::swap(Normal.y, Normal.z);
                            int NormalIdx = AddNormal(Ret, Normal);

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

                            Slicer.AddProcessedQuad(CVector(x[0], x[1], x[2]), CVector(du[0] + dv[0], du[1] + dv[1], du[2] + dv[2]));

                            // Increment counters and continue
                            WidthAxis += w;
                        }
                        else
                            WidthAxis++;
                    }
                }

                Slicer.ClearQuads();
            }

            // break;
        }

        return Ret;
    }
} // namespace VoxelOptimizer
