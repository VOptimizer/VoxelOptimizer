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

#include "../FileUtils.hpp"
#include <algorithm>
#include <fstream>
#include <VoxelOptimizer/Exporters/GLTFExporter.hpp>
#include <VoxelOptimizer/Exporters/GodotSceneExporter.hpp>
#include <VoxelOptimizer/Exporters/IExporter.hpp>
#include <VoxelOptimizer/Exporters/WavefrontObjExporter.hpp>
#include <VoxelOptimizer/Exporters/PLYExporter.hpp>

namespace VoxelOptimizer
{
    Exporter IExporter::Create(ExporterTypes type)
    {
        switch (type)
        {
            case ExporterTypes::OBJ: return Exporter(new CWavefrontObjExporter());

            case ExporterTypes::GLTF: 
            case ExporterTypes::GLB:
            {
                auto tmp = Exporter(new CGLTFExporter());
                tmp->Settings()->Binary = type == ExporterTypes::GLB;

                return tmp;
            } 

            case ExporterTypes::PLY: return Exporter(new CPLYExporter());
            case ExporterTypes::ESCN: return Exporter(new CGodotSceneExporter());
        }
    }

    IExporter::IExporter() : m_Settings(new CExportSettings())
    {

    }

    void IExporter::Save(const std::string &Path, Mesh mesh)
    {
        Save(Path, std::vector<Mesh>() = { mesh });
    }

    void IExporter::Save(const std::string &Path, std::vector<Mesh> Meshes)
    {
        // Names all files like thhe output file.
        m_ExternalFilenames = GetFilenameWithoutExt(Path);
        std::string PathWithoutExt = GetPathWithoutExt(Path);

        auto Files = Generate(Meshes);
        for (auto &&f : Files)
        {
            std::ofstream out(PathWithoutExt + std::string(".") + f.first, std::ios::binary);
            if(out.is_open())
            {
                out.write(f.second.data(), f.second.size());
                out.close();
            }
        }
    }

    std::map<std::string, std::vector<char>> IExporter::Generate(Mesh mesh)
    {
        return Generate(std::vector<Mesh>() = { mesh });
    }
} // namespace VoxelOptimizer
