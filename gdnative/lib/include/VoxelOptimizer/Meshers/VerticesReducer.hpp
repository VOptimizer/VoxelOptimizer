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

#ifndef VERTICESREDUCER_HPP
#define VERTICESREDUCER_HPP

#include <VoxelOptimizer/Mesh.hpp>
#include <list>

namespace VoxelOptimizer 
{
    class CVerticesReducer
    {
        public:
            CVerticesReducer(/* args */) {}

            Mesh Reduce(Mesh mesh);

            ~CVerticesReducer() {}

        private:
            class Point
            {
                public:
                    Point(const CVector& position, const CVector &index) : Position(position), Index(index) {}

                    inline void CalcAngle(const CVector& center, const CVector& normal)
                    {
                        CVector forward(0, 0, 1);
                        CVector right(1, 0, 0);
                        CVector u;

                        CVector n = normal;

                        // Projects the 3D point onto a 2D plane
                        //https://answers.unity.com/questions/1522620/converting-a-3d-polygon-into-a-2d-polygon.html
                        if(fabs(n.Dot(forward)) < 0.2f)
                            u = right - right.Dot(n) * n;
                        else
                            u = forward - forward.Dot(n) * n;

                        CVector v = n.Cross(u).Normalize();

                        CVector p(u.Dot(Position), v.Dot(Position), 0);
                        CVector pc(u.Dot(center), v.Dot(center), 0);

                        Position2D = p;
                        m_Angle = atan2(p.y - pc.y, p.x - pc.x);
                    }

                    CVector Position;
                    CVector Position2D;
                    CVector Index;
                    
                    bool operator<(const Point &p) const {
                        return m_Angle < p.m_Angle;
                    }

                private:
                    double m_Angle;
            };

            std::vector<Point> m_Points;

            float Cross2D(CVector a, CVector b);
            bool PointInTriangle(CVector p, CVector a, CVector b, CVector c);
            bool IsEar(CVector a, CVector b, CVector c);
            bool Has180DegreeAngle();

            Point GetPoint(int index);
    };
} // namespace VoxelOptimizer


#endif //VERTICESREDUCER_HPP