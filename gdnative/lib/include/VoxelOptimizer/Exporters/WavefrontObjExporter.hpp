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

#ifndef WAVEFRONTOBJEXPORTER_HPP
#define WAVEFRONTOBJEXPORTER_HPP

#include <VoxelOptimizer/Mesh.hpp>
#include <string>
#include <tuple>
#include <vector>
namespace VoxelOptimizer
{
    class CWavefrontObjExporter
    {
        public:
            CWavefrontObjExporter() = default;

            /**
             * @brief Generates and saves the obj to disk.
             * 
             * @param Path: Path of the generated obj.
             * @param Mesh: Mesh to save.
             */
            void SaveObj(const std::string &Path, Mesh Mesh);

            /**
             * @brief Generates the obj.
             * 
             * @param Mesh: Mesh to save.
             * 
             * @return Returns a tuple where value one is the obj file, two the mtl and the third one the texture file.
             */
            std::tuple<std::string, std::string, std::vector<char>> GenerateObj(Mesh Mesh);

            /**
             * @brief Sets the name for the MTL file. Only needed for Generate Obj.
             */
            inline void SetMTLFileName(std::string MTLFileName)
            {
                m_MTLFileName = MTLFileName;
            }

            ~CWavefrontObjExporter() = default;
        private:
            std::string m_MTLFileName;
    };
} // namespace VoxelOptimizer

#endif //WAVEFRONTOBJEXPORTER_HPP