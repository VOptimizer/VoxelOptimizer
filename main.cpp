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

    VoxelOptimizer::CVector v1(0, 0.5, 1);
    VoxelOptimizer::CVector v2(0, -1, 0);
    VoxelOptimizer::CVector v3(1, 1, 1);

    VoxelOptimizer::CVector n = (v2 - v1).Cross(v3 - v1).Normalize(); 

    auto res1 = v1.Length() * v1.Dot(n); //n * (v1.Dot(n) / n.Dot(n));
    auto res2 = v2.Length() * v2.Dot(n); //n * (v2.Dot(n) / n.Dot(n));
    auto res3 = v3.Length() * v3.Dot(n); //n * (v3.Dot(n) / n.Dot(n));

    VoxelOptimizer::Loader loader(new VoxelOptimizer::CMagicaVoxelLoader());

    auto start = chrono::system_clock::now();
    loader->Load("windmill.vox");
    auto end = chrono::system_clock::now();
    auto LoadTime = end - start;

    // VoxelOptimizer::CGreedyMesher Mesher;
    VoxelOptimizer::CSimpleMesher Mesher;

    auto VoxelMesh = loader->GetModels().back();//loader.GetModels().back();
    cout << VoxelMesh->GetBlockCount() << endl;
    VoxelMesh->RemeshAlways(true);

    // start = chrono::system_clock::now();
    // DummyMesher(VoxelMesh, loader);
    // end = chrono::system_clock::now();
    // auto DummyTime = end - start;

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

    auto voxels = loader->GetModels();
    int counter = 0;

    std::vector<VoxelOptimizer::Mesh> meshes;

    for (auto &&v : voxels)
    {
        // VoxelOptimizer::CWavefrontObjExporter exporterObj;
        // exporterObj.SetBinary(true);

        v->RemeshAlways(true);

        auto Meshes = Mesher.GenerateMeshes(v, loader);
        auto Mesh = Meshes.begin()->second; 
        meshes.push_back(Mesh);


        // exporterObj.Save(std::to_string(counter) + ".glb", Mesh);
        // counter++;
    }

    VoxelOptimizer::CMarchingCubesMesher mc;
    auto mcMesh = mc.GenerateMeshes(voxels.front(), loader);
    
    end = chrono::system_clock::now();
    auto RealTime = end - start;

    VoxelOptimizer::CWavefrontObjExporter objExporter;
    VoxelOptimizer::CPLYExporter plyExporter;
    VoxelOptimizer::CGLTFExporter gltfExporter;
    VoxelOptimizer::CGodotSceneExporter escnExporter;

    objExporter.Settings()->WorldSpace = false;
    gltfExporter.Settings()->WorldSpace = true;
    escnExporter.Settings()->WorldSpace = true;
    plyExporter.Settings()->WorldSpace = true;

    objExporter.Save("object_rot.obj", meshes);
    gltfExporter.Save("object_rot.gltf", meshes);
    escnExporter.Save("object_rot.esn", meshes);
    plyExporter.Save("object_rot.ply", meshes);

    objExporter.Settings()->WorldSpace = false;
    objExporter.Save("mc.obj", mcMesh.begin()->second);

    gltfExporter.Settings()->WorldSpace = false;
    gltfExporter.Save("mc.gltf", mcMesh.begin()->second);

    auto mesh = VoxelOptimizer::Mesh(new VoxelOptimizer::SMesh());

    // mesh->Vertices.push_back(VoxelOptimizer::CVector(-0.5, 0.5, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0, 0.5, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0.5, 0.5, 0));

    // mesh->Vertices.push_back(VoxelOptimizer::CVector(-0.5, 0, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0, 0, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0.5, 0, 0));

    // mesh->Vertices.push_back(VoxelOptimizer::CVector(-0.5, -0.5, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0, -0.5, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0.5, -0.5, 0));

    // mesh->UVs.push_back(VoxelOptimizer::CVector(0, 0, 0));
    // mesh->Normals.push_back(VoxelOptimizer::CVector(0, 0, 1));
    // mesh->Texture.push_back(VoxelOptimizer::CColor(1, 0, 0, 0));
    // mesh->Faces.push_back(VoxelOptimizer::GroupedFaces(new VoxelOptimizer::SGroupedFaces()));

    // mesh->Faces[0]->FaceMaterial = 0;
    // mesh->Faces[0]->FaceMaterial = VoxelOptimizer::Material(new VoxelOptimizer::CMaterial());
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(1, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(2, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(4, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(2, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(5, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(4, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(2, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(3, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(5, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(3, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(5, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(4, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(5, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(7, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(5, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(8, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(7, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(5, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(8, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(9, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(8, 1, 1));




    // mesh->Vertices.push_back(VoxelOptimizer::CVector(-0.5, 0.5, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0, 0.5, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0.5, 0.5, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(1, 0.5, 0));

    // mesh->Vertices.push_back(VoxelOptimizer::CVector(-0.5, 0, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0, 0, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0.5, 0, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(1, 0, 0));

    // mesh->Vertices.push_back(VoxelOptimizer::CVector(-0.5, -0.5, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0, -0.5, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0.5, -0.5, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(1, -0.5, 0));

    // mesh->Vertices.push_back(VoxelOptimizer::CVector(-0.5, -1, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0, -1, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(0.5, -1, 0));
    // mesh->Vertices.push_back(VoxelOptimizer::CVector(1, -1, 0));

    // mesh->UVs.push_back(VoxelOptimizer::CVector(0, 0, 0));
    // mesh->Normals.push_back(VoxelOptimizer::CVector(0, 0, 1));
    // mesh->Texture.push_back(VoxelOptimizer::CColor(1, 0, 0, 0));
    // mesh->Faces.push_back(VoxelOptimizer::GroupedFaces(new VoxelOptimizer::SGroupedFaces()));

    // mesh->Faces[0]->FaceMaterial = 0;
    // mesh->Faces[0]->FaceMaterial = VoxelOptimizer::Material(new VoxelOptimizer::CMaterial());
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(1, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(2, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(5, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(2, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(5, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(2, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(3, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(3, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(7, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(3, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(4, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(7, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(4, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(8, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(7, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(5, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(9, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(10, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(9, 1, 1));

    // //6
    // //10
    // //7
    // //11

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(7, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(8, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(11, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(8, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(12, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(11, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(9, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(10, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(13, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(10, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(14, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(13, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(10, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(11, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(14, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(11, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(15, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(14, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(11, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(12, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(15, 1, 1));

    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(12, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(16, 1, 1));
    // mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(15, 1, 1));


    mesh->Vertices.push_back(VoxelOptimizer::CVector(-3.000000, 0.000000, 0.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-3.000000, 0.000000, 1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-2.000000, 0.000000, 1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-2.000000, 0.000000, 0.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-3.000000, 0.000000, -1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-2.000000, 0.000000, -1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-2.000000, 0.000000, 2.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-1.000000, 0.000000, 2.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-1.000000, 0.000000, 1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-1.000000, 0.000000, 0.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-1.000000, 0.000000, -1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-2.000000, 0.000000, -2.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-1.000000, 0.000000, -2.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-1.000000, 0.000000, 3.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(0.000000, 0.000000, 3.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(0.000000, 0.000000, 2.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(0.000000, 0.000000, 1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(0.000000, 0.000000, 0.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(0.000000, 0.000000, -1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(0.000000, 0.000000, -2.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(-1.000000, 0.000000, -3.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(0.000000, 0.000000, -3.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(1.000000, 0.000000, 3.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(1.000000, 0.000000, 2.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(1.000000, 0.000000, 1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(1.000000, 0.000000, 0.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(1.000000, 0.000000, -1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(1.000000, 0.000000, -2.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(1.000000, 0.000000, -3.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(2.000000, 0.000000, 2.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(2.000000, 0.000000, 1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(2.000000, 0.000000, 0.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(2.000000, 0.000000, -1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(2.000000, 0.000000, -2.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(3.000000, 0.000000, 1.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(3.000000, 0.000000, 0.000000));
    mesh->Vertices.push_back(VoxelOptimizer::CVector(3.000000, 0.000000, -1.000000));

    mesh->UVs.push_back(VoxelOptimizer::CVector(0.500000, 0.500000, 0));
    mesh->Normals.push_back(VoxelOptimizer::CVector(0, 1, 0));
    mesh->Texture.push_back(VoxelOptimizer::CColor(1, 0, 0, 0));
    mesh->Faces.push_back(VoxelOptimizer::GroupedFaces(new VoxelOptimizer::SGroupedFaces()));

    mesh->Faces[0]->FaceMaterial = 0;
    mesh->Faces[0]->FaceMaterial = VoxelOptimizer::Material(new VoxelOptimizer::CMaterial());

    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(1, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(2, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(3, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(1, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(3, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(4, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(5, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(1, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(4, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(5, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(4, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(3, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(7, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(8, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(3, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(8, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(9, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(4, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(3, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(9, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(4, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(9, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(10, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(4, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(10, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(10, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(11, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(12, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(6, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(11, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(12, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(11, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(13, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(8, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(14, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(15, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(8, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(15, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(16, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(9, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(8, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(16, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(9, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(16, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(17, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(10, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(9, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(17, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(10, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(17, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(18, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(11, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(10, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(18, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(11, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(18, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(19, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(13, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(11, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(19, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(13, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(19, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(20, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(21, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(13, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(20, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(21, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(20, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(22, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(16, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(15, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(23, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(16, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(23, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(24, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(17, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(16, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(24, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(17, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(24, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(25, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(18, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(17, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(25, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(18, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(25, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(26, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(19, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(18, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(26, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(19, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(26, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(27, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(20, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(19, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(27, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(20, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(27, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(28, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(22, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(20, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(28, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(22, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(28, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(29, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(25, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(24, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(30, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(25, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(30, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(31, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(26, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(25, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(31, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(26, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(31, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(32, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(27, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(26, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(32, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(27, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(32, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(33, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(28, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(27, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(33, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(28, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(33, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(34, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(32, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(31, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(35, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(32, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(35, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(36, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(33, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(32, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(36, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(33, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(36, 1, 1));
    mesh->Faces[0]->Indices.push_back(VoxelOptimizer::CVector(37, 1, 1));


    gltfExporter.Settings()->WorldSpace = false;
    gltfExporter.Save("face.gltf", mesh);

    VoxelOptimizer::CVerticesReducer reducer;
    auto reducedMesh = reducer.Reduce(mcMesh.begin()->second); //meshes.front()); //mcMesh.begin()->second);//meshes.front()); //mesh);//

    gltfExporter.Settings()->WorldSpace = false;
    gltfExporter.Save("reduced.gltf", reducedMesh);

    cout << "Load: " << std::chrono::duration_cast<std::chrono::milliseconds>(LoadTime).count() << endl;
    // cout << "Dummy: " << std::chrono::duration_cast<std::chrono::milliseconds>(DummyTime).count() << endl;
    cout << "Real: " << std::chrono::duration_cast<std::chrono::milliseconds>(RealTime).count() << endl;

    // Mesh = VoxelOptimizer::Mesh(new VoxelOptimizer::SMesh());

    // Mesh->Vertices.push_back()

    // VoxelOptimizer::CWavefrontObjExporter exporter1;
    // exporter1.SaveObj("minicube.obj", Mesh);

    // VoxelOptimizer::CGLTFExporter exporter;

    // // exporter.Save("lantern.gltf", Mesh);
    // exporter.Save("windmill.glb", Mesh);

    // VoxelOptimizer::CWavefrontObjExporter exporterObj;

    // // exporter.Save("lantern.gltf", Mesh);
    // exporterObj.Save("windmill.obj", Mesh);

    // // VoxelOptimizer::CSpriteStackingExporter spriter;
    // // spriter.Save("windmill_sprite.png", VoxelMesh, KLoader);

    // VoxelOptimizer::CGodotSceneExporter godot;
    // godot.Save("windmill_2.escn", Mesh);

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