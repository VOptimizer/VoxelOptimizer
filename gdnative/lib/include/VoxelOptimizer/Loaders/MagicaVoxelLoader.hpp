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

#include <VoxelOptimizer/Mat4x4.hpp>
#include <VoxelOptimizer/Loaders/ILoader.hpp>

namespace VoxelOptimizer
{
    class CMagicaVoxelLoader : public ILoader
    {
        public:  
            CMagicaVoxelLoader() = default;

            using ILoader::Load;

            ~CMagicaVoxelLoader() = default;
        private:
            void ParseFormat() override;

            enum NodeType
            {
                TRANSFORM,
                GROUP,
                SHAPE
            };

            struct SNode
            {
                public:
                    SNode() = default;
                    SNode(NodeType type) : Type(type) {} 

                    NodeType Type;

                    int NodeID;
                    std::map<std::string, std::string> Attributes;
            };

            struct STransformNode : public SNode
            {
                public:
                    STransformNode() : SNode(NodeType::TRANSFORM) {}

                    int LayerID;
                    int NumFrames;
                    CVector Translation;
                    CMat4x4 Rotation;

                    int ChildID;
            };

            struct SGroupNode : public SNode
            {
                public:
                    SGroupNode() : SNode(NodeType::GROUP), ChildIdx(0) {}

                    int ChildIdx;

                    std::vector<int> ChildrensID;
            };

            struct SShapeNode : public SNode
            {
                public:
                    SShapeNode() : SNode(NodeType::SHAPE) {}

                    std::vector<int> Models;
            };

            using Node = std::shared_ptr<SNode>;
            using TransformNode = std::shared_ptr<STransformNode>;
            using GroupNode = std::shared_ptr<SGroupNode>;
            using ShapeNode = std::shared_ptr<SShapeNode>;
            
            struct SChunkHeader
            {
                char ID[4];
                int ChunkContentSize;
                int ChildChunkSize;
            };

            void LoadDefaultPalette();

            VoxelMesh ProcessSize();
            void ProcessXYZI(VoxelMesh m);
            void ProcessMaterialAndSceneGraph();

            TransformNode ProcessTransformNode();
            GroupNode ProcessGroupNode();
            ShapeNode ProcessShapeNode();

            std::map<int, int> m_ColorMapping;
            std::map<int, int> m_MaterialMapping;

            std::map<int, CMat4x4> m_ModelMatrices;

            ColorPalette m_ColorPalette;

            size_t m_Index;
            size_t m_UsedColorsPos;
    };
} // namespace VoxelOptimizer


#endif //VOXELLOADER_HPP