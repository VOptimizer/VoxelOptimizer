#include <iostream>
#include <VoxelOptimizer/VoxelOptimizer.hpp>
#include <fstream>

using namespace std;

int main(int argc, char const *argv[])
{
    VoxelOptimizer::CMagicaVoxelLoader loader;
    loader.Load("lantern.vox");

    VoxelOptimizer::CGreedyMesher Mesher;

    auto VoxelMesh = loader.GetModels().back();

    auto v = VoxelMesh->GetVoxels();

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
    

    auto Mesh = Mesher.GenerateMesh(VoxelMesh, loader);

    // VoxelOptimizer::CWavefrontObjExporter exporter1;
    // exporter1.SaveObj("minicube.obj", Mesh);

    VoxelOptimizer::CGLTFExporter exporter;

    // exporter.Save("lantern.gltf", Mesh);
    exporter.Save("lantern.glb", Mesh);

    return 0;
}
