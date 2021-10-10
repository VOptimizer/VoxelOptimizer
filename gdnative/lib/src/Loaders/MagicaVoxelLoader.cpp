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

#include <string.h>
#include <VoxelOptimizer/Loaders/MagicaVoxelLoader.hpp>
#include <VoxelOptimizer/Exceptions.hpp>
#include <stack>
#include <sstream>

namespace VoxelOptimizer
{
    // Copied from the official documentation. https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox.txt
    const static unsigned int default_palette[256] = {
        0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
        0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff, 0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
        0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
        0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
        0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc, 0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
        0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
        0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399, 0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66,
        0xff66ff66, 0xff33ff66, 0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66, 0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666,
        0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
        0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933,
        0xff669933, 0xff339933, 0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633, 0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033,
        0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
        0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300,
        0xff663300, 0xff333300, 0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000, 0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044,
        0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
        0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111
    };

    void CMagicaVoxelLoader::ParseFormat()
    {
        m_Models.clear();
        LoadDefaultPalette();
        m_Index = 0;
        m_UsedColorsPos = 0;
        m_Materials.clear();
        m_ColorMapping.clear();
        m_MaterialMapping.clear();
        m_Materials.clear();
        m_UsedColorPalette.clear();
       
        std::string Signature(4, '\0');
        ReadData(&Signature[0], 4);
        Signature += "\0";

        // Checks the file header
        if(Signature != "VOX ")
            throw CVoxelLoaderException("Unknown file format");

        int Version = ReadData<int>();
        if(Version != 150)
            throw CVoxelLoaderException("Version: " + std::to_string(Version) + " is not supported");

        // First processes the materials that are at the end of the file. 
        ProcessMaterialAndSceneGraph();
        Skip(8);

        if(!IsEof())
        {
            SChunkHeader Tmp = ReadData<SChunkHeader>();
            if(strncmp(Tmp.ID, "MAIN", sizeof(Tmp.ID)) == 0)
            {
                while (!IsEof())
                {
                    Tmp = ReadData<SChunkHeader>();

                    if(strncmp(Tmp.ID, "SIZE", sizeof(Tmp.ID)) == 0)
                    {
                        VoxelMesh m = ProcessSize();
                        Tmp = ReadData<SChunkHeader>();
                        if(strncmp(Tmp.ID, "XYZI", sizeof(Tmp.ID)) != 0)
                            throw CVoxelLoaderException("Can't understand the format.");

                        ProcessXYZI(m);

                        m_Models.push_back(m);
                        auto halfSize = (m->GetSize() / 2.0);

                        // 1. Translation is always relative to the center of the voxel space size. (Without fraction)
                        // 2. All meshers centers the mehs in object space so we need to add the fractal part to the translation
                        // 3. Add the distance from object center to space center and add the start position of the voxel mesh

                        CVector spaceCenter = halfSize.Fract() + m->GetBBox().Beg + (m->GetBBox().GetSize() / 2 - halfSize);

                        auto modelMatrix = m_ModelMatrices.at(m_Models.size() - 1);

                        // TODO: This is dumb! The model matrix should be created on a central point!
                        modelMatrix += CMat4x4(CVector4(0, 0, 0, spaceCenter.x),
                                               CVector4(0, 0, 0, spaceCenter.z),
                                               CVector4(0, 0, 0, -spaceCenter.y),
                                               CVector4(0, 0, 0, 0));

                        m->SetModelMatrix(modelMatrix);
                    }
                    else if(strncmp(Tmp.ID, "RGBA", sizeof(Tmp.ID)) == 0)
                    {
                        for (size_t i = 0; i < m_ColorPalette.size(); i++)
                            ReadData((char*)m_ColorPalette[i].c, 4);
                    }
                    else
                        Skip(Tmp.ChunkContentSize + Tmp.ChildChunkSize);
                }
            }
        }

        // Creates the used color palette.
        m_UsedColorPalette.resize(m_ColorMapping.size());
        for (auto &&c : m_ColorMapping)
            m_UsedColorPalette[c.second] = m_ColorPalette[c.first - 1];
    }

    void CMagicaVoxelLoader::LoadDefaultPalette()
    {
        m_ColorPalette.resize(256);
        for (size_t i = 0; i < m_ColorPalette.size(); i++)
        {
            memcpy(m_ColorPalette[i].c, &default_palette[i], 4);
        }
    }

    VoxelMesh CMagicaVoxelLoader::ProcessSize()
    {
        VoxelMesh Ret = VoxelMesh(new CVoxelMesh());

        CVector Size;

        Size.x = ReadData<int>();
        Size.y = ReadData<int>();
        Size.z = ReadData<int>();

        Ret->SetSize(Size);

        return Ret;
    }

