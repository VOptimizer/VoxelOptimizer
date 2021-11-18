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

namespace VoxelOptimizer
{
    class CTriangle
    {
        public:
            std::vector<CVector> Indices;
            int Material;
    };
    using Triangle = std::shared_ptr<CTriangle>;

    class Point
    {
        public:
            Point(const CVector& position, const CVector &index, const CVector& center) : Position(position), Index(index)
            {
                if(position.x == center.x)
                    m_Angle = 0;
                else
                    m_Angle = atan(double(position.z - center.z) / double(position.y - center.y) / double(position.x - center.x));
            }

            CVector Position;
            CVector Index;
            
            bool operator<(const Point &p) const {
                return m_Angle < p.m_Angle;
            }

        private:
            double m_Angle;
    };

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
            if (initTriangles[trianglesIt->first] > 2)
            {
                // Stores a polygon
                std::vector<Point> points;

                for (auto &&triangle : trianglesIt->second)
                {
                    for (auto &&i : triangle->Indices)
                    {
                        //Reduce the vertex count
                        verticesCounter[i.x - 1]--;

                        if(i != trianglesIt->first)
                        {
                            //Obtain all remaining indices
                            if(points.empty())
                                points.push_back(Point(mesh->Vertices[i.x - 1], i, mesh->Vertices[trianglesIt->first.x - 1]));
                            else
                            {
                                bool found = false;

                                for (auto &&p : points)
                                {
                                    if(p.Index == i)
                                    {
                                        found = true;
                                        break;
                                    }
                                }    

                                if(!found)
                                    points.push_back(Point(mesh->Vertices[i.x - 1], i, mesh->Vertices[trianglesIt->first.x - 1]));
                            }
                            
                            //Delete this triangle from all shared indices
                            auto it = triangles.find(i);
                            if(it != triangles.end())
                                it->second.remove(triangle);
                        }
                    }
                }

                std::vector<CVector> indices;
                std::sort(points.begin(), points.end());

                for (auto &&p : points)
                {
                    indices.push_back(p.Index);
                }               

                trianglesIt = triangles.erase(trianglesIt);

                int triCounter = 0;
                for (size_t i = 2; i < indices.size(); i++)
                {
                    auto triangle = Triangle(new CTriangle());
                    triangle->Indices.push_back(indices[0]);
                    triangle->Indices.push_back(indices[i - 1]);
                    triangle->Indices.push_back(indices[i]);

                    triangles[indices[0]].push_back(triangle);
                    triangles[indices[i - 1]].push_back(triangle);
                    triangles[indices[i]].push_back(triangle);

                    verticesCounter[indices[0].x - 1]++;
                    verticesCounter[indices[i - 1].x - 1]++;
                    verticesCounter[indices[i].x - 1]++;

                    triCounter++;
                }
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
