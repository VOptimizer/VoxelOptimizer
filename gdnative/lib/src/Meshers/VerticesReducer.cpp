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

#include <algorithm>
#include <VoxelOptimizer/Meshers/VerticesReducer.hpp>
#include <list>
#include <map>
#include <vector>

#include <iostream>

using namespace std;

namespace VoxelOptimizer
{
    class CTriangle
    {
        public:
            std::vector<CVector> Indices;
            int Material;
    };
    using Triangle = std::shared_ptr<CTriangle>;

    float CVerticesReducer::Cross2D(CVector a, CVector b)
    {
        return a.x * b.y - a.y * b.x;
    }

    bool CVerticesReducer::PointInTriangle(CVector p, CVector a, CVector b, CVector c)
    {
        float d1 = Cross2D(p - a, b - a);
        float d2 = Cross2D(p - b, c - b);
        float d3 = Cross2D(p - c, a - c);

        return (d1 >= 0 && d2 >= 0 && d3 >= 0) || (d1 <= 0 && d2 <= 0 && d3 <= 0);
    }

    bool CVerticesReducer::IsEar(CVector a, CVector b, CVector c)
    {
        CVector ba = b - a;
        CVector ca = c - a;

        float cross = Cross2D(ba, ca);
        if(cross >= 0)
            return false;

        for (auto& point : m_Points)
        {
            if (point.Position2D == a || point.Position2D == b || point.Position2D == c)
                continue;

            if (PointInTriangle(a, b, c, point.Position2D))
                return false;
        }

        return true;
    }

    bool CVerticesReducer::Has180DegreeAngle()
    {
        for(int i = 0; i < m_Points.size(); i += 2)
        {
            Point a = GetPoint(i);
            Point b = GetPoint(i - 1);
            Point c = GetPoint(i + 1);

            CVector ba = b.Position2D - a.Position2D;
            CVector ca = c.Position2D - a.Position2D;

            float cross = Cross2D(ba, ca);
            if(cross >= 0 && cross <= 0.001)
                return true;
        }

        return false;
    }

    CVerticesReducer::Point CVerticesReducer::GetPoint(int index)
    {
        if(index >= ((long)m_Points.size()))
            index = index % ((long)m_Points.size());
        else if(index < 0)
            index = (index % ((long)m_Points.size())) + ((long)m_Points.size());
        
        return m_Points[index];
    }

