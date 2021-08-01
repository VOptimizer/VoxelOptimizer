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
        m_Binary = Path.find(".glb") != std::string::npos;
        IExporter::Save(Path, Mesh);
    }

    std::map<std::string, std::vector<char>> CGLTFExporter::Generate(Mesh Mesh)
    {
        std::vector<GLTF::CBufferView> BufferViews;
        std::vector<GLTF::CAccessor> Accessors;
        std::vector<GLTF::CMaterial> Materials;

        std::vector<char> Binary;

        GLTF::CMesh GLTFMesh;

        for (auto &&f : Mesh->Faces)
        {
            std::vector<CVector> Vertices, Normals, UVs;
            std::vector<int> Indices;
            std::map<size_t, int> IndicesIndex;

            CVector Max, Min(10000, 10000, 10000);

            GLTF::CMaterial Mat;
            Mat.Name = "Mat" + std::to_string(GLTFMesh.Primitives.size());
            Mat.Metallic = f->FaceMaterial->Metallic;
            Mat.Roughness = f->FaceMaterial->Roughness;
            Mat.Emissive = f->FaceMaterial->Power;
            Mat.Transparency = f->FaceMaterial->Transparency;
            Materials.push_back(Mat);

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

            GLTF::CBufferView Position, Normal, UV, IndexView;

            Position.Offset = Binary.size();
            Position.Size = Vertices.size() * sizeof(CVector);

            Normal.Offset = Position.Offset + Position.Size;
            Normal.Size = Normals.size() * sizeof(CVector);

            UV.Offset = Normal.Offset + Normal.Size;
            UV.Size = UVs.size() * (sizeof(float) * 2);

            IndexView.Offset = UV.Offset + UV.Size;
            IndexView.Size = Indices.size() * sizeof(int);

            GLTF::CAccessor PositionAccessor, NormalAccessor, UVAccessor, IndexAccessor;
            PositionAccessor.BufferView = BufferViews.size();
            PositionAccessor.ComponentType = GLTF::GLTFTypes::FLOAT;
            PositionAccessor.Count = Vertices.size();
            PositionAccessor.Type = "VEC3";
            PositionAccessor.SetMin(Min);
            PositionAccessor.SetMax(Max);

            NormalAccessor.BufferView = BufferViews.size() + 1;
            NormalAccessor.ComponentType = GLTF::GLTFTypes::FLOAT;
            NormalAccessor.Count = Normals.size();
            NormalAccessor.Type = "VEC3";

            UVAccessor.BufferView = BufferViews.size() + 2;
            UVAccessor.ComponentType = GLTF::GLTFTypes::FLOAT;
            UVAccessor.Count = UVs.size();
            UVAccessor.Type = "VEC2";

            IndexAccessor.BufferView = BufferViews.size() + 3;
            IndexAccessor.ComponentType = GLTF::GLTFTypes::INT;
            IndexAccessor.Count = Indices.size();
            IndexAccessor.Type = "SCALAR";

            GLTF::CPrimitive Primitive;
            Primitive.PositionAccessor = Accessors.size();
            Primitive.NormalAccessor = Accessors.size() + 1;
            Primitive.TextCoordAccessor = Accessors.size() + 2;
            Primitive.IndicesAccessor = Accessors.size() + 3;
            Primitive.Material = GLTFMesh.Primitives.size();

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

        GLTF::CImage Image;
        GLTF::CBuffer Buffer;

        std::vector<char> Texture;
        stbi_write_png_to_func([](void *context, void *data, int size){
            std::vector<char> *InnerTexture = (std::vector<char>*)context;
            InnerTexture->insert(InnerTexture->end(), (char*)data, ((char*)data) + size);
        }, &Texture, Mesh->Texture.size(), 1, 4, Mesh->Texture.data(), 4 * Mesh->Texture.size());

        // For glb add padding to satify the 4 Byte boundary.
        if(m_Binary)
        {            
            size_t Size = Binary.size();
            int Padding = (Binary.size() + Texture.size()) % 4;

            Binary.resize(Binary.size() + Texture.size() + Padding, '\0');
            memcpy(Binary.data() + Size, Texture.data(), Texture.size());

            GLTF::CBufferView ImageView;
            ImageView.Offset = Size;
            ImageView.Size = Texture.size();

            Image.BufferView = BufferViews.size();
            BufferViews.push_back(ImageView);

            Texture.clear();    // Clear the memory
        }
        else
        {
            Image.Uri = m_ExternalFilenames + ".png";
            Buffer.Uri = m_ExternalFilenames + ".bin";
        }
            

        Buffer.Size = Binary.size(); 

        CJSON json;
        json.AddPair("asset", GLTF::CAsset());
        json.AddPair("scene", 0);
        json.AddPair("scenes", std::vector<GLTF::CScene>() = { GLTF::CScene() });
        json.AddPair("nodes", std::vector<GLTF::CNode>() = { GLTF::CNode() });

        json.AddPair("meshes", std::vector<GLTF::CMesh>() = { GLTFMesh });
        json.AddPair("accessors", Accessors);
        json.AddPair("bufferViews", BufferViews);
        json.AddPair("materials", Materials);        

        json.AddPair("images", std::vector<GLTF::CImage>() = { Image });
        json.AddPair("textures", std::vector<GLTF::CTexture>() = { GLTF::CTexture() });   
        json.AddPair("buffers", std::vector<GLTF::CBuffer>() = { Buffer });
        
        std::string JS = json.Serialize();
        if(m_Binary)
        {
            int Padding = JS.size() % 4;
            for (int i = 0; i < Padding; i++)
                JS += ' ';
        }

        if(!m_Binary)
        {
            std::vector<char> GLTF(JS.begin(), JS.end());
            return {
                {"gltf", GLTF},
                {"bin", Binary},
                {"png", Texture}
            };
        }

        // Builds the binary buffer.
        std::vector<char> GLB(sizeof(uint32_t) * 3 + sizeof(uint32_t) * 2 + JS.size() + sizeof(uint32_t) * 2 + Binary.size(), '\0');
        uint32_t Magic = 0x46546C67; //GLTF in ASCII
        uint32_t Version = 2;
        uint32_t Length = GLB.size();

        memcpy(GLB.data(), &Magic, sizeof(uint32_t));
        memcpy(GLB.data() + sizeof(uint32_t), &Version, sizeof(uint32_t));
        memcpy(GLB.data() + sizeof(uint32_t) * 2, &Length, sizeof(uint32_t));

        size_t Pos = sizeof(uint32_t) * 3; 

        uint32_t ChunkLength = JS.size();
        uint32_t ChunkType = 0x4E4F534A; //JSON in ASCII

        memcpy(GLB.data() + Pos, &ChunkLength, sizeof(uint32_t));
        memcpy(GLB.data() + Pos + sizeof(uint32_t), &ChunkType, sizeof(uint32_t));
        memcpy(GLB.data() + Pos + sizeof(uint32_t) * 2, JS.data(), JS.size());

        Pos += sizeof(uint32_t) * 2 + JS.size(); 

        ChunkLength = Binary.size();
        ChunkType = 0x004E4942; //Bin in ASCII

        memcpy(GLB.data() + Pos, &ChunkLength, sizeof(uint32_t));
        memcpy(GLB.data() + Pos + sizeof(uint32_t), &ChunkType, sizeof(uint32_t));
        memcpy(GLB.data() + Pos + sizeof(uint32_t) * 2, Binary.data(), Binary.size());

        return {
            {"glb", GLB}
        };
    }
} // namespace VoxelOptimizer
