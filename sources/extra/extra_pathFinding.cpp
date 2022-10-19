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

#include "FastEngine/extra/extra_pathFinding.hpp"
#include <algorithm>
#include <cmath>

using namespace std::placeholders;

namespace fge::AStar
{

Node::Node(fge::AStar::Vector2i coord, Node* parent) :
        _g(0),
        _h(0),
        _coord(coord),
        _parent(parent)
{
}

unsigned int Node::getScore() const
{
    return this->_g + this->_h;
}

Generator::Generator() :
        g_directions({{
            {0, 1}, {1, 0}, {0, -1}, {-1, 0},
            {-1, -1}, {1, 1}, {-1, 1}, {1, -1}
        }})
{
    this->setDiagonalMovement(false);
    this->setHeuristic(&Heuristic::manhattan);
}

void Generator::setWorldSize(fge::AStar::Vector2i worldSize)
{
    this->g_worldSize = worldSize;
}
const fge::AStar::Vector2i& Generator::getWorldSize() const
{
    return this->g_worldSize;
}

void Generator::setDiagonalMovement(bool enable)
{
    this->g_directionsCount = enable ? 8 : 4;
}

void Generator::setHeuristic(HeuristicFunction heuristic)
{
    this->g_heuristic = std::bind(heuristic, _1, _2);
}

void Generator::addCollision(fge::AStar::Vector2i coord)
{
    this->g_walls.insert(coord);
}

void Generator::removeCollision(fge::AStar::Vector2i coord)
{
    this->g_walls.erase(coord);
}

void Generator::clearCollisions()
{
    this->g_walls.clear();
}

CoordinateList Generator::findPath(fge::AStar::Vector2i source, fge::AStar::Vector2i target)
{
    Node* current = nullptr;
    NodeList openNodes;
    NodeList closeNodes;

    openNodes.reserve(100);
    closeNodes.reserve(100);
    openNodes.push_back(new Node(source));

    bool validPath = false;

    while (!openNodes.empty())
    {
        auto itCurrent = openNodes.begin();
        current = *itCurrent;

        for (auto it = ++openNodes.begin(); it != openNodes.end(); ++it)
        {
            auto* node = *it;
            if (node->getScore() <= current->getScore())
            {
                current = node;
                itCurrent = it;
            }
        }

        if (current->_coord == target)
        {
            validPath = true;
            break;
        }

        closeNodes.push_back(current);
        openNodes.erase(itCurrent);

        for (std::size_t i = 0; i < this->g_directionsCount; ++i)
        {
            fge::AStar::Vector2i newCoordinates{current->_coord + this->g_directions[i]};
            if (this->detectCollision(newCoordinates) ||
                this->findNodeOnList(closeNodes, newCoordinates) != nullptr)
            {
                continue;
            }

            unsigned int totalCost = current->_g + ((i < 4) ? 10 : 14);

            Node* successor = findNodeOnList(openNodes, newCoordinates);
            if (successor == nullptr)
            {
                successor = new Node(newCoordinates, current);
                successor->_g = totalCost;
                successor->_h = g_heuristic(successor->_coord, target);
                openNodes.push_back(successor);
            }
            else if (totalCost < successor->_g)
            {
                successor->_parent = current;
                successor->_g = totalCost;
            }
        }
    }

    CoordinateList path;

    if (validPath)
    {
        while (current != nullptr)
        {
            path.push_back(current->_coord);
            current = current->_parent;
        }

        std::reverse(path.begin(), path.end());
    }

    this->releaseNodes(openNodes);
    this->releaseNodes(closeNodes);

    return path;
}

Node* Generator::findNodeOnList(const NodeList& nodes, fge::AStar::Vector2i coord) const
{
    for (auto* node: nodes)
    {
        if (node->_coord == coord)
        {
            return node;
        }
    }
    return nullptr;
}

void Generator::releaseNodes(NodeList& nodes)
{
    for (auto* node: nodes)
    {
        delete node;
    }
    nodes.clear();
}

bool Generator::detectCollision(fge::AStar::Vector2i coord)
{
    bool isWall = this->g_walls.find(coord) != this->g_walls.end();

    return isWall || coord.x < 0 || coord.x >= this->g_worldSize.x ||
                     coord.y < 0 || coord.y >= this->g_worldSize.y;
}

fge::AStar::Vector2i Heuristic::getDelta(fge::AStar::Vector2i source, fge::AStar::Vector2i target)
{
    return {std::abs(source.x - target.x), std::abs(source.y - target.y)};
}

unsigned int Heuristic::manhattan(fge::AStar::Vector2i source, fge::AStar::Vector2i target)
{
    auto delta = Heuristic::getDelta(source, target);
    return static_cast<unsigned int>(10 * (delta.x + delta.y));
}

unsigned int Heuristic::euclidean(fge::AStar::Vector2i source, fge::AStar::Vector2i target)
{
    auto delta = Heuristic::getDelta(source, target);
    return static_cast<unsigned int>(10 * std::sqrt(delta.x * delta.x + delta.y * delta.y));
}

unsigned int Heuristic::octagonal(fge::AStar::Vector2i source, fge::AStar::Vector2i target)
{
    auto delta = Heuristic::getDelta(source, target);
    return 10 * (delta.x + delta.y) + (-6) * std::min(delta.x, delta.y);
}

}//end fge::AStar