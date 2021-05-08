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

#ifndef GODOTVOXELOPTIMIZER_HPP
#define GODOTVOXELOPTIMIZER_HPP

#include <ArrayMesh.hpp>
#include <Godot.hpp>
#include <Reference.hpp>
#include <VoxelOptimizer/VoxelOptimizer.hpp>

using namespace godot;

class CGodotVoxelOptimizer : public Reference
{
    GODOT_CLASS(CGodotVoxelOptimizer, Reference);
    public:
        CGodotVoxelOptimizer() : m_BlockCount(0) {}

        void _init() { }

        static void _register_methods();
        godot_error Load(String Path);
        godot_error Save(String Path);
        godot_error SaveSlices(String Path);
        Ref<ArrayMesh> GetMesh(bool Optimized);
        Dictionary GetStatistics();

        virtual ~CGodotVoxelOptimizer() = default;
    private:
        VoxelOptimizer::CMagicaVoxelLoader m_Loader;
        VoxelOptimizer::Mesh m_Mesh;
        int m_BlockCount;
};

#endif //GODOTVOXELOPTIMIZER_HPP