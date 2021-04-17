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

#ifndef VOXELLOADER_HPP
#define VOXELLOADER_HPP

#include <array>
#include <VoxelOptimizer/Color.hpp>
#include <memory>
#include <stdlib.h>
#include <string>
#include <vector>
#include <VoxelOptimizer/Vector.hpp>
#include <VoxelOptimizer/Loaders/VoxelMesh.hpp>

namespace VoxelOptimizer
{
    class CMagicaVoxelLoader
    {
        public:  
            using ColorPalette = std::array<CColor, 256>;

            CMagicaVoxelLoader() = default;

            /**
             * @brief Loads a .vox file from disk.
             * 
             * @param File: Path to the voxel file.
             */
            void Load(const std::string &File);

            /**
             * @brief Loads voxel file from memory.
             * 
             * @param Data: Data of the file.
             * @param Lenght: Data size.
             */
            void Load(const char *Data, size_t Length);

            /**
             * @return Gets a list with all models inside the voxel file.
             */
            inline std::vector<VoxelModel> GetModels() const
            {
                return m_Models;
            }

            inline ColorPalette GetColorPalette() const
            {
                return m_ColorPalette;
            }

            ~CMagicaVoxelLoader() = default;
        private:
            struct SChunkHeader
            {
                char ID[4];
                int ChunkContentSize;
                int ChildChunkSize;
            };

            void LoadDefaultPalette();

            SChunkHeader LoadChunk(const char *Data, size_t &Pos);
            void ProcessPack(const SChunkHeader &Chunk, const char *Data, size_t &Pos);
            VoxelModel ProcessSize(const char *Data, size_t &Pos);
            void ProcessXYZI(VoxelModel m, const char *Data, size_t &Pos, size_t Size);

            std::vector<VoxelModel> m_Models;
            ColorPalette m_ColorPalette;

            size_t m_Index;
    };
} // namespace VoxelOptimizer


#endif //VOXELLOADER_HPP