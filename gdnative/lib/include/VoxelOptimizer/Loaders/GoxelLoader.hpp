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

#ifndef GOXELLOADER_HPP
#define GOXELLOADER_HPP

#include <VoxelOptimizer/Loaders/ILoader.hpp>
#include <string.h>

namespace VoxelOptimizer
{
    class CGoxelLoader : public ILoader
    {
        public:
            CGoxelLoader() = default;

            using ILoader::Load;
        
            ~CGoxelLoader() = default;

        private:
            struct SChunkHeader
            {
                char Type[4];
                int Size;   // Datasize
            };

            struct BL16
            {
                public:
                    BL16()
                    {
                        m_Data.resize(16 * 16 * 16);
                    }

                    inline void SetData(uint32_t *Data)
                    {
                        memcpy(&m_Data[0], Data, m_Data.size());
                    }

                    inline uint32_t GetVoxel(CVector v)
                    {
                        return m_Data[(size_t)v.x + 16 * (size_t)v.y + 16 * 16 * (size_t)v.z];
                    }

                private:
                    std::vector<uint32_t> m_Data;
            }; 

            struct Layer
            {
                CVector Pos;
                size_t Index;
            };

            void ParseFormat() override;
    };
} // namespace VoxelOptimizer

#endif //GOXELLOADER_HPP