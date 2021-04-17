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

#include <VoxelOptimizer/Meshers/GreedyMesher.hpp>

namespace VoxelOptimizer
{
    Mesh CGreedyMesher::GenerateMesh(VoxelModel m, CMagicaVoxelLoader::ColorPalette Palette)
    {
        Mesh Ret = Mesh(new SMesh());

        auto Size = m->GetSize();

        // For all 3 axis (x, y, z)
        for (size_t Axis = 0; Axis < 3; Axis++)
        {
            int Axis1 = (Axis + 1) % 3; // 1 = 1 = y, 2 = 2 = z, 3 = 0 = x
            int Axis2 = (Axis + 2) % 3; // 2 = 2 = z, 3 = 0 = x, 4 = 1 = y
            int x[3] = {0};
            int q[3] = {0};

            /**
             * x000
             * 0xx0
             * 0xx0
             * 0000
             */

            // 2d plane to project a slice to.
            bool *mask = new bool[(size_t)Size.v[Axis1] * (size_t)Size.v[Axis2]];
            q[Axis] = 1;

            for (x[Axis] = -1; x[Axis] < Size.v[Axis];)
            {
                size_t n = 0;
                for (x[Axis2] = 0; x[Axis2] < Size.v[Axis2]; ++x[Axis2])
                {
                    for (x[Axis1] = 0; x[Axis1] < Size.v[Axis1]; ++x[Axis1])
                    {       
                        Voxel V1, V2;

                        V1 = m->GetVoxel(CVector(x[0], x[1], x[2]));
                        V2 = m->GetVoxel(CVector(x[0] + q[0], x[1] + q[1], x[2] + q[2]));

                        // if(x[Axis] >= 0)
                        //     V1 = Voxels[x[0] + (int)Size.x * x[1] + (int)Size.x * (int)Size.y * x[2]];

                        // if(x[Axis] < Size.v[Axis] - 1)
                        //     V2 = Voxels[x[0] + q[0] + (int)Size.x * (x[1] + q[1]) + (int)Size.x * (int)Size.y * (x[2] + q[2])];
                        
                        bool blockCurrent = 0 <= x[Axis] ? (V1 && V1->IsVisible()) : false;
                        bool blockCompare = x[Axis] < Size.v[Axis] - 1 ? (V2 && V2->IsVisible()) : false;

                        mask[n++] = blockCurrent != blockCompare;
                    }
                }

                ++x[Axis];
                n = 0;

                for (int HeightAxis = 0; HeightAxis < Size.v[Axis2]; ++HeightAxis)
                {
                    for (int WidthAxis = 0; WidthAxis < Size.v[Axis1];)
                    {
                        if(mask[n])
                        {
                            int w, h;

                            //Claculates the width of the rect.
                            for (w = 1; WidthAxis + w < Size.v[Axis1] && mask[n + w]; w++) { }

                            bool done = false;
                            for (h = 1; HeightAxis + h < Size.v[Axis2]; h++)
                            {
                                // Check each block next to this quad
                                for (int k = 0; k < w; ++k)
                                {
                                    // If there's a hole in the mask, exit
                                    if (!mask[n + k + h * (size_t)Size.v[Axis1]])
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

                            I1 = AddVertex(Ret, CVector(x[0], x[1], x[2]));
                            I2 = AddVertex(Ret, CVector(x[0] + du[0], x[1] + du[1], x[2] + du[2]));
                            I3 = AddVertex(Ret, CVector(x[0] + du[0] + dv[0], x[1] + du[1] + dv[1], x[2] + du[2] + dv[2]));
                            I4 = AddVertex(Ret, CVector(x[0] + dv[0], x[1] + dv[1], x[2] + dv[2]));

                            GroupedFaces Faces;

                            auto ITFaces = m_FacesIndex.find(1);
                            if(ITFaces == m_FacesIndex.end())
                            {
                                Faces = GroupedFaces(new SGroupedFaces());
                                Ret->Faces.push_back(Faces);

                                Faces->Material.Diffuse = Palette[1];
                                m_FacesIndex.insert({1, Faces});
                            }
                            else
                                Faces = ITFaces->second;

                            Faces->Indices.push_back(CVector(I1, 0, 0));
                            Faces->Indices.push_back(CVector(I2, 0, 0));
                            Faces->Indices.push_back(CVector(I3, 0, 0));

                            Faces->Indices.push_back(CVector(I1, 0, 0));
                            Faces->Indices.push_back(CVector(I3, 0, 0));
                            Faces->Indices.push_back(CVector(I4, 0, 0));

                            for (int l = 0; l < h; ++l)
                                for (int k = 0; k < w; ++k)
                                    mask[n + k + l * (size_t)Size.v[Axis1]] = false;

                            // Increment counters and continue
                            WidthAxis += w;
                            n += w;
                        }
                        else
                        {
                            WidthAxis++;
                            n++;
                        }
                    }
                }
            }

            delete[] mask;
        }

        return Ret;
    }
} // namespace VoxelOptimizer
