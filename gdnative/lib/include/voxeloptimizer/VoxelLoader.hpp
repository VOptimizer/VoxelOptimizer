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
#include <voxeloptimizer/Color.hpp>
#include <memory>
#include <stdlib.h>
#include <string>
#include <vector>
#include <voxeloptimizer/Vector.hpp>

namespace VoxelOptimizer
{
    class CVoxelLoader
    {
        public:  
            using ColorPalette = std::array<CColor, 256>;

            class CVoxel
            {
                public:
                    enum Direction
                    {
                        UP,
                        DOWN,
                        LEFT,
                        RIGHT,
                        FORWARD,
                        BACKWARD
                    };

                    CVoxel();

                    CVector Pos;
                    int Material;

                    // A normal of (0, 0, 0) means invisible face.
                    std::array<CVector, 6> Normals;

                    inline bool IsVisible() const
                    {
                        for (auto &&n : Normals)
                        {
                            if(!n.IsZero())
                                return true;
                        }
                        
                        return false;
                    }

                    ~CVoxel() = default;
            };

            using Voxel = std::shared_ptr<CVoxel>;

            class CModel
            {
                public:
                    CModel() = default;

                    inline void SetSize(CVector Size)
                    {
                        m_Size = Size;

                        m_Voxels.clear();
                        m_Voxels.resize(m_Size.x * m_Size.y * m_Size.z);
                    }

                    inline CVector GetSize() const
                    {
                        return m_Size;
                    }

                    inline std::vector<Voxel> GetVoxels() const
                    {
                        return m_Voxels;
                    }

                    void SetVoxel(CVector Pos, int Material);

                    ~CModel() = default;
                private:
                    void SetNormal(Voxel Cur, Voxel Neighbor, CVoxel::Direction CurDir, CVoxel::Direction NeighborDir, CVector Val);

                    CVector m_Size;
                    std::vector<Voxel> m_Voxels;
            };

            using Model = std::shared_ptr<CModel>;

            CVoxelLoader() = default;

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

            inline std::vector<Model> GetModels() const
            {
                return m_Models;
            }

            inline ColorPalette GetColorPalette() const
            {
                return m_ColorPalette;
            }

            ~CVoxelLoader() = default;
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
            Model ProcessSize(const char *Data, size_t &Pos);
            void ProcessXYZI(Model m, const char *Data, size_t &Pos, size_t Size);

            std::vector<Model> m_Models;
            ColorPalette m_ColorPalette;

            size_t m_Index;
    };
} // namespace VoxelOptimizer


#endif //VOXELLOADER_HPP