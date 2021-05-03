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

#include <algorithm>
#include <fstream>
#include "Nodes.hpp"
#include <sstream>
#include <string.h>
#include <stb_image_write.h>
#include <VoxelOptimizer/Exporters/GLTFExporter.hpp>

namespace VoxelOptimizer
{
    void CGLTFExporter::Save(const std::string &Path, Mesh Mesh)
    {
        // Names the MTL the same as the obj file.
        m_ExternalFilenames = Path;
        std::replace(m_ExternalFilenames.begin(), m_ExternalFilenames.end(), '\\', '/');
        size_t Pos = m_ExternalFilenames.find_last_of("/");
        if(Pos != std::string::npos)
            m_ExternalFilenames = m_ExternalFilenames.substr(Pos);
        
        // Removes the file extension.
        Pos = m_ExternalFilenames.find_last_of(".");
        if(Pos != std::string::npos)
            m_ExternalFilenames = m_ExternalFilenames.erase(Pos);

        std::string PathWithoutExt = Path;

        // Removes the file extension.
        Pos = PathWithoutExt.find_last_of(".");
        if(Pos != std::string::npos)
            PathWithoutExt = PathWithoutExt.erase(Pos);

        auto Files = Generate(Mesh);
        for (auto &&f : Files)
        {
            std::ofstream out(Path + "." + f.first, std::ios::out);
            if(out.is_open())
            {
                out.write(f.second.data(), f.second.size());
                out.close();
            }
        }
        

        // std::ofstream out(Path, std::ios::out);
        // if(out.is_open())
        // {
        //     std::string Val = std::get<0>(Tuple);

        //     out.write(Val.data(), Val.size());
        //     out.close();

        //     out.open(m_ExternalFilenames + ".mtl", std::ios::out);
        //     if(out.is_open())
        //     {
        //         Val = std::get<1>(Tuple);

        //         out.write(Val.data(), Val.size());
        //         out.close();

        //         out.open(m_ExternalFilenames + ".png", std::ios::out);
        //         if(out.is_open())
        //         {
        //             auto Texture = std::get<2>(Tuple);

        //             out.write(Texture.data(), Texture.size());
        //             out.close();
        //         }
        //     }
        // }
    }

    std::map<std::string, std::vector<char>> CGLTFExporter::Generate(Mesh Mesh)
    {
        CJSON json;
        json.AddPair("asset", CAsset());
        json.AddPair("scene", 0);
        json.AddPair("scenes", std::vector<CScene>() = { CScene() });
        json.AddPair("nodes", std::vector<CNode>() = { CNode() });

        std::vector<CVector> Vertices, Normals, UVs;
        std::vector<int> Indices;
        std::map<size_t, int> IndicesIndex;

        std::vector<CBufferView> BufferViews;
        std::vector<CAccessor> Accessors;

        CMesh GLTFMesh;

        for (auto &&f : Mesh->Faces)
        {
            CBufferView Position, Normal, UV, IndexView;
            Position.Offset = Vertices.size();
            Normal.Offset = Normals.size();
            UV.Offset = UVs.size();
            IndexView.Offset = Indices.size();

            for (auto &&i : f->Indices)
            {
                int Index = 0;
                size_t Hash = i.hash();

                auto IT = IndicesIndex.find(Hash);
                if(IT == IndicesIndex.end())
                {
                    Vertices.push_back(Mesh->Vertices[(size_t)i.x]);
                    Normals.push_back(Mesh->Normals[(size_t)i.y]);
                    UVs.push_back(Mesh->UVs[(size_t)i.z]);

                    Index = Vertices.size() - 1;
                    IndicesIndex.insert({Hash, Index});
                }
                else
                    Index = IT->second;

                Indices.push_back(Index);
            }

            Position.Size = Vertices.size() - Position.Offset;
            Normal.Size = Normals.size() - Normal.Offset;
            UV.Size = UVs.size() - UV.Offset;
            IndexView.Size = Indices.size() - IndexView.Offset;

            CAccessor PositionAccessor, NormalAccessor, UVAccessor, IndexAccessor;
            PositionAccessor.BufferView = BufferViews.size();
            PositionAccessor.ComponentType = GLTFTypes::FLOAT;
            PositionAccessor.Type = "VEC3";
            PositionAccessor.Min = CVector(-1, -1, -1);
            PositionAccessor.Max = CVector(1, 1, 1);

            NormalAccessor.BufferView = BufferViews.size() + 1;
            NormalAccessor.ComponentType = GLTFTypes::FLOAT;
            NormalAccessor.Type = "VEC3";
            NormalAccessor.Min = CVector(-1, -1, -1);
            NormalAccessor.Max = CVector(1, 1, 1);

            UVAccessor.BufferView = BufferViews.size() + 2;
            UVAccessor.ComponentType = GLTFTypes::FLOAT;
            UVAccessor.Type = "VEC2";
            UVAccessor.Min = CVector(0, 0, 0);
            UVAccessor.Max = CVector(1, 1, 0);

            IndexAccessor.BufferView = BufferViews.size() + 3;
            IndexAccessor.ComponentType = GLTFTypes::INT;
            IndexAccessor.Type = "SCALAR";
            IndexAccessor.Min = CVector(INT32_MIN, 0, 0);
            IndexAccessor.Max = CVector(INT32_MAX, 0, 0);

            CPrimitive Primitive;
            Primitive.PositionAccessor = Accessors.size();
            Primitive.NormalAccessor = Accessors.size() + 1;
            Primitive.TextCoordAccessor = Accessors.size() + 2;
            Primitive.IndicesAccessor = Accessors.size() + 3;

            GLTFMesh.Primitives.push_back(Primitive);

            BufferViews.push_back(Position);
            BufferViews.push_back(Normal);
            BufferViews.push_back(UV);
            BufferViews.push_back(IndexView);

            Accessors.push_back(PositionAccessor);
            Accessors.push_back(NormalAccessor);
            Accessors.push_back(UVAccessor);
            Accessors.push_back(IndexAccessor);
        }

        std::vector<char> Binary(Vertices.size() * (sizeof(float) * 3) + Normals.size() * (sizeof(float) * 3) + UVs.size() * (sizeof(float) * 2) + Indices.size() * sizeof(int));
        
        size_t Pos = 0;
        memcpy(&Binary[Pos], Vertices.data(), Vertices.size() * (sizeof(float) * 3));

        Pos += Vertices.size() * (sizeof(float) * 3);
        memcpy(&Binary[Pos], Normals.data(), Normals.size() * (sizeof(float) * 3));

        for (auto &&uv : UVs)
        {
            Binary.push_back(uv.x);
            Binary.push_back(uv.y);
        }

        Pos += UVs.size() * (sizeof(float) * 2);
        memcpy(&Binary[Pos], Indices.data(), Indices.size() * sizeof(int));

        json.AddPair("meshes", std::vector<CMesh>() = { GLTFMesh });
        json.AddPair("accessors", Accessors);
        json.AddPair("bufferViews", BufferViews);

        CBuffer Buffer;
        Buffer.Size = Binary.size();
        Buffer.Uri = m_ExternalFilenames + ".bin";

        json.AddPair("buffers", std::vector<CBuffer>() = { Buffer });

        std::string JS = json.Serialize();
        std::vector<char> GLTF(JS.begin(), JS.end());

        return {
            {"gltf", GLTF},
            {"bin", Binary}
        };
    }
} // namespace VoxelOptimizer
