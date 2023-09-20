/*
 * Copyright 2023 Guillaume Guillet
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

namespace fge::AStar
{

Node::Node(std::optional<fge::Vector2i> parent) :
        _costScore(0),
        _heuristicScore(0),
        _parent(parent)
{}

unsigned int Node::getScore() const
{
    return this->_costScore + this->_heuristicScore;
}

Generator::Generator() :
        g_heuristic(&Heuristic::euclidean),
        g_directions({{{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {-1, -1}, {1, 1}, {-1, 1}, {1, -1}}}),
        g_directionsCount(8)
{
    this->setDiagonalMovement(false);
    this->setHeuristic(&Heuristic::manhattan);
}

void Generator::setWorldSize(fge::Vector2i worldSize)
{
    this->g_worldSize = worldSize;
}
fge::Vector2i const& Generator::getWorldSize() const
{
    return this->g_worldSize;
}

void Generator::setDiagonalMovement(bool enable)
{
    this->g_directionsCount = enable ? 8 : 4;
}

void Generator::setHeuristic(HeuristicFunction heuristic)
{
    this->g_heuristic = heuristic;
}

void Generator::addCollision(fge::Vector2i coord)
{
    this->g_walls.insert(coord);
}

void Generator::removeCollision(fge::Vector2i coord)
{
    this->g_walls.erase(coord);
}

void Generator::clearCollisions()
{
    this->g_walls.clear();
}

CoordinateList Generator::findPath(fge::Vector2i source, fge::Vector2i target)
{
    Node* current = nullptr;
    fge::Vector2i currentCoord;
    NodeMap openNodes;
    NodeMap closeNodes;

    openNodes.reserve(100);
    closeNodes.reserve(100);
    openNodes.emplace(source, Node{});

    bool validPath = false;

    while (!openNodes.empty())
    {
        auto itCurrent = openNodes.begin();
        current = &itCurrent->second;

        for (auto it = ++openNodes.begin(); it != openNodes.end(); ++it)
        {
            if (it->second.getScore() <= current->getScore())
            {
                current = &it->second;
                itCurrent = it;
            }
        }

        currentCoord = itCurrent->first;

        if (currentCoord == target)
        { //Goal reached
            validPath = true;
            break;
        }

        closeNodes.emplace(currentCoord, *current);

        for (std::size_t i = 0; i < this->g_directionsCount; ++i)
        {
            fge::Vector2i newCoordinates{currentCoord + this->g_directions[i]};
            if (this->detectCollision(newCoordinates) || closeNodes.find(newCoordinates) != closeNodes.end())
            {
                continue;
            }

            unsigned int totalCost = current->_costScore + ((i < 4) ? 10 : 14);

            auto successor = openNodes.find(newCoordinates);
            if (successor == openNodes.end())
            {
                successor = openNodes.emplace(newCoordinates, Node{currentCoord}).first;
                successor->second._costScore = totalCost;
                successor->second._heuristicScore = this->g_heuristic(successor->first, target);
            }
            else if (totalCost < successor->second._costScore)
            {
                successor->second._parent = currentCoord;
                successor->second._costScore = totalCost;
            }
        }

        openNodes.erase(itCurrent);
    }

    if (validPath)
    {
        CoordinateList path;
        path.reserve(closeNodes.size());

        while (current != nullptr)
        {
            path.push_back(currentCoord);

            if (current->_parent.has_value())
            {
                auto it = closeNodes.find(current->_parent.value());
                if (it != closeNodes.end())
                {
                    current = &it->second;
                    currentCoord = it->first;
                    continue;
                }
            }
            current = nullptr;
        }

        std::reverse(path.begin(), path.end());
        return path;
    }
    return {};
}

bool Generator::detectCollision(fge::Vector2i coord)
{
    bool isWall = this->g_walls.find(coord) != this->g_walls.end();

    return isWall || coord.x < 0 || coord.x >= this->g_worldSize.x || coord.y < 0 || coord.y >= this->g_worldSize.y;
}

fge::Vector2i Heuristic::getDelta(fge::Vector2i source, fge::Vector2i target)
{
    return {std::abs(source.x - target.x), std::abs(source.y - target.y)};
}

unsigned int Heuristic::manhattan(fge::Vector2i source, fge::Vector2i target)
{
    auto delta = Heuristic::getDelta(source, target);
    return static_cast<unsigned int>(10 * (delta.x + delta.y));
}

unsigned int Heuristic::euclidean(fge::Vector2i source, fge::Vector2i target)
{
    auto delta = Heuristic::getDelta(source, target);
    return static_cast<unsigned int>(10 * std::sqrt(delta.x * delta.x + delta.y * delta.y));
}

unsigned int Heuristic::octagonal(fge::Vector2i source, fge::Vector2i target)
{
    auto delta = Heuristic::getDelta(source, target);
    return 10 * (delta.x + delta.y) + (-6) * std::min(delta.x, delta.y);
}

} // namespace fge::AStar