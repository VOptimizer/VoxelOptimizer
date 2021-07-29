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

#ifndef ILOADER_HPP
#define ILOADER_HPP

#include <array>
#include <VoxelOptimizer/Color.hpp>
#include <istream>
#include <map>
#include <VoxelOptimizer/Material.hpp>
#include <memory>
#include <stdlib.h>
#include <string>
#include <vector>
#include <VoxelOptimizer/Vector.hpp>
#include <VoxelOptimizer/Loaders/VoxelMesh.hpp>

namespace VoxelOptimizer
{
    class ILoader
    {
        public:
            using ColorPalette = std::vector<CColor>;

            /**
             * @brief Loads a .vox file from disk.
             * 
             * @param File: Path to the voxel file.
             */
            virtual void Load(const std::string &File);

            /**
             * @brief Loads voxel file from memory.
             * 
             * @param Data: Data of the file.
             * @param Lenght: Data size.
             */
            virtual void Load(const char *Data, size_t Length);

            /**
             * @return Gets a list with all models inside the voxel file.
             */
            inline std::vector<VoxelMesh> GetModels() const
            {
                return m_Models;
            }

            inline ColorPalette GetColorPalette() const
            {
                return m_UsedColorPalette;
            }

            inline std::vector<Material> GetMaterials() const
            {
                return m_Materials;
            } 

        protected:
            std::vector<VoxelMesh> m_Models;
            std::vector<Material> m_Materials;
            ColorPalette m_UsedColorPalette;

            virtual void ParseFormat() = 0;

            template<class T>
            T ReadData()
            {
                T Ret;
                ReadData((char*)&Ret, sizeof(Ret));
                return Ret;
            }

            void ReadData(char *Buf, size_t Size);
            bool IsEof();
            void Skip(size_t Bytes);
            void Reset();

        private:
            std::vector<char> m_Data;
            size_t m_Pos;
    };
} // namespace VoxelOptimizer

#endif //ILOADER_HPP