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

#include <fstream>
#include <sstream>
#include <voxeloptimizer/WavefrontObjExporter.hpp>

namespace VoxelOptimizer
{   
    void CWavefrontObjExporter::SaveObj(const std::string &Path, SimpleMesh Mesh)
    {
        auto Tuple = GenerateObj(Mesh);

        std::ofstream out(Path, std::ios::out);
        if(out.is_open())
        {
            std::string Val = std::get<0>(Tuple);

            out.write(Val.data(), Val.size());
            out.close();

            std::ofstream out("materials.mtl", std::ios::out);
            if(out.is_open())
            {
                Val = std::get<1>(Tuple);

                out.write(Val.data(), Val.size());
                out.close();
            }
        }
    }

    std::tuple<std::string, std::string, std::vector<char>> CWavefrontObjExporter::GenerateObj(SimpleMesh Mesh)
    {
        std::stringstream ObjFile, MTLFile;

        ObjFile << "# Generated with Voxeloptimizer" << std::endl;
        ObjFile << "# These comments can be removed" << std::endl;
        ObjFile << "mtllib materials.mtl" << std::endl;

        for (auto &&v : Mesh->Vertices)
            ObjFile << "v " << v.x << " " << v.y << " " << v.z << std::endl;

        size_t MatCounter = 0;
        for (auto &&f : Mesh->Faces)
        {
            MTLFile << "newmtl Mat" << MatCounter << std::endl;
            MTLFile << "kd " << f->Material.Diffuse.R / 255.f << " " << f->Material.Diffuse.G / 255.f << " " << f->Material.Diffuse.B / 255.f << std::endl;
            
            ObjFile << "usemtl Mat" << MatCounter << std::endl;

            for (size_t i = 0; i < f->Indices.size(); i += 3)
                ObjFile << "f " << f->Indices[i] << " " << f->Indices[i + 1] << " " << f->Indices[i + 2] << std::endl;

            MatCounter++;         
        }

        return std::make_tuple<std::string, std::string, std::vector<char>>(ObjFile.str(), MTLFile.str(), std::vector<char>()); 
    }
} // namespace VoxelOptimizer
