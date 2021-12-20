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

#include <VoxelOptimizer/Texture.hpp>
#include <string.h>
#include <stb_image_write.h>

namespace VoxelOptimizer
{
    CTexture::CTexture(const CVector &_size)
    {
        m_Size = _size;
        m_Pixels.resize(m_Size.x * m_Size.y);
    }

    CTexture::CTexture(const CTexture &_texture)
    {
        m_Size = _texture.m_Size;
        m_Pixels.resize(m_Size.x * m_Size.y);

        memcpy(&m_Pixels[0], &_texture.m_Pixels[0], m_Pixels.size() * sizeof(uint32_t));
    }

    void CTexture::AddPixel(const CColor &color, const CVector &pos)
    {
        if(pos >= m_Size)
            return;

        m_Pixels[pos.x + m_Size.x * pos.y] = color.AsRGBA();
    }

    void CTexture::AddPixel(const CColor &color)
    {
        if(m_Size.y != 0 && m_Size.y != 1)
            return;

        m_Size.y = 1;
        m_Size.x++;

        m_Pixels.push_back(color.AsRGBA());
    }

    uint32_t CTexture::Pixel(const CVector &pos)
    {
        return m_Pixels[pos.x + m_Size.x * pos.y];
    }

    std::vector<char> CTexture::AsPNG()
    {
        std::vector<char> Texture;
        stbi_write_png_to_func([](void *context, void *data, int size){
            std::vector<char> *InnerTexture = (std::vector<char>*)context;
            InnerTexture->insert(InnerTexture->end(), (char*)data, ((char*)data) + size);
        }, &Texture, m_Size.x, m_Size.y, 4, m_Pixels.data(), sizeof(uint32_t) * m_Pixels.size());

        return Texture;
    }
} // namespace VoxelOptimizer
