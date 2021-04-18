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
#include <sstream>
#include <VoxelOptimizer/Exporters/WavefrontObjExporter.hpp>

namespace VoxelOptimizer
{   
    void CWavefrontObjExporter::SaveObj(const std::string &Path, Mesh Mesh)
    {
        auto Tuple = GenerateObj(Mesh);

        // Names the MTL the same as the obj file.
        m_MTLFileName = Path;
        std::replace(m_MTLFileName.begin(), m_MTLFileName.end(), '\\', '/');
        size_t Pos = m_MTLFileName.find_last_of("/");
        if(Pos != std::string::npos)
            m_MTLFileName = m_MTLFileName.substr(Pos);
        
        Pos = m_MTLFileName.find_last_of(".");
        if(Pos != std::string::npos)
            m_MTLFileName = m_MTLFileName.erase(Pos);

        std::ofstream out(Path, std::ios::out);
        if(out.is_open())
        {
            std::string Val = std::get<0>(Tuple);

            out.write(Val.data(), Val.size());
            out.close();

            std::ofstream out(m_MTLFileName + ".mtl", std::ios::out);
            if(out.is_open())
            {
                Val = std::get<1>(Tuple);

                out.write(Val.data(), Val.size());
                out.close();
            }
        }
    }

    std::tuple<std::string, std::string, std::vector<char>> CWavefrontObjExporter::GenerateObj(Mesh Mesh)
    {
        std::stringstream ObjFile, MTLFile;
        if(m_MTLFileName.empty())
            m_MTLFileName = "materials";


        ObjFile << "# Generated with VoxelOptimizer" << std::endl;
        ObjFile << "# These comments can be removed" << std::endl;
        ObjFile << "mtllib " << m_MTLFileName << ".mtl" << std::endl;

        for (auto &&v : Mesh->Vertices)
            ObjFile << "v " << v.x << " " << v.y << " " << v.z << std::endl;

        for (auto &&vn : Mesh->Normals)
            ObjFile << "vn " << vn.x << " " << vn.y << " " << vn.z << std::endl;

        size_t MatCounter = 0;
        for (auto &&f : Mesh->Faces)
        {
            MTLFile << "newmtl Mat" << MatCounter << std::endl;
            MTLFile << "kd " << f->Material.Diffuse.R / 255.f << " " << f->Material.Diffuse.G / 255.f << " " << f->Material.Diffuse.B / 255.f << std::endl;
            
            ObjFile << "usemtl Mat" << MatCounter << std::endl;

            for (size_t i = 0; i < f->Indices.size(); i += 3)
            {
                ObjFile << "f";
                for (char j = 0; j < 3; j++)
                {
                    ObjFile << " ";
                    ObjFile << f->Indices[i + j].x << "//" << f->Indices[i + j].y;
                }
                ObjFile << std::endl;
            }

            MatCounter++;         
        }

        return std::make_tuple<std::string, std::string, std::vector<char>>(ObjFile.str(), MTLFile.str(), std::vector<char>()); 
    }
} // namespace VoxelOptimizer
