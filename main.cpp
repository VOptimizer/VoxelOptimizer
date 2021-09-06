#include <iostream>
#include <VoxelOptimizer/VoxelOptimizer.hpp>
#include <fstream>
#include <chrono>

using namespace std;

void DummyMesher(VoxelOptimizer::VoxelMesh m, VoxelOptimizer::Loader Loader);

int main(int argc, char const *argv[])
{
    // VoxelOptimizer::Loader GLoader(new VoxelOptimizer::CGoxelLoader());
    // GLoader->Load("windmill.gox");

    // VoxelOptimizer::Loader KLoader(new VoxelOptimizer::CKenshapeLoader());
    // KLoader->Load("bottle-potion.kenshape");

    VoxelOptimizer::Loader loader(new VoxelOptimizer::CMagicaVoxelLoader());

    auto start = chrono::system_clock::now();
    loader->Load("teapot.vox");
    auto end = chrono::system_clock::now();
    auto LoadTime = end - start;

    // VoxelOptimizer::CGreedyMesher Mesher;
    VoxelOptimizer::CSimpleMesher Mesher;

    auto VoxelMesh = loader->GetModels().back();//loader.GetModels().back();
    cout << VoxelMesh->GetBlockCount() << endl;

    start = chrono::system_clock::now();
    DummyMesher(VoxelMesh, loader);
    end = chrono::system_clock::now();
    auto DummyTime = end - start;

    // auto v = VoxelMesh->GetVoxels();

    //Pos.x + m_Size.x * Pos.y + m_Size.x * m_Size.y * Pos.z

    // cout << "Front Bottom Left Backward (0, 0, 0): " << v[0 + 3 * 0 + 3 * 3 * 0]->Material << endl;
    // cout << "Front Bottom Right Backward (2, 0, 0): " << v[2 + 3 * 0 + 3 * 3 * 0]->Material << endl;

    // cout << "Front Bottom Left Forward (0, 2, 0): " << v[0 + 3 * 2 + 3 * 3 * 0]->Material << endl;
    // cout << "Front Bottom Right Backward (2, 2, 0): " << v[2 + 3 * 2 + 3 * 3 * 0]->Material << endl;

    // cout << "Front Top Left Backward (0, 0, 2): " << v[0 + 3 * 0 + 3 * 3 * 2]->Material << endl;
    // cout << "Front Top Right Backward (2, 0, 2): " << v[2 + 3 * 0 + 3 * 3 * 2]->Material << endl;

    // cout << "Front Top Left Forward (0, 2, 2): " << v[0 + 3 * 2 + 3 * 3 * 2]->Material << endl;
    // cout << "Front Top Right Backward (2, 2, 2): " << v[2 + 3 * 2 + 3 * 3 * 2]->Material << endl;

    // for (auto &&e : VoxelMesh->GetVoxels())
    // {
    //     cout << "Voxel: " << e->Pos.x << " " << e->Pos.y << " " << e->Pos.z << endl;
    //     for (auto &&v : e->Normals)
    //     {
    //         cout << "\t" << v.x << " " << v.y << " " << v.z << endl;
    //     }        
    // }

    start = chrono::system_clock::now();
    auto Mesh = Mesher.GenerateMesh(VoxelMesh, loader);
    end = chrono::system_clock::now();
    auto RealTime = end - start;

    

    cout << "Load: " << std::chrono::duration_cast<std::chrono::milliseconds>(LoadTime).count() << endl;
    cout << "Dummy: " << std::chrono::duration_cast<std::chrono::milliseconds>(DummyTime).count() << endl;
    cout << "Real: " << std::chrono::duration_cast<std::chrono::milliseconds>(RealTime).count() << endl;

    // Mesh = VoxelOptimizer::Mesh(new VoxelOptimizer::SMesh());

    // Mesh->Vertices.push_back()

    // VoxelOptimizer::CWavefrontObjExporter exporter1;
    // exporter1.SaveObj("minicube.obj", Mesh);

    VoxelOptimizer::CGLTFExporter exporter;

    // exporter.Save("lantern.gltf", Mesh);
    exporter.Save("windmill.glb", Mesh);

    VoxelOptimizer::CWavefrontObjExporter exporterObj;

    // exporter.Save("lantern.gltf", Mesh);
    exporterObj.Save("windmill.obj", Mesh);

    // VoxelOptimizer::CSpriteStackingExporter spriter;
    // spriter.Save("windmill_sprite.png", VoxelMesh, KLoader);

    VoxelOptimizer::CGodotSceneExporter godot;
    godot.Save("windmill_2.escn", Mesh);

    return 0;
}

