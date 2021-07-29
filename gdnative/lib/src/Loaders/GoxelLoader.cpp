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

#define STB_IMAGE_IMPLEMENTATION
// #define STBI_ONLY_PNG
#include "../stb_image.h"

#include <VoxelOptimizer/Loaders/GoxelLoader.hpp>
#include <string.h>
#include <VoxelOptimizer/Exceptions.hpp>

#include <fstream>
#include <string>

namespace VoxelOptimizer
{
    void CGoxelLoader::ParseFormat()
    {
        m_Models.clear();
        m_Materials.clear();
        m_UsedColorPalette.clear();
        
        std::string Signature(4, '\0');
        ReadData(&Signature[0], 4);
        Signature += "\0";

        // Checks the file header
        if(Signature != "GOX ")
            throw CVoxelLoaderException("Unknown file format");

        int Version = ReadData<int>();
        if(Version != 2)
            throw CVoxelLoaderException("Version: " + std::to_string(Version) + " is not supported");

        VoxelMesh m = VoxelMesh(new CVoxelMesh());
        std::vector<BL16> BL16s;
        std::vector<Layer> Layers;

        CVector MinSize, MaxSize;

        while (!IsEof())
        {
            SChunkHeader Chunk = ReadData<SChunkHeader>();

            if(strncmp(Chunk.Type, "BL16", sizeof(Chunk.Type)) == 0)
            {
                stbi_uc *PngData = new stbi_uc[Chunk.Size];
                ReadData((char*)PngData, Chunk.Size);

                int w, h, c;
                uint32_t *ImgData = (uint32_t*)stbi_load_from_memory(PngData, Chunk.Size, &w, &h, &c, 4);
                BL16s.emplace_back();
                BL16s.back().SetData(ImgData);
                delete[] ImgData;
                delete[] PngData;

                Skip(sizeof(int));
            }
            /*if(strncmp(Chunk.Type, "IMG ", sizeof(Chunk.Type)) == 0)
            {
                Pos += sizeof(int) + 3 + sizeof(int);
                float Box[16];

                memcpy(Box, Data + Pos, 64 * sizeof(float));

                Pos += Chunk.Size + sizeof(int);
            }*/
            else if(strncmp(Chunk.Type, "LAYR", sizeof(Chunk.Type)) == 0)
            {
                int Blocks = ReadData<int>();

                for (size_t i = 0; i < Blocks; i++)
                {
                    Layer l;

                    l.Index = ReadData<int>();
                    l.Pos.x = ReadData<int>();
                    l.Pos.y = ReadData<int>();
                    l.Pos.z = ReadData<int>();

                    CVector v = l.Pos + CVector(16, 16, 16);

                    MinSize = MinSize.Min(l.Pos);
                    MaxSize = MaxSize.Max(v);

                    Skip(sizeof(int));

                    Layers.push_back(l);

                    // if(m->GetSize().IsZero())
                    //     m->SetSize(CVector(64, 64, 64));

                    // for (size_t z = v.z; z < v.z + 16; z++)
                    // {
                    //     for (size_t y = v.y; y < v.y + 16; y++)
                    //     {
                    //         for (size_t x = v.x; x < v.x + 16; x++)
                    //         {
                    //             BL16 &tmp = BL16s[idx];

                    //             CVector vi(x, y, z);
                    //             uint32_t p = tmp.GetVoxel(CVector(x - v.x, y - v.y, z - v.z));

                    //             if(p != 0)
                    //             {
                    //                 m->SetVoxel(vi, 0, 0, false);
                    //             }
                    //         }
                    //     }
                    // }
                    

                    // if(v > m->GetSize())
                    //     m->SetSize(v);

                    // m->SetVoxel(v, 0, 0, false);
                }
                
                Skip(Chunk.Size - (Blocks * sizeof(int) * 5));
            }
            else
                Skip(Chunk.Size + sizeof(int));
        }

        m_Materials.push_back(Material(new CMaterial()));
        m->SetSize(MaxSize + MinSize.Abs());

        std::map<int, int> ColorIdx;
        
        for (auto &&l : Layers)
        {
            CVector v = l.Pos;
            for (int z = v.z; z < v.z + 16; z++)
            {
                for (int y = v.y; y < v.y + 16; y++)
                {
                    for (int x = v.x; x < v.x + 16; x++)
                    {
                        BL16 &tmp = BL16s[l.Index];

                        CVector vi(x, y, z);
                        uint32_t p = tmp.GetVoxel(CVector(x - v.x, y - v.y, z - v.z));

                        if(p != 0)
                        {
                            int IdxC = 0;
                            if(ColorIdx.find(p) == ColorIdx.end())
                            {
                                CColor c;
                                memcpy(c.c, &p, 4);

                                m_UsedColorPalette.push_back(c);
                                ColorIdx[p] = m_UsedColorPalette.size() - 1;
                                IdxC = m_UsedColorPalette.size() - 1;
                            }
                            else
                                IdxC = ColorIdx[p];

                            m->SetVoxel(vi + MinSize.Abs(), 0, IdxC, false);
                        }
                    }
                }
            }
        }

        m_Models.push_back(m);
    }
} // namespace VoxelOptimizer
