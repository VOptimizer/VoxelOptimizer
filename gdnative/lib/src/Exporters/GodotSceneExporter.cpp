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

#include <sstream>
#include <VoxelOptimizer/Exporters/GodotSceneExporter.hpp>

namespace VoxelOptimizer
{
    std::map<std::string, std::vector<char>> CGodotSceneExporter::Generate(std::vector<Mesh> Meshes)
    {
        std::stringstream os, nodes;
        // os << "[gd_scene load_steps=" << 3 + Mesh->Faces.size() << " format=2]\n" << std::endl;
        size_t ID = 1;
        os << "[ext_resource path=\"res://" << m_ExternalFilenames << ".albedo.png\" type=\"Texture\" id=1]\n" << std::endl;

        auto textures = Meshes[0]->Textures;
        if(textures.find(TextureType::EMISSION) != textures.end())
        {
            os << "[ext_resource path=\"res://" << m_ExternalFilenames << ".emission.png\" type=\"Texture\" id=2]\n" << std::endl;
            ID++;
        }

        std::map<int, size_t> processedMaterials;
        size_t loadStepsSize = 0;

        nodes << "[node name=\"root\" type=\"Spatial\"]\n" << std::endl;

        for (auto &&mesh : Meshes)
        {
            std::stringstream arrayMesh;
            loadStepsSize += mesh->Faces.size();

            size_t surface = 0;

            for (auto &&f : mesh->Faces)
            {
                size_t matID = 0;

                auto IT = processedMaterials.find(f->MaterialIndex);
                if(IT == processedMaterials.end())
                {
                    os << "[sub_resource type=\"SpatialMaterial\" id=" << ID << "]" << std::endl;
                    os << "albedo_texture = ExtResource( 1 )" << std::endl;
                    os << "metallic = " << f->FaceMaterial->Metallic << std::endl;
                    os << "metallic_specular = " << f->FaceMaterial->Specular << std::endl;
                    os << "roughness = " << f->FaceMaterial->Roughness << std::endl;

                    if(f->FaceMaterial->Power != 0)
                    {
                        os << "emission_enabled = true" << std::endl;
                        os << "emission_energy = " << f->FaceMaterial->Power << std::endl;
                        os << "emission_texture = ExtResource( 2 )" << std::endl;
                    }

                    if(f->FaceMaterial->IOR != 0)
                    {
                        os << "refraction_enabled = true" << std::endl;
                        os << "refraction_energy = " << f->FaceMaterial->IOR << std::endl;
                    }

                    if(f->FaceMaterial->Transparency != 0.0)
                    {
                        os << "flags_transparent = true" << std::endl;
                        os << "albedo_color = Color( 1, 1, 1, " << 1.f - f->FaceMaterial->Transparency << ")" << std::endl;
                    }

                    matID = ID;
                    processedMaterials.insert({f->MaterialIndex, matID});
                }
                else
                    matID = IT->second;

                arrayMesh << "surfaces/" << surface << "= {" << std::endl;
                arrayMesh << "\t\"material\":SubResource(" << matID << ")," << std::endl;
                arrayMesh << "\t\"primitive\":4," << std::endl;
                arrayMesh << "\t\"arrays\":[" << std::endl;
                surface++;

                std::map<CVector, int> Index;
                int TmpIndices[3];
                int VertexCounter = 0;

                std::vector<int> Indices;
                std::vector<CVector> Vertices, Normals, UVs;

                for (auto &&v : f->Indices)
                {
                    auto IT = Index.find(v);
                    if(IT != Index.end())
                        TmpIndices[VertexCounter] = IT->second;
                    else
                    {
                        VoxelOptimizer::CVector Vertex, Normal, UV;
                        Vertex = mesh->Vertices[(size_t)v.x - 1];
                        Normal = mesh->Normals[(size_t)v.y - 1];
                        UV = mesh->UVs[(size_t)v.z - 1];

                        Vertices.push_back(Vertex);
                        Normals.push_back(Normal);
                        UVs.push_back(UV);

                        int Idx = Vertices.size() - 1;
                        Index.insert({v, Idx});
                        TmpIndices[VertexCounter] = Idx;
                    }

                    VertexCounter++;
                    if(VertexCounter == 3)
                    {
                        VertexCounter = 0;

                        for (char i = 2; i > -1; i--)
                            Indices.push_back(TmpIndices[i]);
                    }
                }

                arrayMesh << "\t\tVector3Array(";
                bool first = true;
                for (auto &&v : Vertices)
                {
                    if(!first)
                        arrayMesh << ", ";

                    arrayMesh << v.x << ", " << v.y << ", " << v.z;
                    first = false;
                }

                arrayMesh << ")," << std::endl;
                arrayMesh << "\t\tVector3Array(";
                first = true;
                for (auto &&n : Normals)
                {
                    if(!first)
                        arrayMesh << ", ";

                    arrayMesh << n.x << ", " << n.y << ", " << n.z;
                    first = false;
                }
                arrayMesh << ")," << std::endl;
                arrayMesh << "\t\tnull," << std::endl;
                arrayMesh << "\t\tnull," << std::endl;

                arrayMesh << "\t\tVector2Array(";
                first = true;
                for (auto &&uv : UVs)
                {
                    if(!first)
                        arrayMesh << ", ";

                    arrayMesh << uv.x << ", " << uv.y;
                    first = false;
                }
                arrayMesh << ")," << std::endl;
                arrayMesh << "\t\tnull," << std::endl;
                arrayMesh << "\t\tnull," << std::endl;
                arrayMesh << "\t\tnull," << std::endl;

                arrayMesh << "\t\tIntArray(";
                first = true;
                for (auto &&i : Indices)
                {
                    if(!first)
                        arrayMesh << ", ";

                    arrayMesh << i;
                    first = false;
                }
                arrayMesh << ")" << std::endl;
                arrayMesh << "\t]," << std::endl;

                arrayMesh << "\t\"morph_arrays\":[]" << std::endl;
                arrayMesh << "}" << std::endl;

                ID++;
            }        
    
            os << "\n[sub_resource id=" << ID << " type=\"ArrayMesh\"]\n" << std::endl;
            os << arrayMesh.str() << std::endl;

            nodes << "[node name=\"Voxel" << ID << "\" type=\"MeshInstance\" parent=\".\"]\n" << std::endl;
            nodes << "mesh = SubResource(" << ID << ")" << std::endl;
            nodes << "visible = true" << std::endl;

            if(m_Settings->WorldSpace)
            {
                auto matrix = mesh->ModelMatrix;

                nodes << "transform = Transform(";
                nodes << matrix.x.x << ", " << matrix.y.x << ", " << matrix.z.x << ", ";
                nodes << matrix.x.y << ", " << matrix.y.y << ", " << matrix.z.y << ", ";
                nodes << matrix.x.z << ", " << matrix.y.z << ", " << matrix.z.z << ", ";
                nodes << matrix.x.w << ", " << matrix.y.w << ", " << matrix.z.w << ")" << std::endl;
            }
            else
                nodes << "transform = Transform(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0)" << std::endl;
        }

        os << nodes.str();
        
        std::string ESCNFileStr = "[gd_scene load_steps=" + std::to_string(3 + loadStepsSize) + " format=2]\n\n" + os.str();

        std::map<std::string, std::vector<char>> ret = 
        {
            {"escn", std::vector<char>(ESCNFileStr.begin(), ESCNFileStr.end())},
            {"albedo.png", textures[TextureType::DIFFIUSE]->AsPNG()},
        };

        if(textures.find(TextureType::EMISSION) != textures.end())
            ret["emission.png"] = textures[TextureType::EMISSION]->AsPNG();

        return ret;
    }
} // namespace VoxelOptimizer
