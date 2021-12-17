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

#include <VoxelOptimizer/Meshers/GreedyMesher.hpp>
#include <VoxelOptimizer/Meshers/IMesher.hpp>
#include <VoxelOptimizer/Meshers/MarchingCubesMesher.hpp>
#include <VoxelOptimizer/Meshers/SimpleMesher.hpp>
namespace VoxelOptimizer
{
    Mesher IMesher::Create(MesherTypes type)
    {
        switch (type)
        {
            case MesherTypes::SIMPLE: return Mesher(new CSimpleMesher());
            case MesherTypes::GREEDY: return Mesher(new CGreedyMesher());
            case MesherTypes::MARCHING_CUBES: return Mesher(new CMarchingCubesMesher());
        }
    }

    void IMesher::ClearCache()
    {
        m_Index.clear();
        m_NormalIndex.clear();
        m_UVIndex.clear();
        m_FacesIndex.clear();
    }

    void IMesher::AddFace(Mesh Mesh, SVertex v1, SVertex v2, SVertex v3)
    {
        int I1, I2, I3;
        I1 = AddVertex(Mesh, v1.Pos);
        I2 = AddVertex(Mesh, v2.Pos);
        I3 = AddVertex(Mesh, v3.Pos);

        GroupedFaces Faces;

        auto ITFaces = m_FacesIndex.find(v1.Material);
        if(ITFaces == m_FacesIndex.end())
        {
            Faces = GroupedFaces(new SGroupedFaces());
            Mesh->Faces.push_back(Faces);

            Faces->MaterialIndex = v1.Material;
            Faces->FaceMaterial = m_Loader->GetMaterials()[v1.Material];
            m_FacesIndex.insert({v1.Material, Faces});
        }
        else
            Faces = ITFaces->second;
            
        // CVector FaceNormal = (v2 - v1).Cross(v3 - v1).Normalize(); 
        int NormalIdx = AddNormal(Mesh, v1.Normal);
        int UVIdx1 = AddUV(Mesh, v1.UV);
        int UVIdx2 = AddUV(Mesh, v2.UV);
        int UVIdx3 = AddUV(Mesh, v3.UV);

        Faces->Indices.push_back(CVector(I1, NormalIdx, UVIdx1));
        Faces->Indices.push_back(CVector(I2, NormalIdx, UVIdx2));
        Faces->Indices.push_back(CVector(I3, NormalIdx, UVIdx3));

        // if(FaceNormal == Normal)
        // {
        //     Faces->Indices.push_back(CVector(I1, NormalIdx, UVIdx));
        //     Faces->Indices.push_back(CVector(I2, NormalIdx, UVIdx));
        //     Faces->Indices.push_back(CVector(I3, NormalIdx, UVIdx));
        // }
        // else
        // {
        //     Faces->Indices.push_back(CVector(I3, NormalIdx, UVIdx));
        //     Faces->Indices.push_back(CVector(I2, NormalIdx, UVIdx));
        //     Faces->Indices.push_back(CVector(I1, NormalIdx, UVIdx));
        // }
    }

    void IMesher::AddFace(Mesh Mesh, CVector v1, CVector v2, CVector v3, CVector v4, CVector Normal, int Color, int Material)
    {
        int I1, I2, I3, I4;
        I1 = AddVertex(Mesh, v1);
        I2 = AddVertex(Mesh, v2);
        I3 = AddVertex(Mesh, v3);
        I4 = AddVertex(Mesh, v4);

        GroupedFaces Faces;

        auto ITFaces = m_FacesIndex.find(Material);
        if(ITFaces == m_FacesIndex.end())
        {
            Faces = GroupedFaces(new SGroupedFaces());
            Mesh->Faces.push_back(Faces);

            Faces->MaterialIndex = Material;
            Faces->FaceMaterial = m_Loader->GetMaterials()[Material];
            m_FacesIndex.insert({Material, Faces});
        }
        else
            Faces = ITFaces->second;
            
        CVector FaceNormal = (v2 - v1).Cross(v3 - v1).Normalize(); 
        int NormalIdx = AddNormal(Mesh, Normal);
        int UVIdx = AddUV(Mesh, CVector(((float)(Color + 0.5f)) / Mesh->Textures[TextureType::DIFFIUSE]->Size().x, 0.5f, 0));

        if(FaceNormal == Normal)
        {
            Faces->Indices.push_back(CVector(I1, NormalIdx, UVIdx));
            Faces->Indices.push_back(CVector(I2, NormalIdx, UVIdx));
            Faces->Indices.push_back(CVector(I3, NormalIdx, UVIdx));

            Faces->Indices.push_back(CVector(I1, NormalIdx, UVIdx));
            Faces->Indices.push_back(CVector(I3, NormalIdx, UVIdx));
            Faces->Indices.push_back(CVector(I4, NormalIdx, UVIdx));
        }
        else
        {
            Faces->Indices.push_back(CVector(I3, NormalIdx, UVIdx));
            Faces->Indices.push_back(CVector(I2, NormalIdx, UVIdx));
            Faces->Indices.push_back(CVector(I1, NormalIdx, UVIdx));

            Faces->Indices.push_back(CVector(I4, NormalIdx, UVIdx));
            Faces->Indices.push_back(CVector(I3, NormalIdx, UVIdx));
            Faces->Indices.push_back(CVector(I1, NormalIdx, UVIdx));
        }
    }

    int IMesher::AddVertex(Mesh Mesh, CVector Vertex)
    {
        int Ret = 0;
        // auto End = Mesh->Vertices.data() + Mesh->Vertices.size();
        auto IT = m_Index.find(Vertex); //std::find(Mesh->Vertices.data(), End, Vertex);

        if(IT != m_Index.end())
            Ret = IT->second;
        else
        {
            Mesh->Vertices.push_back(Vertex);
            Ret = Mesh->Vertices.size();

            m_Index.insert({Vertex, Ret});
        }

        return Ret;
    }

    int IMesher::AddNormal(Mesh Mesh, CVector Normal)
    {
        int Ret = 0;
        // Normal = CVector(Normal.x, Normal.z, Normal.y);

        // auto End = Mesh->Vertices.data() + Mesh->Vertices.size();
        auto IT = m_NormalIndex.find(Normal); //std::find(Mesh->Vertices.data(), End, Vertex);

        if(IT != m_NormalIndex.end())
            Ret = IT->second;
        else
        {
            Mesh->Normals.push_back(Normal);
            Ret = Mesh->Normals.size();

            m_NormalIndex.insert({Normal, Ret});
        }

        return Ret;
    }

    int IMesher::AddUV(Mesh Mesh, CVector UV)
    {
        int Ret = 0;
        // Normal = CVector(Normal.x, Normal.z, Normal.y);

        // auto End = Mesh->Vertices.data() + Mesh->Vertices.size();
        auto IT = m_UVIndex.find(UV); //std::find(Mesh->Vertices.data(), End, Vertex);

        if(IT != m_UVIndex.end())
            Ret = IT->second;
        else
        {
            Mesh->UVs.push_back(UV);
            Ret = Mesh->UVs.size();

            m_UVIndex.insert({UV, Ret});
        }

        return Ret;
    }
} // namespace VoxelOptimizer
