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

#ifndef GLTFEXPORTER_HPP
#define GLTFEXPORTER_HPP

#include <map>
#include <VoxelOptimizer/Mesh.hpp>
#include <string>
#include <vector>

namespace VoxelOptimizer
{
    class CGLTFExporter
    {
        public:
            CGLTFExporter() = default;

            /**
             * @brief Generates and saves the mesh.
             * 
             * @param Path: Path of the file.
             * @param Mesh: Mesh to save.
             */
            void Save(const std::string &Path, Mesh Mesh);

            /**
             * @brief Generates the file streams.
             * 
             * @param Mesh: Mesh to save.
             * 
             * @return Returns a map where the key is the type and the std::vector<char> is the data.
             */
            std::map<std::string, std::vector<char>> Generate(Mesh Mesh);

            /**
             * @brief Sets the name for the all the external files. Only needed for memory generation.
             */
            inline void SetExternaFilenames(std::string ExternalFilenames)
            {
                m_ExternalFilenames = ExternalFilenames;
            }

            ~CGLTFExporter() = default;

        private:
            std::string m_ExternalFilenames;
    };
} // namespace VoxelOptimizer

#endif //GLTFEXPORTER_HPP