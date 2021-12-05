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

#ifndef VEDITHANDLER_HPP
#define VEDITHANDLER_HPP

#include <VoxelOptimizer/Loaders/ILoader.hpp>
#include <VoxelOptimizer/Loaders/ISaver.hpp>

namespace VoxelOptimizer
{
    class CVEditHandler : public ILoader, ISaver
    {
        public:
            CVEditHandler(/* args */) {}

            std::vector<char> Save(VoxelMesh m) override;

            ~CVEditHandler() {}

        private:
            enum Types
            {
                COLORS,
                MATERIALS,
                VOXELS
            };

            void ParseFormat() override;  

            void WriteSection(Types type, std::vector<char> &Ret, const std::vector<char> &Data); 
            
            std::vector<char> WriteColors();
            std::vector<char> WriteMaterials();
            std::vector<char> WriteVoxels();

            VoxelMesh m_Mesh;
    };
} // namespace VoxelOptimizer

#endif //VEDITHANDLER_HPP