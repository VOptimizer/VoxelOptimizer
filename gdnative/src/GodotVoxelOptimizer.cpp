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
#include <MeshInstance.hpp>

void CGodotVoxelOptimizer::_register_methods()
{
    register_method("load", &CGodotVoxelOptimizer::Load);
    register_method("save", &CGodotVoxelOptimizer::Save);
    register_method("save_slices", &CGodotVoxelOptimizer::SaveSlices);
    register_method("get_meshes", &CGodotVoxelOptimizer::GetMeshes);
    register_method("get_statistics", &CGodotVoxelOptimizer::GetStatistics);
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

    String Ext = Path.get_extension().to_lower();
    if(Ext == "vox")
        m_Loader = VoxelOptimizer::Loader(new VoxelOptimizer::CMagicaVoxelLoader());
    else if(Ext == "gox")
        m_Loader = VoxelOptimizer::Loader(new VoxelOptimizer::CGoxelLoader());
    else if(Ext == "kenshape")
        m_Loader = VoxelOptimizer::Loader(new VoxelOptimizer::CKenshapeLoader());
    
    try
    {
        m_Loader->Load((char*)Data.read().ptr(), Data.size());
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
    if(m_Meshes.empty())
    {
        ERR_PRINT("No mesh data to save.");
        return (godot_error)Error::ERR_INVALID_DATA;
    }

    Ref<File> VFile = File::_new();
    VoxelOptimizer::Exporter Exporter;

    String Ext = Path.get_extension().to_lower();
    VoxelOptimizer::ExporterTypes type;

    if(Ext == "gltf")
        type = VoxelOptimizer::ExporterTypes::GLTF;
    else if(Ext == "glb")
        type = VoxelOptimizer::ExporterTypes::GLB;
    else if(Ext == "obj")
        type = VoxelOptimizer::ExporterTypes::OBJ;
    else if(Ext == "escn")
        type = VoxelOptimizer::ExporterTypes::ESCN;
    else if(Ext == "ply")
        type = VoxelOptimizer::ExporterTypes::PLY;
    else
        return (godot_error)Error::ERR_FILE_UNRECOGNIZED;

    Exporter = VoxelOptimizer::IExporter::Create(type);

    Path = Path.get_basename();
    Exporter->SetExternaFilenames(std::string(Path.get_file().utf8().get_data()));
    auto Files = Exporter->Generate(m_Meshes);

    for (auto &&f : Files)
    {
        VFile->open(Path + String(".") + String(f.first.c_str()), File::WRITE);
        if(!VFile->is_open())
        {
            ERR_PRINT("Couldn't open file: " + Path);
            return (godot_error)Error::ERR_FILE_CANT_WRITE;
        }

        PoolByteArray Data;
        Data.resize(f.second.size());
        auto Writer = Data.write();
        memcpy(Writer.ptr(), f.second.data(), f.second.size());

        VFile->store_buffer(Data);
        VFile->close();
    }
    
    return (godot_error)Error::OK;
}

godot_error CGodotVoxelOptimizer::SaveSlices(String Path)
{
    if(m_Meshes.empty())
    {
        ERR_PRINT("No mesh data to save.");
        return (godot_error)Error::ERR_INVALID_DATA;
    }

    Ref<File> VFile = File::_new();
    VoxelOptimizer::CSpriteStackingExporter Exporter;

    auto models = m_Loader->GetModels();
    int count = 0;

    for (auto &&m : models)
    {
        auto Image = Exporter.Generate(m, m_Loader);

        if(models.size() > 1)
        {
            Path = Path.get_basename() + "_" + String::num(count) + "." + Path.get_extension();
            count++;
        }

        VFile->open(Path, File::WRITE);
        if(!VFile->is_open())
        {
            ERR_PRINT("Couldn't open file: " + Path);
            return (godot_error)Error::ERR_FILE_CANT_WRITE;
        }

        PoolByteArray Data;
        Data.resize(Image.size());
        auto Writer = Data.write();
        memcpy(Writer.ptr(), Image.data(), Image.size());

        VFile->store_buffer(Data);
        VFile->close();
    }
    
    return (godot_error)Error::OK;
}

Array CGodotVoxelOptimizer::GetMeshes(int mesherType)
{
    Array ret;

    if(m_Loader->GetModels().empty())
    {
        ERR_PRINT("No file loaded please call load before!");
        return ret;
    }

    VoxelOptimizer::Mesher Mesher = VoxelOptimizer::IMesher::Create((VoxelOptimizer::MesherTypes)mesherType);
    auto meshes = m_Loader->GetModels();

    m_BlockCount = 0;
    m_VerticesCount = 0;
    m_FacesCount = 0;
    for (auto &&m : meshes)
    {
        auto mesh = Mesher->GenerateMeshes(m, m_Loader).begin()->second;
        m_BlockCount += m->GetBlockCount();
        m_VerticesCount += mesh->Vertices.size();
        
        for (auto &&f : mesh->Faces)
            m_FacesCount += f->Indices.size() / 3;        

        Ref<ArrayMesh> tmpMesh = ArrayMesh::_new();
        Array arr;
        arr.resize(ArrayMesh::ARRAY_MAX);

        std::map<VoxelOptimizer::CVector, int> Index;

        PoolVector3Array Vertices, Normals;
        PoolVector2Array UVs;    
        PoolIntArray Indices;

        int Surface = 0;

        int TmpIndices[3];
        int VertexCounter = 0;

        Ref<Image> Img = Image::_new();
        Img->create(mesh->Texture.size(), 1, false, Image::Format::FORMAT_RGBA8);
        Img->lock();

        int x = 0;
        for (auto &&c : mesh->Texture)
        {
            Img->set_pixel(x, 0, Color(c.R / 255.f, c.G / 255.f, c.B / 255.f, c.A / 255.f));
            x++;
        }

        Img->unlock();

        Ref<ImageTexture> Tex = ImageTexture::_new();
        Tex->create_from_image(Img);

        for (auto &&f : mesh->Faces)
        {
            for (auto &&v : f->Indices)
            {
                auto IT = Index.find(v);
                if(IT != Index.end())
                    TmpIndices[VertexCounter] = IT->second;
                    // Indices.append(IT->second);
                else
                {
                    VoxelOptimizer::CVector Vertex, Normal, UV;
                    Vertex = mesh->Vertices[(size_t)v.x - 1];
                    Normal = mesh->Normals[(size_t)v.y - 1];
                    UV = mesh->UVs[(size_t)v.z - 1];

                    Vertices.append(Vector3(Vertex.x, Vertex.y, Vertex.z));
                    Normals.append(Vector3(Normal.x, Normal.y, Normal.z));
                    UVs.append(Vector2(UV.x, UV.y));

                    int Idx = Vertices.size() - 1;
                    Index.insert({v, Idx});
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
            // arr[ArrayMesh::ARRAY_TEX_UV2] = UVs2;
            arr[ArrayMesh::ARRAY_INDEX] = Indices;
            tmpMesh->add_surface_from_arrays(ArrayMesh::PRIMITIVE_TRIANGLES, arr);

            Ref<SpatialMaterial> Mat = SpatialMaterial::_new();

            Mat->set_metallic(f->FaceMaterial->Metallic);
            Mat->set_roughness(f->FaceMaterial->Roughness);
            Mat->set_emission_energy(f->FaceMaterial->Power);
            Mat->set_specular(f->FaceMaterial->Specular);
            Mat->set_refraction(f->FaceMaterial->IOR);
            Mat->set_texture(SpatialMaterial::TextureParam::TEXTURE_ALBEDO, Tex);

            if(f->FaceMaterial->Transparency != 0.0)
            {
                Mat->set_feature(SpatialMaterial::Feature::FEATURE_TRANSPARENT, true);
                Mat->set_albedo(Color(1, 1, 1, 1 - f->FaceMaterial->Transparency));
            }

            if(f->FaceMaterial->Power != 0.0)
            {
                Mat->set_feature(SpatialMaterial::Feature::FEATURE_EMISSION, true);
            }

            tmpMesh->surface_set_material(Surface, Mat);
            Surface++;

            Index.clear();
            Vertices = PoolVector3Array();
            Normals = PoolVector3Array();
            UVs = PoolVector2Array();
            // UVs2 = PoolVector2Array();
            Indices = PoolIntArray();
        }

        auto instance = MeshInstance::_new();
        instance->set_mesh(tmpMesh);

        // Worldspace
        if(meshes.size() > 1)
        {
            auto matrix = mesh->ModelMatrix;
            instance->set_transform(Transform(matrix.x.x, matrix.y.x, matrix.z.x,
                                              matrix.x.y, matrix.y.y, matrix.z.y,
                                              matrix.x.z, matrix.y.z, matrix.z.z,
                                              matrix.x.w, matrix.y.w, matrix.z.w));
        }

        ret.append(instance);
    }
    
    return ret;
}

Dictionary CGodotVoxelOptimizer::GetStatistics()
{
    Dictionary Ret;

    Ret["blocks"] = m_BlockCount;

    if(!m_Meshes.empty())
    {
        Ret["vertices"] = m_VerticesCount;
        Ret["faces"] = m_FacesCount;
    }
    else
    {
        Ret["vertices"] = 0;
        Ret["faces"] = 0;
    }

    return Ret;
}