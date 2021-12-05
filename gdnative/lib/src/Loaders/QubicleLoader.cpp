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

#include <VoxelOptimizer/Loaders/QubicleLoader.hpp>
#include <VoxelOptimizer/Exceptions.hpp>
#include <stb_image.h>
#include <string.h>

#include <fstream>

#include <iostream>
#include <iomanip>

using namespace std;

namespace VoxelOptimizer
{
    static int counter111 = 1;

    void CQubicleLoader::ParseFormat()
    {
        m_Models.clear();
        m_Materials.clear();
        m_Materials.clear();
        m_UsedColorPalette.clear();

        m_Materials.push_back(Material(new CMaterial()));

        std::string Signature(4, '\0');
        ReadData(&Signature[0], 4);
        Signature += "\0";

        // Checks the file header
        if(Signature != "QBCL")
            throw CVoxelLoaderException("Unknown file format");

        Skip(4);    // Program version e.g. 3.1.2.0

        int version = ReadData<int>();
        if(version != 2)   // Version 2.0
            throw CVoxelLoaderException("Unsupported version!");

        //Thumbnail size
        uint32_t width = ReadData<uint32_t>();
        uint32_t height = ReadData<uint32_t>();

        // Skips the BGRA encoded thumbnail.
        Skip(width * height * 4);

        // Metadata
        // Skips all the metadata. (Same order as in the gui)
        for (char i = 0; i < 7; i++)
        {
            uint32_t size = ReadData<uint32_t>();
            Skip(size);
        }

        Skip(16);   //Timestamp?
        LoadNode();
    }

    void CQubicleLoader::LoadNode()
    {
        uint32_t type = ReadData<uint32_t>();
        Skip(sizeof(int));  // I dont know.
        // uint32_t size = ReadData<uint32_t>();

        switch (type)
        {
            case 0: // Matrix
            {
                LoadMatrix();
            }break;

            case 1: // Model
            {
                LoadModel();
            }break;

            // case 2: // Compound
            // {
            //     LoadCompound();
            // }break;
        
            default:
            {
                throw CVoxelLoaderException("Unknown type: " + std::to_string(type));
                // Skip(size);
            } break;
        }
    }

    void CQubicleLoader::LoadModel()
    {
        uint32_t size = ReadData<uint32_t>();
        Skip(size);
        Skip(39);   // I dont know for which this chunk is for. It's always the same.

        uint32_t childCount = ReadData<uint32_t>();
        for (uint32_t i = 0; i < childCount; i++)
            LoadNode();        
    }

    void CQubicleLoader::LoadMatrix()
    {
        uint32_t size = ReadData<uint32_t>();
        Skip(size);
        Skip(3); //Mysterious 3 bytes always 0x01 0x01 0x00

        VoxelMesh mesh = VoxelMesh(new CVoxelMesh());
        mesh->SetSize(ReadVector());

        auto pos = ReadVector();
        auto halfSize = (mesh->GetSize() / 2.0);
        pos += halfSize;
        std::swap(pos.y, pos.z);

        mesh->SetModelMatrix(CMat4x4::Translation(pos));
        Skip(3 * sizeof(float));    //Pivot position.

        uint32_t dataSize = ReadData<uint32_t>();

        int OutSize = 0;
        char *Data = stbi_zlib_decode_malloc(Ptr(), dataSize, &OutSize);
        Skip(dataSize);
        int strmPos = 0;

        // if(counter111 == 3)
        // {
        //     std::ofstream out("voxel1.bin", std::ios::binary);
        //     if(out.is_open())
        //     {
        //         out.write(Data, OutSize);
        //         out.close();
        //     }
        // }

        // if(counter111 == 4)
        // {
        //     std::ofstream out("voxel2.bin", std::ios::binary);
        //     if(out.is_open())
        //     {
        //         out.write(Data, OutSize);
        //         out.close();
        //     }
        // }

        // counter111++;

        // int OutSize2 = 0;
        // char *Data2 = stbi_zlib_decode_noheader_malloc(Data, 50, &OutSize2);

        // for (size_t i = 0; i < 50; i += 4)
        // {
        //     cout << setfill('0') << setw(8) << hex << *((int*)(Data + i));
        //     cout << " " << dec << *((int*)(Data + i));
        //     cout << " (" << dec << *((short*)(Data + i));
        //     cout << ", " << dec << *((short*)(Data + i + 2)) << ")";
        //     cout << " " << dec << *((float*)(Data + i)) << endl;
        // }
        // cout << endl;

        CVector Beg(1000, 1000, 1000), End;
        uint32_t index = 0;

        while(strmPos < OutSize)
        {
            uint32_t z = 0; 
            uint16_t dataSize;

            memcpy(&dataSize, Data + strmPos, sizeof(uint16_t));
            strmPos += sizeof(uint16_t);
            
            for (size_t i = 0; i < dataSize; i++)
            {
                uint32_t data;
                memcpy(&data, Data + strmPos, sizeof(uint32_t));
                strmPos += sizeof(uint32_t);

                CColor c;
                c.FromRGBA(data);
                if(c.A == 2)    //RLE
                {
                    memcpy(&data, Data + strmPos, sizeof(uint32_t));
                    strmPos += sizeof(uint32_t);

                    for (uint8_t j = 0; j < c.R; j++)
                    {
                        CVector pos;

                        pos.y = index % (uint32_t)mesh->GetSize().y;
                        pos.x = (uint32_t)(index / (uint32_t)mesh->GetSize().y);
                        pos.z = z;
                        
                        AddVoxel(mesh, data, pos, Beg, End);
                        z++;
                    }
                    
                    i++;
                }
                else
                {
                    CVector pos;

                    pos.y = index % (uint32_t)mesh->GetSize().y;
                    pos.x = (uint32_t)(index / (uint32_t)mesh->GetSize().y);
                    pos.z = z;
                    
                    AddVoxel(mesh, data, pos, Beg, End);
                    z++;
                }
            }

            index++;
        }
        free(Data);

        mesh->SetBBox(CBBox(Beg, End));
        m_Models.push_back(mesh);
    }

    void CQubicleLoader::LoadCompound()
    {

    }

    CVector CQubicleLoader::ReadVector()
    {
        CVector ret;

        ret.x = ReadData<int>();
        ret.z = ReadData<int>();
        ret.y = ReadData<int>();

        return ret;
    }

    int CQubicleLoader::GetColorIdx(int color)
    {
        int ret = 0;

        auto IT = m_ColorIdx.find(color);
        if(IT == m_ColorIdx.end())
        {
            CColor c;
            c.FromRGBA(color);

            if(c.A == 0)
                return -1;

            m_UsedColorPalette.push_back(c);
            ret = m_UsedColorPalette.size() - 1;

            m_ColorIdx.insert({color, ret});
        }
        else
            ret = IT->second;

        return ret;
    }

    void CQubicleLoader::AddVoxel(VoxelMesh mesh, int color, CVector pos, CVector &Beg, CVector &End)
    {
        int cid = GetColorIdx(color);
        if(cid == -1)
            return;

        pos.y = (uint32_t)mesh->GetSize().y - pos.y;

        if(pos.y < 0)
            int i = 0;

        Beg = Beg.Min(pos);
        End = End.Max(pos);

        mesh->SetVoxel(pos, 0, cid, false);
    }
} // namespace VoxelOptimizer
