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
            std::ofstream out(PathWithoutExt + "." + f.first, std::ios::out);
            if(out.is_open())
            {
                out.write(f.second.data(), f.second.size());
                out.close();
            }
        }
    }

    std::map<std::string, std::vector<char>> CGLTFExporter::Generate(Mesh Mesh)
    {
        CJSON json;
        json.AddPair("asset", CAsset());
        json.AddPair("scene", 0);
        json.AddPair("scenes", std::vector<CScene>() = { CScene() });
        json.AddPair("nodes", std::vector<CNode>() = { CNode() });

        std::vector<CBufferView> BufferViews;
        std::vector<CAccessor> Accessors;

        std::vector<char> Binary;

        CMesh GLTFMesh;

        for (auto &&f : Mesh->Faces)
        {
            std::vector<CVector> Vertices, Normals, UVs;
            std::vector<int> Indices;
            std::map<size_t, int> IndicesIndex;

            CVector Max, Min(10000, 10000, 10000);

            for (size_t i = 0; i < f->Indices.size(); i += 3)
            {
                for (size_t j = 0; j < 3; j++)
                {
                    CVector vec = f->Indices[i + j];

                    int Index = 0;
                    size_t Hash = vec.hash();

                    auto IT = IndicesIndex.find(Hash);
                    if(IT == IndicesIndex.end())
                    {
                        Vertices.push_back(Mesh->Vertices[(size_t)vec.x - 1]);
                        Normals.push_back(Mesh->Normals[(size_t)vec.y - 1]);
                        UVs.push_back(Mesh->UVs[(size_t)vec.z - 1]);

                        Min = Min.Min(Mesh->Vertices[(size_t)vec.x - 1]);
                        Max = Max.Max(Mesh->Vertices[(size_t)vec.x - 1]);

                        Index = Vertices.size() - 1;
                        IndicesIndex.insert({Hash, Index});
                    }
                    else
                        Index = IT->second;

                    Indices.push_back(Index);
                }
            }

            CBufferView Position, Normal, UV, IndexView;

            Position.Offset = Binary.size();
            Position.Size = Vertices.size() * sizeof(CVector);

            Normal.Offset = Position.Offset + Position.Size;
            Normal.Size = Normals.size() * sizeof(CVector);

            UV.Offset = Normal.Offset + Normal.Size;
            UV.Size = UVs.size() * (sizeof(float) * 2);

            IndexView.Offset = UV.Offset + UV.Size;
            IndexView.Size = Indices.size() * sizeof(int);

            CAccessor PositionAccessor, NormalAccessor, UVAccessor, IndexAccessor;
            PositionAccessor.BufferView = BufferViews.size();
            PositionAccessor.ComponentType = GLTFTypes::FLOAT;
            PositionAccessor.Count = Vertices.size();
            PositionAccessor.Type = "VEC3";
            PositionAccessor.SetMin(Min);
            PositionAccessor.SetMax(Max);

            NormalAccessor.BufferView = BufferViews.size() + 1;
            NormalAccessor.ComponentType = GLTFTypes::FLOAT;
            NormalAccessor.Count = Normals.size();
            NormalAccessor.Type = "VEC3";

            UVAccessor.BufferView = BufferViews.size() + 2;
            UVAccessor.ComponentType = GLTFTypes::FLOAT;
            UVAccessor.Count = UVs.size();
            UVAccessor.Type = "VEC2";

            IndexAccessor.BufferView = BufferViews.size() + 3;
            IndexAccessor.ComponentType = GLTFTypes::INT;
            IndexAccessor.Count = Indices.size();
            IndexAccessor.Type = "SCALAR";

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

            size_t Pos = Binary.size();

            Binary.resize(Binary.size() + Position.Size + Normal.Size + UV.Size + IndexView.Size);

            memcpy(Binary.data() + Pos, Vertices.data(), Position.Size);
            Pos += Position.Size;

            memcpy(Binary.data() + Pos, Normals.data(), Normal.Size);
            Pos += Normal.Size;

            for (auto &&uv : UVs)
            {
                memcpy(Binary.data() + Pos, &uv, sizeof(float) * 2);
                Pos += sizeof(float) * 2;
            }

            memcpy(Binary.data() + Pos, Indices.data(), IndexView.Size);
        }

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