    void CMagicaVoxelLoader::ProcessXYZI(VoxelMesh m)
    {
        int VoxelCount = ReadData<int>();

        CVector Beg(1000, 1000, 1000), End;

        for (size_t i = 0; i < VoxelCount; i++)
        {
            CVector vec;

            vec.x = ReadData<char>();
            vec.y = ReadData<char>();
            vec.z = ReadData<char>();

            Beg = Beg.Min(vec);
            End = End.Max(vec);

            int MatIdx = ReadData<uint8_t>();
            int Color = 0;
            bool Transparent = false;

            // Remaps the indices.
            auto IT = m_ColorMapping.find(MatIdx);
            if(IT == m_ColorMapping.end())
            {
                m_ColorMapping.insert({MatIdx, m_UsedColorsPos});
                Color = m_UsedColorsPos;
                m_UsedColorsPos++;
            }
            else
                Color = IT->second;

            IT = m_MaterialMapping.find(MatIdx);
            if(IT == m_MaterialMapping.end())
                MatIdx = 0;
            else
            {
                MatIdx = IT->second;
                Transparent = m_Materials[MatIdx]->Transparency != 0.0;
                if(Transparent)
                {
                    int k= 0;
                    k++;
                }
            }

            m->SetVoxel(vec, MatIdx, Color, Transparent);
        } 

        m->SetBBox(CBBox(Beg, End));
    }

    void CMagicaVoxelLoader::ProcessMaterialAndSceneGraph()
    {
        std::map<int, Node> nodes;
        m_Materials.push_back(Material(new CMaterial()));

        if(!IsEof())
        {
            SChunkHeader Tmp = ReadData<SChunkHeader>();
            if(strncmp(Tmp.ID, "MAIN", sizeof(Tmp.ID)) == 0)
            {
                while (!IsEof())
                {
                    Tmp = ReadData<SChunkHeader>();

                    if(strncmp(Tmp.ID, "MATL", sizeof(Tmp.ID)) == 0)
                    {
                        Material Mat = Material(new CMaterial());
                        int ID = ReadData<int>();
                        int KeyValueCount = ReadData<int>();

                        std::string MaterialType;

                        for (int i = 0; i < KeyValueCount; i++)
                        {
                            int StrLen = ReadData<int>();

                            std::string Key(StrLen, '\0'); 
                            ReadData(&Key[0], StrLen);

                            if(Key == "_plastic")
                                continue;

                            StrLen = ReadData<int>();

                            std::string Value(StrLen, '\0'); 
                            ReadData(&Value[0], StrLen);

                            if(Key == "_type")
                                MaterialType = Value;
                            else if(Key == "_metal")
                                Mat->Metallic = std::stof(Value);
                            else if(Key == "_alpha")
                                Mat->Transparency = std::stof(Value);     
                            else if(Key == "_rough")
                                Mat->Roughness = std::stof(Value);
                            else if(Key == "_spec")
                                Mat->Specular = std::stof(Value);
                            else if(Key == "_ior")
                                Mat->IOR = std::stof(Value);
                            else if(Key == "_flux")
                                Mat->Power = std::stof(Value);   
                        }

                        if(MaterialType == "_diffuse" || MaterialType.empty())
                            continue;
                        
                        m_Materials.push_back(Mat);
                        m_MaterialMapping.insert({ID, m_Materials.size() - 1});
                    }
                    else if(strncmp(Tmp.ID, "nTRN", sizeof(Tmp.ID)) == 0)
                    {
                        auto tmp = ProcessTransformNode();
                        nodes.insert({tmp->NodeID, tmp});
                    }
                    else if(strncmp(Tmp.ID, "nGRP", sizeof(Tmp.ID)) == 0)
                    {
                        auto tmp = ProcessGroupNode();
                        nodes.insert({tmp->NodeID, tmp});
                    }
                    else if(strncmp(Tmp.ID, "nSHP", sizeof(Tmp.ID)) == 0)
                    {
                        auto tmp = ProcessShapeNode();
                        nodes.insert({tmp->NodeID, tmp});
                    }
                    else
                        Skip(Tmp.ChunkContentSize + Tmp.ChildChunkSize);
                }
            }
        }

        std::stack<int> nodeIDs;
        std::stack<CVector> vectors;
        std::stack<CMat4x4> rotations;
        CVector currentTranslation;
        CMat4x4 currentRotation;

        nodeIDs.push(0);
        while (!nodeIDs.empty())
        {
            Node tmp = nodes[nodeIDs.top()];

            switch (tmp->Type)
            {
                case NodeType::TRANSFORM:
                {
                    auto transform = std::static_pointer_cast<STransformNode>(tmp);
                    nodeIDs.pop();

                    currentTranslation += transform->Translation;
                    currentRotation *= transform->Rotation;

                    nodeIDs.push(transform->ChildID);
                } break;

                case NodeType::GROUP:
                {
                    auto group = std::static_pointer_cast<SGroupNode>(tmp);
                    if(group->ChildIdx > 0)
                    {
                        currentTranslation = vectors.top();
                        currentRotation = rotations.top();
                        vectors.pop();
                        rotations.pop();
                    }

                    if(group->ChildIdx < group->ChildrensID.size())
                    {
                        vectors.push(currentTranslation);
                        rotations.push(currentRotation);

                        nodeIDs.push(group->ChildrensID[group->ChildIdx]);
                        group->ChildIdx++;
                    }
                    else                     
                        nodeIDs.pop();
                } break;

                case NodeType::SHAPE:
                {
                    nodeIDs.pop();
                    auto shapes = std::static_pointer_cast<SShapeNode>(tmp);

                    for (auto &&m : shapes->Models)
                        m_ModelMatrices.insert({m, CMat4x4::Translation(CVector(currentTranslation.x, currentTranslation.z, -currentTranslation.y)) * currentRotation});                
                } break;
            }
        }

        Reset();
    }