    Mesh CVerticesReducer::Reduce(Mesh mesh)
    {
        std::map<CVector, std::list<Triangle>> triangles;
        std::map<CVector, int> initTriangles;
        std::vector<int> verticesCounter(mesh->Vertices.size(), 0);

        // Builds a map of indices to triangles.
        for (auto &&f : mesh->Faces)
        {
            for (size_t i = 0; i < f->Indices.size(); i += 3)
            {
                auto triangle = Triangle(new CTriangle());
                triangle->Indices.push_back(f->Indices[i]);
                triangle->Indices.push_back(f->Indices[i + 1]);
                triangle->Indices.push_back(f->Indices[i + 2]);
                triangle->Material = f->MaterialIndex;

                triangles[f->Indices[i]].push_back(triangle);
                triangles[f->Indices[i + 1]].push_back(triangle);
                triangles[f->Indices[i + 2]].push_back(triangle);

                initTriangles[f->Indices[i]]++;
                initTriangles[f->Indices[i + 1]]++;
                initTriangles[f->Indices[i + 2]]++;

                verticesCounter[f->Indices[i].x - 1]++;
                verticesCounter[f->Indices[i + 1].x - 1]++;
                verticesCounter[f->Indices[i + 2].x - 1]++;
            }
        }
        
        //Iterate over all triangles and delete all triangles which have shared indices
        auto trianglesIt = triangles.begin();

        while (trianglesIt != triangles.end())
        {
            if (initTriangles[trianglesIt->first] % 3 == 0)
            {
                for (auto &&triangle : trianglesIt->second)
                {
                    for (auto &&i : triangle->Indices)
                    {
                        //Reduce the vertex count
                        verticesCounter[i.x - 1]--;

                        if(i != trianglesIt->first)
                        {
                            //Obtain all remaining indices
                            if(m_Points.empty())
                            {
                                m_Points.push_back(Point(mesh->Vertices[i.x - 1], i));
                                m_Points.back().CalcAngle(mesh->Vertices[trianglesIt->first.x - 1], mesh->Normals[trianglesIt->first.y - 1]);
                            }
                            else
                            {
                                bool found = false;

                                for (auto &&p : m_Points)
                                {
                                    if(p.Index == i)
                                    {
                                        found = true;
                                        break;
                                    }
                                }    

                                if(!found)
                                {
                                    m_Points.push_back(Point(mesh->Vertices[i.x - 1], i));
                                    m_Points.back().CalcAngle(mesh->Vertices[trianglesIt->first.x - 1], mesh->Normals[trianglesIt->first.y - 1]);
                                }
                            }
                            
                            //Delete this triangle from all shared indices
                            auto it = triangles.find(i);
                            if(it != triangles.end())
                                it->second.remove(triangle);
                        }
                    }
                }

                std::sort(m_Points.begin(), m_Points.end());
                if(m_Points.size() < 3 || Has180DegreeAngle())
                {
                    m_Points.clear();

                    for (auto &&triangle : trianglesIt->second)
                    {
                        for (auto &&i : triangle->Indices)
                        {
                            //Reduce the vertex count
                            verticesCounter[i.x - 1]++;

                            if(i != trianglesIt->first)
                            {
                                //Delete this triangle from all shared indices
                                auto it = triangles.find(i);
                                if(it != triangles.end())
                                    it->second.push_back(triangle);                               
                            }
                        }
                    }

                    trianglesIt++;
                    
                    continue;
                }

                // std::vector<CVector> indices;

                cout << "\n\n\n\n" << endl;
                for (auto &&p : m_Points)
                {
                    cout << "Position3D: " << p.Position << " Position2D: " << p.Position2D << endl;
                }
                

                while (m_Points.size() > 3)
                { 
Has180DegreeAngle();

                    for(int i = 0; i < m_Points.size(); i++)
                    {
                        Point a = GetPoint(i);
                        Point b = GetPoint(i - 1);
                        Point c = GetPoint(i + 1);

                        if(IsEar(a.Position2D, b.Position2D, c.Position2D))
                        {
                            auto triangle = Triangle(new CTriangle());
                            triangle->Indices.push_back(a.Index);
                            triangle->Indices.push_back(b.Index);
                            triangle->Indices.push_back(c.Index);

                            triangles[a.Index].push_back(triangle);
                            triangles[b.Index].push_back(triangle);
                            triangles[c.Index].push_back(triangle);

                            verticesCounter[a.Index.x - 1]++;
                            verticesCounter[b.Index.x - 1]++;
                            verticesCounter[c.Index.x - 1]++;

                            m_Points.erase(m_Points.begin() + i);
                            break;
                        }
                    }
                }

                Point a = GetPoint(0);
                Point b = GetPoint(-1);
                Point c = GetPoint(1);

                auto triangle = Triangle(new CTriangle());
                triangle->Indices.push_back(a.Index);
                triangle->Indices.push_back(b.Index);
                triangle->Indices.push_back(c.Index);

                triangles[a.Index].push_back(triangle);
                triangles[b.Index].push_back(triangle);
                triangles[c.Index].push_back(triangle);

                verticesCounter[a.Index.x - 1]++;
                verticesCounter[b.Index.x - 1]++;
                verticesCounter[c.Index.x - 1]++;
                
                trianglesIt = triangles.erase(trianglesIt);
                m_Points.clear();

                // cout << "\n\n\n\n\nRemove Index: " << trianglesIt->first << " Remove Position: " << mesh->Vertices[trianglesIt->first.x - 1] << endl;

                // for (auto &&p : m_Points)
                // {
                //     cout << "Index: " << p.Index << " Position: " << p.Position << endl;

                //     indices.push_back(p.Index);
                // }               

                // int triCounter = 0;
                // for (size_t i = 2; i < indices.size(); i++)
                // {
                //     auto triangle = Triangle(new CTriangle());
                //     triangle->Indices.push_back(indices[0]);
                //     triangle->Indices.push_back(indices[i - 1]);
                //     triangle->Indices.push_back(indices[i]);

                //     triangles[indices[0]].push_back(triangle);
                //     triangles[indices[i - 1]].push_back(triangle);
                //     triangles[indices[i]].push_back(triangle);

                //     verticesCounter[indices[0].x - 1]++;
                //     verticesCounter[indices[i - 1].x - 1]++;
                //     verticesCounter[indices[i].x - 1]++;

                //     triCounter++;
                // }
            }
            else
                trianglesIt++;
        }
    
        std::vector<int> verticesToRemove;
        int sub = 0;

        for (size_t i = 0; i < verticesCounter.size(); i++)
        {
            if (verticesCounter[i] <= 0)
            {
                mesh->Vertices.erase(mesh->Vertices.begin() + (i - sub));
                sub++;
                verticesToRemove.push_back(i + 1);
            }
        }  

        std::sort(verticesToRemove.begin(), verticesToRemove.end());

        std::vector<CVector> indices;
        for (auto &&t : triangles)
        {
            for (auto &&triangle : t.second)
            {
                for (auto &&i : triangle->Indices)
                {
                    CVector index = i;

                    for (auto &&r : verticesToRemove)
                    {
                        if (i.x > r)
                            index.x--;
                        else
                            break;
                    }

                    indices.push_back(index);

                    if(i != t.first)
                    {
                        //Delete this triangle from all shared indices
                        auto it = triangles.find(i);
                        if(it != triangles.end())
                            it->second.remove(triangle);
                    }
                }
            }
        }

        mesh->Faces.front()->Indices = indices;

        return mesh;
    }
} // namespace VoxelOptimizer
