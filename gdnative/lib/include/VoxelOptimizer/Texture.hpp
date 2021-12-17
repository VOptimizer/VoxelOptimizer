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

#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <VoxelOptimizer/Color.hpp>
#include <memory>
#include <stddef.h>
#include <vector>
#include <VoxelOptimizer/Vector.hpp>

namespace VoxelOptimizer
{
    class CTexture
    {
        public:
            CTexture() = default;
            CTexture(const CVector &_size);
            CTexture(const CTexture &_texture);

            void AddPixel(const CColor &color, const CVector &pos);
            void AddPixel(const CColor &color);

            inline CVector Size() const
            {
                return m_Size;
            }

            const std::vector<uint32_t> &Pixels() const
            {
                return m_Pixels;
            }

            uint32_t Pixel(const CVector &pos);

            std::vector<char> AsPNG();

            ~CTexture() = default;
        private:
            CVector m_Size;
            std::vector<uint32_t> m_Pixels;
    };

    using Texture = std::shared_ptr<CTexture>;
} // namespace VoxelOptimizer

#endif //TEXTURE_HPP