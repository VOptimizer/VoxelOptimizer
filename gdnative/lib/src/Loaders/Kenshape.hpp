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

#ifndef KENSHAPE_HPP
#define KENSHAPE_HPP

#include <VoxelOptimizer/Color.hpp>
#include <CJSON/JSON.hpp>
#include <VoxelOptimizer/Vector.hpp>

namespace VoxelOptimizer
{
    class CTile
    {
        public:
            CTile() = default;

            int Shape;
            float Angle;
            int ColorIdx;
            int Depth;
            bool Enabled;
            bool Visited;

            void Deserialize(CJSON &json)
            {
                Shape = json.GetValue<int>("shape");
                Angle = json.GetValue<float>("angle");
                ColorIdx = json.GetValue<int>("color");
                Depth = json.GetValue<int>("depth");
                Enabled = json.GetValue<bool>("enabled");
                Visited = json.GetValue<bool>("visited");
            }

            ~CTile() = default;
    };

    using Tile = std::shared_ptr<CTile>;

    class CKenshape
    {
        public:
            CKenshape() = default;

            std::string Title;
            std::string Author;
            CVector Size;
            std::vector<Tile> Tiles;
            std::vector<CColor> Colors;

            void Deserialize(CJSON &json)
            {
                Title = json.GetValue<std::string>("title");
                Author = json.GetValue<std::string>("author");

                std::string SizeStr = json.GetValue<std::string>("size");
                CJSON tmp;
                tmp.ParseObject(SizeStr);
                Size.x = tmp.GetValue<float>("x");
                Size.z = tmp.GetValue<float>("y");
                Size.y = 15;

                Tiles = json.GetValue<std::vector<Tile>>("tiles");
                auto ColorsStrs = json.GetValue<std::vector<std::string>>("colors");

                for (auto &&c : ColorsStrs)
                {
                    Colors.emplace_back();
                    auto s = c.substr(1);
                    int ci = std::stoi(s, nullptr, 16) | 0xFF000000;

                    // Color conversion.
                    ci = ((ci >> 16) & 0xFF) | ((ci << 16) & 0x00FF0000) | (ci & 0xFF00FF00);

                    memcpy(Colors.back().c, &ci, sizeof(int));
                }
            }

            ~CKenshape() = default;
    };

    using Kenshape = std::shared_ptr<CKenshape>;
} // namespace VoxelOptimizer

#endif //KENSHAPE_HPP