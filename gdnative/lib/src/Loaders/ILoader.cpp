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
#include <VoxelOptimizer/Exceptions.hpp>
#include <fstream>
#include <VoxelOptimizer/Loaders/GoxelLoader.hpp>
#include <VoxelOptimizer/Loaders/ILoader.hpp>
#include <VoxelOptimizer/Loaders/KenshapeLoader.hpp>
#include <VoxelOptimizer/Loaders/MagicaVoxelLoader.hpp>
#include <VoxelOptimizer/Loaders/QubicleBinaryLoader.hpp>
#include <VoxelOptimizer/Loaders/QubicleBinaryTreeLoader.hpp>
#include <VoxelOptimizer/Loaders/QubicleExchangeLoader.hpp>
#include <VoxelOptimizer/Loaders/QubicleLoader.hpp>
#include <string.h>

namespace VoxelOptimizer
{
    Loader ILoader::CreateAndLoad(const std::string &filename)
    {
        std::string ext = GetFileExt(filename);
        LoaderTypes type = LoaderTypes::UNKNOWN;
        
        if(ext == "vox")
            type = LoaderTypes::MAGICAVOXEL;
        else if(ext == "gox")
            type = LoaderTypes::GOXEL;
        else if(ext == "kenshape")
            type = LoaderTypes::KENSHAPE;
        else if(ext == "qb")
            type = LoaderTypes::QUBICLE_BIN;
        else if(ext == "qbt")
            type = LoaderTypes::QUBICLE_BIN_TREE;
        else if(ext == "qef")
            type = LoaderTypes::QUBICLE_EXCHANGE;
        else if(ext == "qbcl")
            type = LoaderTypes::QUBICLE;

        auto loader = Create(type);
        loader->Load(filename);

        return loader;
    }

    Loader ILoader::Create(LoaderTypes type)
    {
        switch (type)
        {
            case LoaderTypes::MAGICAVOXEL: return Loader(new CMagicaVoxelLoader());
            case LoaderTypes::GOXEL: return Loader(new CGoxelLoader());
            case LoaderTypes::KENSHAPE: return Loader(new CKenshapeLoader());
            case LoaderTypes::QUBICLE_BIN: return Loader(new CQubicleBinaryLoader());
            case LoaderTypes::QUBICLE_BIN_TREE: return Loader(new CQubicleBinaryTreeLoader());
            case LoaderTypes::QUBICLE_EXCHANGE: return Loader(new CQubicleExchangeLoader());
            case LoaderTypes::QUBICLE: return Loader(new CQubicleLoader());

            default: throw CVoxelLoaderException("Unknown file type!");
        }
    }

    void ILoader::Load(const std::string &File)
    {
        std::ifstream in(File, std::ios::binary);
        if(in.is_open())
        {
            m_Pos = 0;

            in.seekg(0, in.end);
            size_t Len = in.tellg();
            in.seekg(0, in.beg);

            m_Data.resize(Len);
            in.read(&m_Data[0], Len);
            in.close();

            ParseFormat();
        }
        else
            throw CVoxelLoaderException("Failed to open '" + File + "'");
    }

    void ILoader::Load(const char *Data, size_t Length)
    {
        m_Pos = 0;
        m_Data = std::vector<char>(Data, Data + Length);
        ParseFormat();
    }

    void ILoader::ReadData(char *Buf, size_t Size)
    {
        if(m_Pos + Size > m_Data.size())
            throw CVoxelLoaderException("Unexpected file ending.");

        memcpy(Buf, m_Data.data() + m_Pos, Size);
        m_Pos += Size;
    }

    bool ILoader::IsEof()
    {
        return m_Pos >= m_Data.size();
    }

    size_t ILoader::Tellg()
    {
        return m_Pos;
    }

    void ILoader::Skip(size_t Bytes)
    {
        m_Pos += Bytes;
    }

    void ILoader::Reset()
    {
        m_Pos = 0;
    }

    size_t ILoader::Size()
    {
        return m_Data.size();
    }

    char *ILoader::Ptr()
    {
        return m_Data.data() + m_Pos;
    }
} // namespace VoxelOptimizer
