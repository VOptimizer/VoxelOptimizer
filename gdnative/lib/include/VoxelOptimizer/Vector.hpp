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

#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <functional>
#include <string>
#include <math.h>

namespace VoxelOptimizer
{
    class CVector
    {
        public:
            union
            {
                struct
                {
                    float x;
                    float y;
                    float z;
                };
                
                float v[3];
            };

            CVector() : x(0), y(0), z(0) {}
            CVector(float x, float y, float z) : x(x), y(y), z(z) {}

            inline bool IsZero() const
            {
                return x == 0 && y == 0 && z == 0;
            }

            inline bool operator==(const CVector &vr) const
            {
                return x == vr.x && y == vr.y && z == vr.z;
            }

            inline CVector Min(CVector &vec)
            {
                return CVector(std::min(x, vec.x), std::min(y, vec.y), std::min(z, vec.z));
            }

            inline CVector Max(CVector &vec)
            {
                return CVector(std::max(x, vec.x), std::max(y, vec.y), std::max(z, vec.z));
            }

            // Upon here just math. Math is magic :D

            inline CVector operator*(const CVector &vr) const
            {
                return CVector(x * vr.x, y * vr.y, z * vr.z);
            }

            inline CVector operator-(const CVector &vr) const
            {
                return CVector(x - vr.x, y - vr.y, z - vr.z);
            }

            inline CVector operator+(const CVector &vr) const
            {
                return CVector(x + vr.x, y + vr.y, z + vr.z);
            }

            inline CVector operator/(float scalar) const
            {
                return CVector(x / scalar, y / scalar, z / scalar);
            }

            inline CVector operator-() const
            {
                return CVector(-x, -y, -z);
            }

            inline float Dot(const CVector &vr) const
            {
                return x * vr.x + y * vr.y + z * vr.z;
            }

            inline CVector Cross(const CVector &vr)
            {
                return CVector(y * vr.z - z * vr.y, 
                               z * vr.x - x * vr.z, 
                               x * vr.y - y * vr.x);
            }

            inline float Length()
            {
                return sqrt((x * x) + (y * y) + (z * z));
            }

            inline CVector Normalize()
            {
                float v = Length();
                return CVector(x / v, y / v, z / v);
            }

            inline size_t hash() const
            {
                std::hash<std::string> Hash;
                return Hash(std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z));
            }

            ~CVector() = default;
    };
} // namespace VoxelOptimizer

#endif //VECTOR_HPP