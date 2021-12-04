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

#ifndef QUBICLEBINARY_HPP
#define QUBICLEBINARY_HPP

#include <VoxelOptimizer/Mat4x4.hpp>
#include <VoxelOptimizer/Loaders/ILoader.hpp>

namespace VoxelOptimizer
{
    class CQubicleBinary : public ILoader
    {
        public:
            CQubicleBinary() = default;
            ~CQubicleBinary() = default;

        protected:
            struct SQubicleBinaryHeader
            {
                char Version[4];        // major, minor, release, build
                int ColorFormat;        // 0 = RGBA, 1 = BGRA
                int ZAxisOrientation;   // 0 = Left hand, 1 = Right hand
                int Compression;        // 1 = RLE, 0 = Uncompressed
                int VisibilityMask;     // 0 = If Alpha is 0 means invisible 255 visible, 1 = A tells which side is visible
                int MatrixCount;        // Number of matrices(Models) inside this file.
            };
            SQubicleBinaryHeader m_Header;
            std::map<int, int> m_ColorIdx;
            
            void ParseFormat() override;
            void ReadUncompressed(VoxelMesh mesh);
            void ReadRLECompressed(VoxelMesh mesh);
            int GetColorIdx(int color);

            CVector ReadVector();
    };
} // namespace VoxelOptimizer

#endif //QUBICLEBINARY_HPP