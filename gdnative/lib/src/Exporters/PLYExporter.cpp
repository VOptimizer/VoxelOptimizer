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
#include <VoxelOptimizer/Exporters/PLYExporter.hpp>

namespace VoxelOptimizer
{
    std::map<std::string, std::vector<char>> CPLYExporter::Generate(std::vector<Mesh> Meshes)
    {
        size_t counter = 0;
        std::map<std::string, std::vector<char>> ret;

        for (auto &&mesh : Meshes)
        {
            std::map<CVector, int> Index;
            std::vector<int> Indices;
            std::vector<CVector> Vertices, Normals, UVs;

            //"Extracts" the rotation matrix. Quick'n Dirty
            CMat4x4 rotMat = mesh->ModelMatrix;
            rotMat.x.w = 0;
            rotMat.y.w = 0;
            rotMat.z.w = 0;

            for (auto &&f : mesh->Faces)
            {
                int TmpIndices[3];
                int VertexCounter = 0;

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

                        if(m_Settings->WorldSpace)
                        {
                            Vertex = mesh->ModelMatrix * Vertex;
                            Normal = rotMat * Normal;
                        }

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
            }
            
            std::stringstream file;
            file << "ply" << std::endl;
            file << "format ascii 1.0" << std::endl;
            file << "comment Generated with VoxelOptimizer" << std::endl;
            file << "element vertex " << Vertices.size() << std::endl;
            file << "property float x" << std::endl;
            file << "property float y" << std::endl;
            file << "property float z" << std::endl;
            file << "property float nx" << std::endl;
            file << "property float nx" << std::endl;
            file << "property float nz" << std::endl;
            file << "property float s" << std::endl;
            file << "property float t" << std::endl;
            file << "element face " << Indices.size() / 3 << std::endl;
            file << "property list uchar uint vertex_indices" << std::endl;
            file << "end_header" << std::endl;

            for (size_t i = 0; i < Vertices.size(); i++)
            {
                CVector v = Vertices[i];
                CVector n = Normals[i];
                CVector uv = UVs[i];

                file << v.x << " " << -v.z << " " << v.y << " " << n.x << " " << -n.z << " " << n.y << " " << uv.x << " " << uv.y << std::endl;
            }
            
            for (size_t i = 0; i < Indices.size(); i += 3)
                file << "3 " << Indices[i] << " " << Indices[i + 1] << " " << Indices[i + 2] << std::endl;
            
            std::string fileStr = file.str();

            std::string ext = "ply";
            if(Meshes.size() > 1)
                ext = std::to_string(counter) + ".ply";

            ret.insert({ext, std::vector<char>(fileStr.begin(), fileStr.end())});
            counter++;
        }

        return ret;
    }
} // namespace VoxelOptimizer
