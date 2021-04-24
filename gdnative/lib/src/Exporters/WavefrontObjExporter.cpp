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
#include <string.h>
#include "../stb_image_write.h"
#include <VoxelOptimizer/Exporters/WavefrontObjExporter.hpp>

namespace VoxelOptimizer
{   
    void CWavefrontObjExporter::SaveObj(const std::string &Path, Mesh Mesh)
    {
        // Names the MTL the same as the obj file.
        m_MTLFileName = Path;
        std::replace(m_MTLFileName.begin(), m_MTLFileName.end(), '\\', '/');
        size_t Pos = m_MTLFileName.find_last_of("/");
        if(Pos != std::string::npos)
            m_MTLFileName = m_MTLFileName.substr(Pos);
        
        // Removes the file extension.
        Pos = m_MTLFileName.find_last_of(".");
        if(Pos != std::string::npos)
            m_MTLFileName = m_MTLFileName.erase(Pos);

        auto Tuple = GenerateObj(Mesh);

        std::ofstream out(Path, std::ios::out);
        if(out.is_open())
        {
            std::string Val = std::get<0>(Tuple);

            out.write(Val.data(), Val.size());
            out.close();

            out.open(m_MTLFileName + ".mtl", std::ios::out);
            if(out.is_open())
            {
                Val = std::get<1>(Tuple);

                out.write(Val.data(), Val.size());
                out.close();

                out.open(m_MTLFileName + ".png", std::ios::out);
                if(out.is_open())
                {
                    auto Texture = std::get<2>(Tuple);

                    out.write(Texture.data(), Texture.size());
                    out.close();
                }
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

        for (auto &&vt : Mesh->UVs)
            ObjFile << "vt " << vt.x << " " << vt.y << std::endl;

        size_t MatCounter = 0;
        for (auto &&f : Mesh->Faces)
        {
            float Ambient = 1.0;
            int Illum = 2;
            float Transparency = 0;
            float Alpha = 1.0;

            if(f->FaceMaterial->Metallic != 0.0)
            {
                Ambient = f->FaceMaterial->Metallic;
                Illum = 3;
            }
            else if(f->FaceMaterial->Transparency != 0.0) // Glass
            {
                Illum = 4;
                Transparency = f->FaceMaterial->Transparency;
                Alpha = 1 - f->FaceMaterial->Transparency;
            }

            MTLFile << "newmtl Mat" << MatCounter << std::endl;
            MTLFile << "Ns " << (1.0f - f->FaceMaterial->Roughness) * 1000.f << std::endl;
            MTLFile << "Ka " << Ambient << " " << Ambient << " " << Ambient << std::endl;
            MTLFile << "Kd 1.0 1.0 1.0" << std::endl;
            MTLFile << "Ks " << f->FaceMaterial->Specular << " " << f->FaceMaterial->Specular << " " << f->FaceMaterial->Specular << std::endl;
            if(f->FaceMaterial->Power != 0.0)
                MTLFile << "Ke " << f->FaceMaterial->Power << " " << f->FaceMaterial->Power << " " << f->FaceMaterial->Power << std::endl;
            MTLFile << "Tr " << Transparency << std::endl;
            MTLFile << "d " << Alpha << std::endl;
            MTLFile << "Ni " << f->FaceMaterial->IOR << std::endl;
            MTLFile << "illum " << Illum << std::endl;
            MTLFile << "map_Kd " << m_MTLFileName << ".png" << std::endl;
            
            ObjFile << "usemtl Mat" << MatCounter << std::endl;

            for (size_t i = 0; i < f->Indices.size(); i += 3)
            {
                ObjFile << "f";
                for (char j = 0; j < 3; j++)
                {
                    ObjFile << " ";
                    ObjFile << f->Indices[i + j].x << "/" << f->Indices[i + j].z << "/" << f->Indices[i + j].y;
                }
                ObjFile << std::endl;
            }

            MatCounter++;         
        }

        std::vector<char> Texture;
        stbi_write_png_to_func([](void *context, void *data, int size){
            std::vector<char> *InnerTexture = (std::vector<char>*)context;
            InnerTexture->insert(InnerTexture->end(), (char*)data, ((char*)data) + size);
        }, &Texture, Mesh->Texture.size(), 1, 4, Mesh->Texture.data(), 4 * Mesh->Texture.size());

        return std::make_tuple<std::string, std::string, std::vector<char>>(ObjFile.str(), MTLFile.str(), std::move(Texture)); 
    }
} // namespace VoxelOptimizer
