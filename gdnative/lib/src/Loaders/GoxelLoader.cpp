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

#include <VoxelOptimizer/Loaders/GoxelLoader.hpp>
#include <string.h>
#include <VoxelOptimizer/Exceptions.hpp>

#include <fstream>
#include <string>

namespace VoxelOptimizer
{
    void CGoxelLoader::Load(const char *Data, size_t Length)
    {
        m_Models.clear();
        m_Materials.clear();
        m_UsedColorPalette.clear();

        size_t Pos = 0;
        if(Length < 8)
            throw CVoxelLoaderException("File too short to be a voxel file");
        
        std::string Signature(4, '\0');
        memcpy(&Signature[0], Data, 4);
        Signature += "\0";

        // Checks the file header
        if(Signature != "GOX ")
            throw CVoxelLoaderException("Unknown file format");

        int Version = *((int*)(Data + 4));
        if(Version != 2)
            throw CVoxelLoaderException("Version: " + std::to_string(Version) + " is not supported");

        Pos += 8;
        VoxelMesh m = VoxelMesh(new CVoxelMesh());

        while (Pos < Length)
        {
            if(Pos + sizeof(SChunkHeader) > Length)
                break;

            SChunkHeader Chunk = LoadChunk(Data, Pos);

            // if(strncmp(Chunk.Type, "BL16", sizeof(Chunk.Type)) == 0)
            // {
            //     std::ofstream out(std::to_string(Pos) + ".png", std::ios::binary);
            //     if(out.is_open())
            //     {
            //         out.write(Data + Pos, Chunk.Size);
            //         out.close();
            //     }

            //     Pos += Chunk.Size + sizeof(int);
            // }
            /*if(strncmp(Chunk.Type, "IMG ", sizeof(Chunk.Type)) == 0)
            {
                Pos += sizeof(int) + 3 + sizeof(int);
                float Box[16];

                memcpy(Box, Data + Pos, 64 * sizeof(float));

                Pos += Chunk.Size + sizeof(int);
            }
            else*/ if(strncmp(Chunk.Type, "LAYR", sizeof(Chunk.Type)) == 0)
            {
                int Blocks = *((int*)(Data + Pos));
                Pos += sizeof(int);

                for (size_t i = 0; i < Blocks; i++)
                {
                    Pos += sizeof(int);
                    CVector v;

                    v.x = *((int*)(Data + Pos));
                    Pos += sizeof(int);

                    v.y = *((int*)(Data + Pos));
                    Pos += sizeof(int);

                    v.z = *((int*)(Data + Pos));
                    Pos += sizeof(int);
                    Pos += sizeof(int);

                    if(v.IsZero() && m->GetSize().IsZero())
                        m->SetSize(CVector(16, 16, 16));

                    if(v > m->GetSize())
                        m->SetSize(v);

                    m->SetVoxel(v, 0, 0, false);
                }
                
                Pos += Chunk.Size - (Blocks * sizeof(int) * 5);
            }
            else
                Pos += Chunk.Size + sizeof(int);
        }

        m_Models.push_back(m);
    }

    CGoxelLoader::SChunkHeader CGoxelLoader::LoadChunk(const char *Data, size_t &Pos)
    {
        SChunkHeader Tmp;
        memcpy(&Tmp, Data + Pos, sizeof(SChunkHeader));
        Pos += sizeof(SChunkHeader);

        return Tmp;
    }
} // namespace VoxelOptimizer
