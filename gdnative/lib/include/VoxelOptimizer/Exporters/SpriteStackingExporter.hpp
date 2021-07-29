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

#ifndef SPRITESTACKINGEXPORTER_HPP
#define SPRITESTACKINGEXPORTER_HPP

#include <VoxelOptimizer/Loaders/MagicaVoxelLoader.hpp>
#include <string>
#include <vector>

namespace VoxelOptimizer
{
    class CSpriteStackingExporter
    {
        public:
            CSpriteStackingExporter() = default;

            /**
             * @brief Generates and saves the mesh as png slices.
             * 
             * @param Path: Path of the file.
             * @param Mesh: Mesh to save.
             */
            void Save(const std::string &Path, VoxelMesh m, ILoader *Loader);

            /**
             * @brief Generates the file stream.
             * 
             * @param Mesh: Mesh to save.
             * 
             * @return Returns a png image.
             */
            std::vector<char> Generate(VoxelMesh m, ILoader *Loader); 

            ~CSpriteStackingExporter() = default;
        private:
        /* data */
    };
} // namespace VoxelOptimizer


#endif //SPRITESTACKINGEXPORTER_HPP