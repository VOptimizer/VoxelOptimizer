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

#include <voxeloptimizer/Exceptions.hpp>
#include <File.hpp>
#include <GodotVoxelOptimizer.hpp>
#include <voxeloptimizer/SimpleMesher.hpp>
#include <voxeloptimizer/WavefrontObjExporter.hpp>

void CGodotVoxelOptimizer::_register_methods()
{
    register_method("load", &CGodotVoxelOptimizer::Load);
    register_method("get_mesh", &CGodotVoxelOptimizer::GetMesh);
}

godot_error CGodotVoxelOptimizer::Load(String Path)
{
    Ref<File> VFile = File::_new();
    if(!VFile->file_exists(Path))
    {
        ERR_PRINT("File " + Path + " doesn't exists!");
        return (godot_error)Error::ERR_FILE_NOT_FOUND;
    }

    Error err = VFile->open(Path, File::READ);
    if(err != Error::OK)
        return (godot_error)err;

    PoolByteArray Data = VFile->get_buffer(VFile->get_len());
    
    try
    {
        m_Loader.Load((char*)Data.read().ptr(), Data.size());
    }
    catch(const VoxelOptimizer::CVoxelLoaderException &e)
    {
        VFile->close();

        ERR_PRINT(e.what());
        return (godot_error)Error::ERR_PARSE_ERROR;
    }
    
    VFile->close();

    return (godot_error)Error::OK;
}

Ref<ArrayMesh> CGodotVoxelOptimizer::GetMesh(bool Optimized)
{
    if(m_Loader.GetModels().empty())
    {
        ERR_PRINT("No file loaded please call load before!");
        return nullptr;
    }

    VoxelOptimizer::CSimpleMesher Mesher;
    auto Mesh = Mesher.GenerateMesh(m_Loader.GetModels().front(), m_Loader.GetColorPalette());

    Ref<ArrayMesh> Ret = ArrayMesh::_new();
    Array arr;
    arr.resize(ArrayMesh::ARRAY_MAX);

    PoolVector3Array Vec3Arr;

    for (auto &&m : Mesh->Vertices)
        Vec3Arr.append(Vector3(m.x, m.y, m.z));
    arr[ArrayMesh::ARRAY_VERTEX] = Vec3Arr;
    
    PoolIntArray Indices;

    for (auto &&f : Mesh->Faces)
    {
        for (auto &&i : f->Indices)
        {
            Indices.append(i - 1);
        }
    }
    arr[ArrayMesh::ARRAY_INDEX] = Indices;
    Ret->add_surface_from_arrays(ArrayMesh::PRIMITIVE_TRIANGLES, arr);

    VoxelOptimizer::CWavefrontObjExporter obj;
    obj.SaveObj("test.obj", Mesh);

    return Ret;
}