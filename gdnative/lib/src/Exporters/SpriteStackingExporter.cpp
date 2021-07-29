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

#include <fstream>
#include <stb_image_write.h>
#include <VoxelOptimizer/Exporters/SpriteStackingExporter.hpp>

namespace VoxelOptimizer
{
    void CSpriteStackingExporter::Save(const std::string &Path, VoxelMesh m, ILoader *Loader)
    {
        auto Image = Generate(m, Loader);
        std::ofstream out(Path, std::ios::binary);
        if(out.is_open())
        {
            out.write(Image.data(), Image.size());
            out.close();
        }
    }

    std::vector<char> CSpriteStackingExporter::Generate(VoxelMesh m, ILoader *Loader)
    {
        CVector Size = m->GetSize();
        std::vector<uint32_t> Pixels(Size.x * Size.y * Size.z, 0);

        auto Palette = Loader->GetColorPalette();

        for (size_t z = 0; z < Size.z; z++)
        {
            for (size_t x = 0; x < Size.x; x++)
            {
                for (size_t y = 0; y < Size.y; y++)
                {
                    auto Vox = m->GetVoxel(CVector(x, y, z));

                    if(Vox)
                        Pixels[x + (size_t)Size.x * y + (size_t)Size.x * (size_t)Size.y * z] = Palette[Vox->Color].AsRGBA();
                }
            }
        }
        
        std::vector<char> Texture;
        stbi_write_png_to_func([](void *context, void *data, int size){
            std::vector<char> *InnerTexture = (std::vector<char>*)context;
            InnerTexture->insert(InnerTexture->end(), (char*)data, ((char*)data) + size);
        }, &Texture, Size.x, Size.y * Size.z, 4, Pixels.data(), sizeof(uint32_t) * Size.x);

        return Texture;
    }
} // namespace VoxelOptimizer