void DummyMesher(VoxelOptimizer::VoxelMesh m, VoxelOptimizer::Loader Loader)
{
    auto Voxels = m->GetVoxels();
    auto BBox = m->GetBBox();
    VoxelOptimizer::CVector Beg = BBox.Beg;
    std::swap(Beg.y, Beg.z);

    VoxelOptimizer::CVector BoxCenter = BBox.GetSize() / 2;
    std::swap(BoxCenter.y, BoxCenter.z);

    for (auto &&pair : Voxels)
    {          
        VoxelOptimizer::Voxel v = pair.second;

        if(v && v->IsVisible())
        {
            for (char i = 0; i < v->Normals.size(); i++)
            {
                // Invisible
                if(v->Normals[i] == VoxelOptimizer::CVector(0, 0, 0))
                    continue;

                VoxelOptimizer::CVector v1, v2, v3, v4, Normal = v->Normals[i];
                std::swap(Normal.y, Normal.z);
                
                switch (i)
                {
                    case VoxelOptimizer::CVoxel::Direction::UP:
                    case VoxelOptimizer::CVoxel::Direction::DOWN:
                    {
                        float PosZ = v->Normals[i].z < 0 ? 0 : v->Normals[i].z;

                        v1 = VoxelOptimizer::CVector(v->Pos.x, v->Pos.z + PosZ, v->Pos.y + 1.f) - BoxCenter;
                        v2 = VoxelOptimizer::CVector(v->Pos.x, v->Pos.z + PosZ, v->Pos.y) - BoxCenter;
                        v3 = VoxelOptimizer::CVector(v->Pos.x + 1.f, v->Pos.z + PosZ, v->Pos.y) - BoxCenter;
                        v4 = VoxelOptimizer::CVector(v->Pos.x + 1.f, v->Pos.z + PosZ, v->Pos.y + 1.f) - BoxCenter;
                    }break;

                    case VoxelOptimizer::CVoxel::Direction::LEFT:
                    case VoxelOptimizer::CVoxel::Direction::RIGHT:
                    {
                        float Posx = v->Normals[i].x < 0 ? 0 : v->Normals[i].x;

                        v1 = VoxelOptimizer::CVector(v->Pos.x + Posx, v->Pos.z, v->Pos.y) - BoxCenter;
                        v2 = VoxelOptimizer::CVector(v->Pos.x + Posx, v->Pos.z, v->Pos.y + 1.f) - BoxCenter;
                        v3 = VoxelOptimizer::CVector(v->Pos.x + Posx, v->Pos.z + 1.f, v->Pos.y + 1.f) - BoxCenter;
                        v4 = VoxelOptimizer::CVector(v->Pos.x + Posx, v->Pos.z + 1.f, v->Pos.y) - BoxCenter;
                    }break;

                    case VoxelOptimizer::CVoxel::Direction::FORWARD:
                    case VoxelOptimizer::CVoxel::Direction::BACKWARD:
                    {
                        float PosY = v->Normals[i].y < 0 ? 0 : v->Normals[i].y;

                        v1 = VoxelOptimizer::CVector(v->Pos.x, v->Pos.z + 1.f, v->Pos.y + PosY) - BoxCenter;
                        v2 = VoxelOptimizer::CVector(v->Pos.x, v->Pos.z, v->Pos.y + PosY) - BoxCenter;
                        v3 = VoxelOptimizer::CVector(v->Pos.x + 1.f, v->Pos.z, v->Pos.y + PosY) - BoxCenter;
                        v4 = VoxelOptimizer::CVector(v->Pos.x + 1.f, v->Pos.z + 1.f, v->Pos.y + PosY) - BoxCenter;
                    }break;
                }

                // AddFace(Ret, v1 - Beg, v2 - Beg, v3 - Beg, v4 - Beg, Normal, v->Color, v->Material);
            }
        }
    }

    // ClearCache();
    // return Ret;
}