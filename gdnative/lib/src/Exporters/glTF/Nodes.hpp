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

#ifndef NODES_HPP
#define NODES_HPP

#include <CJSON/JSON.hpp>
#include <string.h>
#include <VoxelOptimizer/Vector.hpp>
#include <cstdint>

namespace VoxelOptimizer
{
    namespace GLTF
    {
        enum GLTFTypes
        {
            FLOAT = 5126,
            INT = 5125
        };

        class CAsset
        {
            public:
                void Serialize(CJSON &json) const
                {
                    json.AddPair("version", std::string("2.0"));
                    json.AddPair("generator", std::string("Generated with VoxelOptimizer"));
                }        
        };

        class CScene
        {
            public:
                void Serialize(CJSON &json) const
                {
                    json.AddPair("nodes", std::vector<int>(1));
                }    
        };

        class CNode
        {
            public:
                void Serialize(CJSON &json) const
                {
                    json.AddPair("mesh", 0);
                }    
        };

        class CBuffer
        {
            public:
                size_t Size;
                std::string Uri;

                void Serialize(CJSON &json) const
                {
                    json.AddPair("byteLength", Size);
                    json.AddPair("uri", Uri);
                }    
        };

        class CBufferView
        {
            public:
                size_t Size;
                size_t Offset;

                void Serialize(CJSON &json) const
                {
                    json.AddPair("buffer", 0);
                    json.AddPair("byteLength", Size);
                    json.AddPair("byteOffset", Offset);
                }    
        };

        class CAccessor
        {
            public:
                size_t BufferView;
                GLTFTypes ComponentType;
                std::string Type;
                size_t Count;

                inline void SetMax(CVector Max)
                {
                    if(Type == "VEC3")
                        m_Max.insert(m_Max.end(), Max.v, Max.v + 3);
                    else if(Type == "VEC2")
                        m_Max.insert(m_Max.end(), Max.v, Max.v + 2);
                    else
                        m_Max.push_back(Max.x);
                }

                inline void SetMin(CVector Min)
                {
                    if(Type == "VEC3")
                        m_Min.insert(m_Min.end(), Min.v, Min.v + 3);
                    else if(Type == "VEC2")
                        m_Min.insert(m_Min.end(), Min.v, Min.v + 2);
                    else
                        m_Min.push_back(Min.x);
                }

                void Serialize(CJSON &json) const
                {
                    json.AddPair("bufferView", BufferView);
                    json.AddPair("componentType", (int)ComponentType);

                    if(!m_Max.empty())
                        json.AddPair("max", m_Max);

                    if(!m_Min.empty())
                        json.AddPair("min", m_Min);

                    json.AddPair("type", Type);
                    json.AddPair("count", Count);
                }

            private:
                std::vector<float> m_Max, m_Min;
        };

        class CPrimitive
        {
            public:
                CPrimitive() : 
                    NormalAccessor(-1),
                    PositionAccessor(-1),
                    TextCoordAccessor(-1),
                    IndicesAccessor(-1),
                    Material(-1) {}

                int64_t NormalAccessor;
                int64_t PositionAccessor;
                int64_t TextCoordAccessor;
                int64_t IndicesAccessor;
                int64_t Material;

                void Serialize(CJSON &json) const
                {
                    std::map<std::string, int64_t> Attributes;

                    Attributes["NORMAL"] = NormalAccessor;
                    Attributes["POSITION"] = PositionAccessor;
                    Attributes["TEXCOORD_0"] = TextCoordAccessor;

                    json.AddPair("attributes", Attributes);
                    json.AddPair("indices", IndicesAccessor);
                    json.AddPair("material", Material);
                }
        };

        class CMesh
        {
            public:
                std::vector<CPrimitive> Primitives;

                void Serialize(CJSON &json) const
                {
                    json.AddPair("primitives", Primitives);
                }
        };

        class CTexture
        {
            public:
                void Serialize(CJSON &json) const
                {
                    json.AddPair("source", 0);
                }
        };

        class CImage
        {
            public:
                std::string Uri;

                void Serialize(CJSON &json) const
                {
                    json.AddPair("uri", Uri);
                }
        };

        class CMaterial
        {
            public:
                std::string Name;
                float Roughness;
                float Metallic;
                float Emissive;
                float Transparency;

                void Serialize(CJSON &json) const
                {
                    CJSON PBRMetallicRoughness;

                    std::map<std::string, int> BaseColorTexture = {
                        {"index", 0},
                        {"texCoord", 0}
                    };

                    PBRMetallicRoughness.AddPair("baseColorTexture", BaseColorTexture);
                    PBRMetallicRoughness.AddPair("roughnessFactor", Roughness);
                    PBRMetallicRoughness.AddPair("metallicFactor", Metallic);

                    json.AddPair("name", Name);
                    json.AddJSON("pbrMetallicRoughness", PBRMetallicRoughness.Serialize());
                    json.AddPair("emissiveFactor", std::vector<float>(3, Emissive));

                    if(Transparency != 0)
                    {
                        json.AddPair("alphaMode", std::string("MASK"));
                        json.AddPair("alphaCutoff", Transparency);
                    }
                }
        };
    } // namespace GLTF
    
} // namespace VoxelOptimizer

#endif //NODES_HPP