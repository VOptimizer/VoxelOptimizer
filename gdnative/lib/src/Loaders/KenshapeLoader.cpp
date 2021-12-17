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

#include <VoxelOptimizer/Exceptions.hpp>
#include <VoxelOptimizer/Loaders/KenshapeLoader.hpp>
#include <stb_image.h>
#include "Kenshape.hpp"

namespace VoxelOptimizer
{
    void CKenshapeLoader::ParseFormat()
    {
        m_Models.clear();
        m_Materials.clear();
        m_Textures.clear();

        // Quick'n dirty gzip check.
        if(ReadData<uint8_t>() != 0x1f || ReadData<uint8_t>() != 0x8b || ReadData<uint8_t>() != 8)
            throw CVoxelLoaderException("Invalid file format!");

        Skip(7);

        int OutSize = 0;
        char *Data = stbi_zlib_decode_noheader_malloc(Ptr(), Size() - Tellg(), &OutSize);

        CJSON json;
        Kenshape Content;

        try
        {
            Content = json.Deserialize<Kenshape>(std::string(Data, OutSize));
            free(Data);
        }
        catch(const std::exception& e)
        {
            free(Data);
            throw CVoxelLoaderException("Invalid file format!");
        }

        m_Materials.push_back(Material(new CMaterial()));
        VoxelMesh m = VoxelMesh(new CVoxelMesh());
        m->SetSize(Content->Size);

        std::map<int, int> ColorIdx;
        CVector Pos;
        CVector Beg, End;

        Pos.z = Content->Size.z - 1;
        Pos.y = (int)(Content->Size.y / 2.f);

        for (auto &&tile : Content->Tiles)
        {
            if(tile->ColorIdx != -1)
            {
                int IdxC = 0;
                if(ColorIdx.find(tile->ColorIdx) == ColorIdx.end())
                {
                    auto texIT = m_Textures.find(TextureType::DIFFIUSE);
                    if(texIT == m_Textures.end())
                        m_Textures[TextureType::DIFFIUSE] = Texture(new CTexture());

                    m_Textures[TextureType::DIFFIUSE]->AddPixel(Content->Colors[tile->ColorIdx]);
                    IdxC = m_Textures[TextureType::DIFFIUSE]->Size().x - 1;
                    ColorIdx[tile->ColorIdx] = IdxC;
                }
                else
                    IdxC = ColorIdx[tile->ColorIdx];

                int Blocks = tile->Depth - 1;

                for (size_t y = Pos.y - Blocks; y <= Pos.y + Blocks; y++)
                {
                    CVector v(Pos.x, y, Pos.z);
                    if(Beg.IsZero())
                        Beg = v;

                    Beg = Beg.Min(v);
                    End = End.Max(v);

                    m->SetVoxel(v, 0, IdxC, false);
                }
            }

            Pos.z--;
            if(Pos.z < 0)
            {
                Pos.z = Content->Size.z - 1;;
                Pos.x++;
            }
        }

        m->SetBBox(CBBox(Beg, End));
        m_Models.push_back(m);
    }
} // namespace VoxelOptimizer
