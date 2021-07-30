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

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <VoxelOptimizer/Loaders/GoxelLoader.hpp>
#include <string.h>
#include <VoxelOptimizer/Exceptions.hpp>

#include <fstream>
#include <string>

#include <iostream>

namespace VoxelOptimizer
{
    void CGoxelLoader::ParseFormat()
    {
        m_Models.clear();
        m_Materials.clear();
        m_UsedColorPalette.clear();

        ReadFile();
        
        VoxelMesh m = VoxelMesh(new CVoxelMesh());
        m->SetSize(m_BBox.End + m_BBox.Beg.Abs());

        std::map<int, int> ColorIdx;
        CVector Beg, End;
        
        for (auto &&l : m_Layers)
        {
            for (auto &&b : l.Blocks)
            {
                CVector v = b.Pos;
                BL16 &tmp = m_BL16s[b.Index];

                for (int z = v.z; z < v.z + 16; z++)
                {
                    for (int y = v.y; y < v.y + 16; y++)
                    {
                        for (int x = v.x; x < v.x + 16; x++)
                        {
                            CVector vi(x, y, z);
                            vi += m_BBox.Beg.Abs();

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

                                if(Beg.IsZero())
                                    Beg = vi;

                                Beg = Beg.Min(vi);
                                End = End.Max(vi);

                                m->SetVoxel(vi, l.MatIdx, IdxC, false);
                            }
                        }
                    }
                }
            }
            
        }

        m->SetBBox(CBBox(Beg, End));
        m_Models.push_back(m);

        m_BBox = CBBox();
        m_BL16s.clear();
        m_Layers.clear();
    }

    void CGoxelLoader::ReadFile()
    {
        std::string Signature(4, '\0');
        ReadData(&Signature[0], 4);
        Signature += "\0";

        // Checks the file header
        if(Signature != "GOX ")
            throw CVoxelLoaderException("Unknown file format");

        int Version = ReadData<int>();
        if(Version != 2)
            throw CVoxelLoaderException("Version: " + std::to_string(Version) + " is not supported");

        while (!IsEof())
        {
            SChunkHeader Chunk = ReadData<SChunkHeader>();

            if(strncmp(Chunk.Type, "BL16", sizeof(Chunk.Type)) == 0)
                ProcessBL16(Chunk);
            else if(strncmp(Chunk.Type, "LAYR", sizeof(Chunk.Type)) == 0)
                ProcessLayer(Chunk);
            else if(strncmp(Chunk.Type, "MATE", sizeof(Chunk.Type)) == 0)
                ProcessMaterial(Chunk);
            else
                Skip(Chunk.Size + sizeof(int));
        }
    }

    void CGoxelLoader::ProcessMaterial(const SChunkHeader &Chunk)
    {
        auto Dict = ReadDict(Chunk, Tellg());
        m_Materials.push_back(Material(new CMaterial()));

        float c[4];
        memcpy(c, Dict["color"].data(), 4 * sizeof(float));

        m_Materials.back()->Transparency = c[3];
        m_Materials.back()->Metallic = *((float*)(Dict["metallic"]).data());
        m_Materials.back()->Roughness = *((float*)(Dict["roughness"]).data());
        m_Materials.back()->Power = *((float*)(Dict["emission"]).data());

        Skip(sizeof(int));
    }

    void CGoxelLoader::ProcessLayer(const SChunkHeader &Chunk)
    {
        Layer l;
        size_t StartPos = Tellg();

        int Blocks = ReadData<int>();

        for (size_t i = 0; i < Blocks; i++)
        {
            Block B;

            B.Index = ReadData<int>();
            B.Pos.x = ReadData<int>();
            B.Pos.y = ReadData<int>();
            B.Pos.z = ReadData<int>();

            CVector v = B.Pos + CVector(16, 16, 16);

            m_BBox.Beg = m_BBox.Beg.Min(B.Pos);
            m_BBox.End = m_BBox.End.Max(v);

            Skip(sizeof(int));

            l.Blocks.push_back(B);
        }

        auto Dict = ReadDict(Chunk, StartPos);
        l.MatIdx = *((int*)(Dict["material"].data()));
        
        m_Layers.push_back(l);
        Skip(sizeof(int));
    }

    void CGoxelLoader::ProcessBL16(const SChunkHeader &Chunk)
    {
        stbi_uc *PngData = new stbi_uc[Chunk.Size];
        ReadData((char*)PngData, Chunk.Size);

        int w, h, c;
        uint32_t *ImgData = (uint32_t*)stbi_load_from_memory(PngData, Chunk.Size, &w, &h, &c, 4);
        m_BL16s.emplace_back();
        m_BL16s.back().SetData(ImgData);
        delete[] ImgData;
        delete[] PngData;

        Skip(sizeof(int));
    }

    std::map<std::string, std::string> CGoxelLoader::ReadDict(const SChunkHeader &Chunk, size_t StartPos)
    {
        std::map<std::string, std::string> Ret;

        while (Tellg() - StartPos < Chunk.Size)
        {
            int Size = ReadData<int>();
            std::string Key(Size, '\0');
            ReadData(&Key[0], Size);
            // Key += '\0';

            Size = ReadData<int>();
            std::string Value(Size, '\0');
            ReadData(&Value[0], Size);

            Ret[Key] = Value;
        }

        return Ret;
    }
} // namespace VoxelOptimizer
