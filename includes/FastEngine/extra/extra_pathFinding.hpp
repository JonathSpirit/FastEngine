/*
 * Copyright 2025 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FGE_EXTRA_PATHFINDING_HPP_INCLUDED
#define _FGE_EXTRA_PATHFINDING_HPP_INCLUDED

/*
 * Original from : https://github.com/daancode/a-star
 * Copyright (c) 2015, Damian Barczynski <daan.net@wp.eu>
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_vector.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fge::AStar
{

using HeuristicFunction = unsigned int (*)(fge::Vector2i, fge::Vector2i);
using CoordinateList = std::vector<fge::Vector2i>;

struct Vector2iHash
{
    static_assert(sizeof(fge::Vector2i) == 8, "bad fge::Vector2i size, should be 8 !");

    inline std::size_t operator()(fge::Vector2i const& coord) const
    {
        return std::hash<uint64_t>()(*reinterpret_cast<uint64_t const*>(&coord));
    }
};

using CoordinateSet = std::unordered_set<fge::Vector2i, fge::AStar::Vector2iHash>;

struct FGE_API Node
{
    explicit Node(std::optional<fge::Vector2i> parent = std::nullopt);
    [[nodiscard]] unsigned int getScore() const;

    unsigned int _costScore;
    unsigned int _heuristicScore;
    std::optional<fge::Vector2i> _parent;
};

using NodeMap = std::unordered_map<fge::Vector2i, fge::AStar::Node, fge::AStar::Vector2iHash>;

class FGE_API Generator
{
public:
    Generator();
    ~Generator() = default;

    void setWorldSize(fge::Vector2i worldSize);
    [[nodiscard]] fge::Vector2i const& getWorldSize() const;

    void setDiagonalMovement(bool enable);
    void setHeuristic(HeuristicFunction heuristic);
    CoordinateList findPath(fge::Vector2i source, fge::Vector2i target);
    void addCollision(fge::Vector2i coord);
    void removeCollision(fge::Vector2i coord);
    void clearCollisions();

private:
    bool detectCollision(fge::Vector2i coord);

    HeuristicFunction g_heuristic;
    CoordinateSet g_walls;
    fge::Vector2i g_worldSize;

    std::array<fge::Vector2i, 8> const g_directions;
    std::size_t g_directionsCount;
};

class FGE_API Heuristic
{
private:
    static fge::Vector2i getDelta(fge::Vector2i source, fge::Vector2i target);

public:
    static unsigned int manhattan(fge::Vector2i source, fge::Vector2i target);
    static unsigned int euclidean(fge::Vector2i source, fge::Vector2i target);
    static unsigned int octagonal(fge::Vector2i source, fge::Vector2i target);
};

} // namespace fge::AStar

#endif //_FGE_EXTRA_PATHFINDING_HPP_INCLUDED
