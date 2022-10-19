/*
 * Copyright 2022 Guillaume Guillet
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

#include "FastEngine/fastengine_extern.hpp"
#include <functional>
#include <vector>
#include <unordered_set>
#include <array>
#include <cstdint>
#include "SFML/System/Vector2.hpp"

namespace fge::AStar
{

using Vector2i = sf::Vector2<int32_t>;
using HeuristicFunction = std::function<unsigned int(fge::AStar::Vector2i, fge::AStar::Vector2i)>;
using CoordinateList = std::vector<fge::AStar::Vector2i>;

struct Vector2int32Hash
{
    static_assert(sizeof(fge::AStar::Vector2i) == 8, "bad sf::Vector2<int32_t> size, should be 8 !");

    inline std::size_t operator() (const fge::AStar::Vector2i& coord) const
    {
        return std::hash<uint64_t>()( *reinterpret_cast<const uint64_t*>(&coord) );
    }
};

using CoordinateSet = std::unordered_set<fge::AStar::Vector2i, fge::AStar::Vector2int32Hash>;

struct FGE_API Node
{
    explicit Node(fge::AStar::Vector2i coord, Node* parent = nullptr);
    [[nodiscard]] unsigned int getScore() const;

    unsigned int _g, _h;
    fge::AStar::Vector2i _coord;
    Node* _parent;
};

using NodeList = std::vector<Node*>;

class FGE_API Generator
{
public:
    Generator();
    ~Generator() = default;

    void setWorldSize(fge::AStar::Vector2i worldSize);
    [[nodiscard]] const fge::AStar::Vector2i& getWorldSize() const;

    void setDiagonalMovement(bool enable);
    void setHeuristic(HeuristicFunction heuristic);
    CoordinateList findPath(fge::AStar::Vector2i source, fge::AStar::Vector2i target);
    void addCollision(fge::AStar::Vector2i coord);
    void removeCollision(fge::AStar::Vector2i coord);
    void clearCollisions();

private:
    bool detectCollision(fge::AStar::Vector2i coord);
    [[nodiscard]] Node* findNodeOnList(const NodeList& nodes, fge::AStar::Vector2i coord) const;
    void releaseNodes(NodeList& nodes);

    HeuristicFunction g_heuristic;
    CoordinateSet g_walls;
    fge::AStar::Vector2i g_worldSize;

    const std::array<fge::AStar::Vector2i, 8> g_directions;
    std::size_t g_directionsCount;
};

class FGE_API Heuristic
{
private:
    static fge::AStar::Vector2i getDelta(fge::AStar::Vector2i source, fge::AStar::Vector2i target);

public:
    static unsigned int manhattan(fge::AStar::Vector2i source, fge::AStar::Vector2i target);
    static unsigned int euclidean(fge::AStar::Vector2i source, fge::AStar::Vector2i target);
    static unsigned int octagonal(fge::AStar::Vector2i source, fge::AStar::Vector2i target);
};

}//end fge::AStar

#endif //_FGE_EXTRA_PATHFINDING_HPP_INCLUDED
