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

#include <VoxelOptimizer/Loaders/QubicleBinaryTreeLoader.hpp>
#include <VoxelOptimizer/Exceptions.hpp>
#include <stb_image.h>
#include <string.h>

namespace VoxelOptimizer
{
    void CQubicleBinaryTreeLoader::ParseFormat()
    {
        m_Models.clear();
        m_Materials.clear();
        m_Materials.clear();
        m_Textures.clear();

        if(ReadData<int>() != 0x32204251)
            throw CVoxelLoaderException("Unknown file format");

        char major = ReadData<char>();
        char minor = ReadData<char>();

        if(major != 1 && minor != 0)
            throw CVoxelLoaderException("Unsupported version!");

        m_Materials.push_back(Material(new CMaterial()));

        Skip(3 * sizeof(float));
        Skip(8); // COLORMAP
        ReadColors();

        Skip(8); // DATATREE
        LoadNode();

        m_ColorIdx.clear();
    }

    void CQubicleBinaryTreeLoader::ReadColors()
    {
        int count = ReadData<int>();
        m_HasColormap = count > 0;
        for (size_t i = 0; i < count; i++)
        {
            CColor c;
            c.FromRGBA(ReadData<uint32_t>());

            auto texIT = m_Textures.find(TextureType::DIFFIUSE);
            if(texIT == m_Textures.end())
                m_Textures[TextureType::DIFFIUSE] = Texture(new CTexture());

            m_Textures[TextureType::DIFFIUSE]->AddPixel(c);
        }
    }

    void CQubicleBinaryTreeLoader::LoadNode()
    {
        uint32_t type = ReadData<uint32_t>();
        uint32_t size = ReadData<uint32_t>();

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

            case 2: // Compound
            {
                LoadCompound();
            }break;
        
            default:
            {
                Skip(size);
            } break;
        }
    }

    void CQubicleBinaryTreeLoader::LoadModel()
    {
        uint32_t childCount = ReadData<uint32_t>();
        for (uint32_t i = 0; i < childCount; i++)
            LoadNode();
    }

    void CQubicleBinaryTreeLoader::LoadMatrix()
    {
        int nameLen = ReadData<int>();
        Skip(nameLen);

        VoxelMesh mesh = VoxelMesh(new CVoxelMesh());
        auto pos = ReadVector();

        Skip(6 * sizeof(int));
        mesh->SetSize(ReadVector());

        auto halfSize = (mesh->GetSize() / 2.0);
        pos += halfSize;
        std::swap(pos.y, pos.z);

        mesh->SetModelMatrix(CMat4x4::Translation(pos));

        uint32_t dataSize = ReadData<uint32_t>();

        int OutSize = 0;
        char *Data = stbi_zlib_decode_malloc(Ptr(), dataSize, &OutSize);
        Skip(dataSize);
        int strmPos = 0;

        CVector Beg(1000, 1000, 1000), End;
        for (uint32_t x = 0; x < (uint32_t)mesh->GetSize().x; x++)
        {
            for (uint32_t z = 0; z < (uint32_t)mesh->GetSize().y; z++)
            {
                for (uint32_t y = 0; y < (uint32_t)mesh->GetSize().z; y++)
                {
                    int color;
                    memcpy(&color, Data + strmPos, sizeof(int));
                    strmPos += sizeof(int);

                    int cid;

                    if(m_HasColormap)
                        cid = color & 0xFF;
                    else
                        cid = GetColorIdx(color);

                    if(cid == -1 || ((color & 0xFF000000) >> 24) == 0)
                        continue;

                    auto pos = CVector(x, z, y);
                    pos.y = (uint32_t)(mesh->GetSize().y - 1) - pos.y;

                    Beg = Beg.Min(pos);
                    End = End.Max(pos);

                    mesh->SetVoxel(pos, 0, cid, false);
                }
            }
        }
        free(Data);

        mesh->SetBBox(CBBox(Beg, End));
        m_Models.push_back(mesh);
    }

    void CQubicleBinaryTreeLoader::LoadCompound()
    {
        int nameLen = ReadData<int>();
        Skip(nameLen);

        VoxelMesh mesh = VoxelMesh(new CVoxelMesh());
        auto pos = ReadVector();

        Skip(6 * sizeof(int));
        mesh->SetSize(ReadVector());

        auto halfSize = (mesh->GetSize() / 2.0);
        pos += halfSize;
        std::swap(pos.y, pos.z);

        mesh->SetModelMatrix(CMat4x4::Translation(pos));

        uint32_t dataSize = ReadData<uint32_t>();

        int OutSize = 0;
        char *Data = stbi_zlib_decode_malloc(Ptr(), dataSize, &OutSize);
        Skip(dataSize);
        int strmPos = 0;

        CVector Beg(1000, 1000, 1000), End;
        for (uint32_t x = 0; x < (uint32_t)mesh->GetSize().x; x++)
        {
            for (uint32_t z = 0; z < (uint32_t)mesh->GetSize().y; z++)
            {
                for (uint32_t y = 0; y < (uint32_t)mesh->GetSize().z; y++)
                {
                    int color;
                    memcpy(&color, Data + strmPos, sizeof(int));
                    strmPos += sizeof(int);

                    int cid;

                    if(m_HasColormap)
                        cid = color & 0xFF;
                    else
                        cid = GetColorIdx(color);

                    if(cid == -1 || ((color & 0xFF000000) >> 24) == 0)
                        continue;

                    auto pos = CVector(x, z, y);
                    pos.y = (uint32_t)(mesh->GetSize().y - 1) - pos.y;

                    Beg = Beg.Min(pos);
                    End = End.Max(pos);

                    mesh->SetVoxel(pos, 0, cid, false);
                }
            }
        }
        free(Data);

        mesh->SetBBox(CBBox(Beg, End));
        m_Models.push_back(mesh);

        uint32_t childCount = ReadData<uint32_t>();
        for (uint32_t i = 0; i < childCount; i++)
            LoadNode();
    }

    CVector CQubicleBinaryTreeLoader::ReadVector()
    {
        CVector ret;

        ret.x = ReadData<int>();
        ret.z = ReadData<int>();
        ret.y = ReadData<int>();

        return ret;
    }

    int CQubicleBinaryTreeLoader::GetColorIdx(int color)
    {
        int ret = 0;

        auto IT = m_ColorIdx.find(color);
        if(IT == m_ColorIdx.end())
        {
            CColor c;
            c.FromRGBA(color);

            if(c.A == 0)
                return -1;

            auto texIT = m_Textures.find(TextureType::DIFFIUSE);
            if(texIT == m_Textures.end())
                m_Textures[TextureType::DIFFIUSE] = Texture(new CTexture());

            m_Textures[TextureType::DIFFIUSE]->AddPixel(c);
            ret = m_Textures[TextureType::DIFFIUSE]->Size().x - 1;

            m_ColorIdx.insert({color, ret});
        }
        else
            ret = IT->second;

        return ret;
    }
} // namespace VoxelOptimizer
