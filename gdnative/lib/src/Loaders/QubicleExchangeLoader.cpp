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

#include <VoxelOptimizer/Loaders/QubicleExchangeLoader.hpp>
#include <VoxelOptimizer/Exceptions.hpp>
#include <sstream>

namespace VoxelOptimizer
{
    void CQubicleExchangeLoader::ParseFormat()
    {
        m_Models.clear();
        m_Materials.clear();
        m_Materials.clear();
        m_UsedColorPalette.clear();

        if(ReadLine() != "Qubicle Exchange Format")
            throw CVoxelLoaderException("Unknown file format");

        if(ReadLine() != "Version 0.2")
            throw CVoxelLoaderException("Unsupported version!");

        ReadLine();

        VoxelMesh mesh = VoxelMesh(new CVoxelMesh());
        m_Materials.push_back(Material(new CMaterial()));
        mesh->SetSize(ReadVector());
        ReadColors();
        ReadVoxels(mesh);

        m_Models.push_back(mesh);
    }

    std::string CQubicleExchangeLoader::ReadLine()
    {
        std::string ret;

        char c = 0;
        do
        {
            c = ReadData<char>();

            if(c != '\n' && c != '\r')
                ret += c;
        }while(c != '\n' && !IsEof());

        return ret;
    }

    CVector CQubicleExchangeLoader::ReadVector()
    {
        std::stringstream strm;
        strm << ReadLine();

        CVector ret;

        strm >> ret.x >> ret.z >> ret.y;

        return ret;
    }

    void CQubicleExchangeLoader::ReadColors()
    {
        std::stringstream strm;
        strm << ReadLine();

        int count;
        strm >> count;
        strm.clear();

        for (size_t i = 0; i < count; i++)
        {
            strm << ReadLine();
            float r, g, b;

            strm >> r >> g >> b;
            strm.clear();

            m_UsedColorPalette.push_back(CColor(r * 255.0, g * 255.0, b * 255.0, 255.0));
        }
    }

    void CQubicleExchangeLoader::ReadVoxels(VoxelMesh mesh)
    {
        CVector Beg(1000, 1000, 1000), End;
        while (!IsEof())
        {
            std::stringstream strm;
            strm << ReadLine();

            int mask, cid;

            CVector pos;
            strm >> pos.x >> pos.z >> pos.y >> cid >> mask;

            pos.y = mesh->GetSize().y - pos.y;

            if(mask == 0)
                continue;

            Beg = Beg.Min(pos);
            End = End.Max(pos);
            mesh->SetVoxel(pos, 0, cid, false);
        }
        mesh->SetBBox(CBBox(Beg, End));
    }
} // namespace VoxelOptimizer