    CMagicaVoxelLoader::TransformNode CMagicaVoxelLoader::ProcessTransformNode()
    {
        TransformNode Ret = TransformNode(new STransformNode());

        Ret->NodeID = ReadData<int>();
        
        // Skips the dictionary
        int keys = ReadData<int>();
        for (size_t i = 0; i < keys; i++)
        {
            int size = ReadData<int>();
            Skip(size);
            size = ReadData<int>();
            Skip(size);
        }

        Ret->ChildID = ReadData<int>();
        Skip(sizeof(int));
        Ret->LayerID = ReadData<int>();

        int frames = ReadData<int>();
        for (size_t i = 0; i < frames; i++)
        {
            keys = ReadData<int>();
            for (size_t j = 0; j < keys; j++)
            {
                int size = ReadData<int>();
                std::string Key(size, '\0'); 
                ReadData(&Key[0], size);

                if(Key == "_t")
                {
                    size = ReadData<int>();
                    std::string Value(size, '\0'); 
                    ReadData(&Value[0], size);

                    std::stringstream tmp;
                    tmp << Value;

                    tmp >> Ret->Translation.x >> Ret->Translation.y >> Ret->Translation.z;
                }
                else if(Key == "_r")
                {
                    size = ReadData<int>();
                    std::string Value(size, '\0'); 
                    ReadData(&Value[0], size);

                    char rot = std::stoi(Value);

                    char idx1 = rot & 3;
                    char idx2 = (rot >> 2) & 3;
                    char idx3 = 3 - idx1 - idx2;

                    Ret->Rotation = CMat4x4(CVector4(0, 0, 0, 0),
                                            CVector4(0, 0, 0, 0),
                                            CVector4(0, 0, 0, 0),
                                            CVector4(0, 0, 0, 1));

                    Ret->Rotation.x.v[idx1] = ((rot & 0x10) == 0x10) ? -1 : 1;
                    Ret->Rotation.y.v[idx2] = ((rot & 0x20) == 0x20) ? -1 : 1;
                    Ret->Rotation.z.v[idx3] = ((rot & 0x40) == 0x40) ? -1 : 1;

                    CVector rotation;

                    // Calculates the euler angle of the roation matrix.
                    // Source: http://eecs.qmul.ac.uk/~gslabaugh/publications/euler.pdf (09.10.2021)
                    if(Ret->Rotation.z.x != 1 && Ret->Rotation.z.x != -1)
                    {
                        rotation.y = -asin(Ret->Rotation.z.x);
                        rotation.x = atan2(Ret->Rotation.z.y / cos(rotation.y), Ret->Rotation.z.z / cos(rotation.y));
                        rotation.z = atan2(Ret->Rotation.y.x / cos(rotation.y), Ret->Rotation.x.x / cos(rotation.y));
                    }
                    else
                    {
                        if(Ret->Rotation.z.x == -1)
                        {
                            rotation.y = M_PI / 2.f;
                            rotation.x = atan2(Ret->Rotation.x.y, Ret->Rotation.x.z);
                        }
                        else
                        {
                            rotation.y = -M_PI / 2.f;
                            rotation.x = atan2(-Ret->Rotation.x.y, -Ret->Rotation.x.z);
                        }
                    }

                    // Builds a new rotation matrix, around the y-axis
                    Ret->Rotation = CMat4x4::Rotation(rotation);
                }
                else if(Key == "_f")
                    Skip(ReadData<int>());
            }
        }

        return Ret;
    }
    
    CMagicaVoxelLoader::GroupNode CMagicaVoxelLoader::ProcessGroupNode()
    {
        GroupNode Ret = GroupNode(new SGroupNode());

        Ret->NodeID = ReadData<int>();
        Skip(sizeof(int));

        int childs = ReadData<int>();
        for (size_t i = 0; i < childs; i++)
            Ret->ChildrensID.push_back(ReadData<int>());

        return Ret;
    }

    CMagicaVoxelLoader::ShapeNode CMagicaVoxelLoader::ProcessShapeNode()
    {
        ShapeNode Ret = ShapeNode(new SShapeNode());

        Ret->NodeID = ReadData<int>();
        Skip(sizeof(int));

        int childs = ReadData<int>();
        for (size_t i = 0; i < childs; i++)
        {
            Ret->Models.push_back(ReadData<int>());

            // Skips the dictionary
            int keys = ReadData<int>();
            for (size_t i = 0; i < keys; i++)
            {
                Skip(ReadData<int>());
                Skip(ReadData<int>());
            }
        }

        return Ret;
    }
} // namespace VoxelOptimizer
