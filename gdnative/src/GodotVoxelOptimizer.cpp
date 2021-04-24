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

#include <VoxelOptimizer/Exceptions.hpp>
#include <File.hpp>
#include <SpatialMaterial.hpp>
#include <GodotVoxelOptimizer.hpp>
#include <ImageTexture.hpp>
#include <Image.hpp>

void CGodotVoxelOptimizer::_register_methods()
{
    register_method("load", &CGodotVoxelOptimizer::Load);
    register_method("save", &CGodotVoxelOptimizer::Save);
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
    if(!VFile->is_open())
    {
        ERR_PRINT("Couldn't open file: " + Path);
        return (godot_error)Error::ERR_FILE_CANT_READ;
    }

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

godot_error CGodotVoxelOptimizer::Save(String Path)
{
    if(!m_Mesh)
    {
        ERR_PRINT("No mesh data to save.");
        return (godot_error)Error::ERR_INVALID_DATA;
    }

    Ref<File> VFile = File::_new();
    VoxelOptimizer::CWavefrontObjExporter Exporter;
    Exporter.SetMTLFileName(std::string(Path.get_basename().get_file().utf8().get_data()));
    auto Res = Exporter.GenerateObj(m_Mesh);

    VFile->open(Path, File::WRITE);
    if(!VFile->is_open())
    {
        ERR_PRINT("Couldn't open file: " + Path);
        return (godot_error)Error::ERR_FILE_CANT_WRITE;
    }

    auto ObjData = std::get<0>(Res);

    PoolByteArray Data;
    Data.resize(ObjData.size());
    auto Writer = Data.write();
    memcpy(Writer.ptr(), ObjData.data(), ObjData.size());

    VFile->store_buffer(Data);
    VFile->close();

    VFile->open(Path.replace(Path.get_file(), "materials.mtl"), File::WRITE);
    if(!VFile->is_open())
    {
        ERR_PRINT("Couldn't open file: " + Path);
        return (godot_error)Error::ERR_FILE_CANT_WRITE;
    }

    ObjData = std::get<1>(Res);

    PoolByteArray Data1;
    Data1.resize(ObjData.size());
    Writer = Data1.write();
    memcpy(Writer.ptr(), ObjData.data(), ObjData.size());

    VFile->store_buffer(Data1);
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

    VoxelOptimizer::Mesher Mesher;

    if(Optimized)
        Mesher = VoxelOptimizer::Mesher(new VoxelOptimizer::CGreedyMesher());
    else
        Mesher = VoxelOptimizer::Mesher(new VoxelOptimizer::CSimpleMesher());

    m_Mesh = Mesher->GenerateMesh(m_Loader.GetModels().front(), m_Loader);

    Ref<ArrayMesh> Ret = ArrayMesh::_new();
    Array arr;
    arr.resize(ArrayMesh::ARRAY_MAX);

    std::map<size_t, int> m_Index;

    PoolVector3Array Vertices, Normals;
    PoolVector2Array UVs;

    // for (auto &&m : Mesh->Vertices)
    //     Vec3Arr.append(Vector3(m.x, m.y, m.z));
    // arr[ArrayMesh::ARRAY_VERTEX] = Vec3Arr;
    
    PoolIntArray Indices;

    int Surface = 0;

    int TmpIndices[3];
    int VertexCounter = 0;

    Ref<Image> Img = Image::_new();
    Img->create(m_Mesh->Texture.size(), 1, false, Image::Format::FORMAT_RGBA8);
    Img->lock();

    int x = 0;
    for (auto &&c : m_Mesh->Texture)
    {
        Img->set_pixel(x, 0, Color(c.R / 255.f, c.G / 255.f, c.B / 255.f, c.A / 255.f));
        x++;
    }

    Img->unlock();

    Ref<ImageTexture> Tex = ImageTexture::_new();
    Tex->create_from_image(Img);

    for (auto &&f : m_Mesh->Faces)
    {
        for (auto &&v : f->Indices)
        {
            size_t Hash = v.hash();
            auto IT = m_Index.find(Hash);
            if(IT != m_Index.end())
                TmpIndices[VertexCounter] = IT->second;
                // Indices.append(IT->second);
            else
            {
                VoxelOptimizer::CVector Vertex, Normal, UV;
                Vertex = m_Mesh->Vertices[(size_t)v.x - 1];
                Normal = m_Mesh->Normals[(size_t)v.y - 1];
                UV = m_Mesh->Normals[(size_t)v.z - 1];

                Vertices.append(Vector3(Vertex.x, Vertex.y, Vertex.z));
                Normals.append(Vector3(Normal.x, Normal.y, Normal.z));
                UVs.append(Vector2(UV.x, UV.y));

                int Idx = Vertices.size() - 1;
                m_Index.insert({Hash, Idx});
                TmpIndices[VertexCounter] = Idx;
            }

            VertexCounter++;
            if(VertexCounter == 3)
            {
                VertexCounter = 0;

                for (char i = 2; i > -1; i--)
                    Indices.append(TmpIndices[i]);
            }
        }

        arr[ArrayMesh::ARRAY_VERTEX] = Vertices;
        arr[ArrayMesh::ARRAY_NORMAL] = Normals;
        arr[ArrayMesh::ARRAY_TEX_UV] = UVs;
        arr[ArrayMesh::ARRAY_INDEX] = Indices;
        Ret->add_surface_from_arrays(ArrayMesh::PRIMITIVE_TRIANGLES, arr);

        Ref<SpatialMaterial> Mat = SpatialMaterial::_new();

        Mat->set_metallic(f->FaceMaterial->Metallic);
        Mat->set_roughness(f->FaceMaterial->Roughness);
        Mat->set_emission_energy(f->FaceMaterial->Power);
        Mat->set_specular(f->FaceMaterial->Specular);
        Mat->set_refraction(f->FaceMaterial->IOR);
        Mat->set_texture(SpatialMaterial::TextureParam::TEXTURE_ALBEDO, Tex);

        Ret->surface_set_material(Surface, Mat);
        Surface++;

        m_Index.clear();
        Vertices = PoolVector3Array();
        Normals = PoolVector3Array();
        Indices = PoolIntArray();
    }


    // arr[ArrayMesh::ARRAY_VERTEX] = Vertices;
    // arr[ArrayMesh::ARRAY_NORMAL] = Normals;
    // arr[ArrayMesh::ARRAY_INDEX] = Indices;
    // Ret->add_surface_from_arrays(ArrayMesh::PRIMITIVE_TRIANGLES, arr);
    // VoxelOptimizer::CWavefrontObjExporter obj;
    // obj.SaveObj("test.obj", Mesh);

    return Ret;
}