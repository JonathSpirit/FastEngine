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

#include "FastEngine/C_concavePolygon.hpp"
#include <queue>

namespace fge
{

ConcavePolygon::ConcavePolygon(VertexArray const& vertices) :
        g_vertices{vertices}
{
    if (this->g_vertices.size() > 2)
    {
        if (!this->checkIfRightHanded())
        {
            this->flipPolygon();
        }
    }
}
ConcavePolygon::ConcavePolygon(VertexArray&& vertices) :
        g_vertices{std::move(vertices)}
{
    if (this->g_vertices.size() > 2)
    {
        if (!this->checkIfRightHanded())
        {
            this->flipPolygon();
        }
    }
}

bool ConcavePolygon::checkIfRightHanded()
{
    if (this->g_vertices.size() < 3)
    {
        return false;
    }

    float signedArea = 0.0f;

    for (std::size_t i = 0; i < this->g_vertices.size(); ++i)
    {
        auto const& vert1 = this->g_vertices[i];
        auto const& vert2 = this->g_vertices[(i + 1) % this->g_vertices.size()];
        signedArea += (vert2.x - vert1.x) * (vert2.y + vert1.y);
    }

    return signedArea < 0.0f;
}

void ConcavePolygon::slicePolygon(fge::Line const& segment)
{
    if (this->g_subPolygons.empty())
    {
        auto polygons = ConcavePolygon::slicePolygon(segment, this->g_vertices);
        if (polygons)
        {
            this->g_subPolygons.push_back(std::move(polygons.value().first));
            this->g_subPolygons.push_back(std::move(polygons.value().second));
        }
        return;
    }

    auto oldSize = this->g_subPolygons.size();
    for (std::size_t i = 0; i < oldSize; ++i)
    {
        auto vertices = ConcavePolygon::slicePolygon(segment, this->g_subPolygons[i]);
        if (vertices)
        {
            this->g_subPolygons.push_back(std::move(vertices.value().first));
            this->g_subPolygons.push_back(std::move(vertices.value().second));
        }
    }
    this->g_subPolygons.erase(this->g_subPolygons.begin(), this->g_subPolygons.begin() + oldSize);
}

void ConcavePolygon::convexDecomposition()
{
    if (!this->g_subPolygons.empty())
    {
        return;
    }

    std::queue<VertexArray> polygonQueue;
    polygonQueue.emplace(this->g_vertices);

    do {
        auto polygon = std::move(polygonQueue.front());
        polygonQueue.pop();

        if (polygon.size() <= 3)
        {
            this->g_subPolygons.push_back(std::move(polygon));
            continue;
        }

        auto reflexIndex = ConcavePolygon::findFirstReflexVertex(polygon);
        if (!reflexIndex)
        {
            this->g_subPolygons.push_back(std::move(polygon));
            continue;
        }

        auto const& prevVertPos = polygon[(reflexIndex.value() - 1) % polygon.size()];
        auto const& currVertPos = polygon[reflexIndex.value()];
        auto const& nextVertPos = polygon[(reflexIndex.value() + 1) % polygon.size()];

        fge::Line const line1(prevVertPos, currVertPos);
        fge::Line const line2(nextVertPos, currVertPos);

        auto const verticesInCone = ConcavePolygon::findVerticesInCone(line1, line2, currVertPos, polygon);

        std::optional<std::size_t> bestVert{};

        if (!verticesInCone.empty())
        {
            bestVert = ConcavePolygon::getBestVertexToConnect(verticesInCone, polygon, currVertPos);
            if (bestVert)
            {
                fge::Line const newLine(currVertPos, polygon[bestVert.value()]);

                auto polygons = ConcavePolygon::slicePolygon(newLine, polygon);
                if (polygons)
                {
                    polygonQueue.emplace(std::move(polygons.value().first));
                    polygonQueue.emplace(std::move(polygons.value().second));
                }
            }
        }
        if (verticesInCone.empty() || !bestVert)
        {
            auto const direction = -glm::normalize((line1._start - line1._end) + (line2._start - line2._end));

            auto polygons = ConcavePolygon::slicePolygon(currVertPos, direction, polygon);
            if (polygons)
            {
                polygonQueue.emplace(std::move(polygons.value().first));
                polygonQueue.emplace(std::move(polygons.value().second));
            }
        }
    } while (!polygonQueue.empty());
}

void ConcavePolygon::setVertices(VertexArray const& vertices)
{
    this->g_subPolygons.clear();
    this->g_vertices = vertices;

    if (this->g_vertices.size() > 2)
    {
        if (!this->checkIfRightHanded())
        {
            this->flipPolygon();
        }
    }
}
void ConcavePolygon::setVertices(VertexArray&& vertices)
{
    this->g_subPolygons.clear();
    this->g_vertices = std::move(vertices);

    if (this->g_vertices.size() > 2)
    {
        if (!this->checkIfRightHanded())
        {
            this->flipPolygon();
        }
    }
}

void ConcavePolygon::clear()
{
    this->g_subPolygons.clear();
}

std::optional<std::pair<ConcavePolygon::VertexArray, ConcavePolygon::VertexArray>>
ConcavePolygon::slicePolygon(fge::Vector2f const& position,
                             fge::Vector2f const& direction,
                             VertexArray const& inputVertices)
{
    constexpr float TOLERANCE = 1e-5f;

    auto slicedVertices = ConcavePolygon::verticesAlongLineSegment(position, direction, inputVertices);
    slicedVertices = ConcavePolygon::cullByDistance(slicedVertices, position, 2);

    if (slicedVertices.size() < 2)
    {
        return std::nullopt;
    }

    VertexArray leftVertices;
    VertexArray rightVertices;

    for (std::size_t i = 0; i < inputVertices.size(); ++i)
    {
        auto relativePosition = inputVertices[i] - position;

        auto it = slicedVertices.begin();

        float const perpDistance = std::abs(fge::Cross2d(relativePosition, direction));

        if (perpDistance > TOLERANCE || (perpDistance <= TOLERANCE && (slicedVertices.find(i) == slicedVertices.end())))
        {
            if ((i > it->first) && (i <= (++it)->first))
            {
                leftVertices.push_back(inputVertices[i]);
            }
            else
            {
                rightVertices.push_back(inputVertices[i]);
            }
        }

        if (slicedVertices.find(i) != slicedVertices.end())
        {
            rightVertices.push_back(slicedVertices[i]);
            leftVertices.push_back(slicedVertices[i]);
        }
    }

    return {{std::move(leftVertices), std::move(rightVertices)}};
}
std::optional<std::pair<ConcavePolygon::VertexArray, ConcavePolygon::VertexArray>>
ConcavePolygon::slicePolygon(fge::Line const& segment, VertexArray const& inputVertices)
{
    return ConcavePolygon::slicePolygon(segment._start, segment.getDirection(), inputVertices);
}

ConcavePolygon::Indices ConcavePolygon::findVerticesInCone(fge::Line const& line1,
                                                           fge::Line const& line2,
                                                           fge::Vector2f const& origin,
                                                           VertexArray const& inputVertices)
{
    Indices result;
    result.reserve(inputVertices.size());

    for (std::size_t i = 0; i < inputVertices.size(); ++i)
    {
        if (fge::IsVertexInCone(line1, line2, origin, inputVertices[i]))
        {
            result.push_back(i);
        }
    }
    return result;
}

bool ConcavePolygon::checkVisibility(fge::Vector2f const& originalPosition,
                                     fge::Vector2f const& vert,
                                     VertexArray const& polygonVertices)
{
    fge::Line const ls(originalPosition, vert);
    auto intersectingVertices = verticesAlongLineSegment(ls, polygonVertices);

    return intersectingVertices.size() <= 3;
}


std::optional<std::size_t> ConcavePolygon::getBestVertexToConnect(Indices const& indices,
                                                                  VertexArray const& polygonVertices,
                                                                  fge::Vector2f const& origin)
{
    if (indices.size() == 1)
    {
        if (ConcavePolygon::checkVisibility(origin, polygonVertices[indices.front()], polygonVertices))
        {
            return indices.front();
        }
    }
    else if (indices.size() > 1)
    {
        std::size_t alternateIndex = std::numeric_limits<std::size_t>::max();
        for (auto index: indices)
        {
            auto const vertSize = polygonVertices.size();

            auto const prevVert = polygonVertices[(index - 1) % vertSize];
            auto const currVert = polygonVertices[index];
            auto const nextVert = polygonVertices[(index + 1) % vertSize];

            fge::Line const line1(prevVert, currVert);
            fge::Line const line2(nextVert, currVert);

            if (fge::GetHandedness(prevVert, currVert, nextVert) < 0.0f &&
                ConcavePolygon::checkVisibility(origin, polygonVertices[index], polygonVertices))
            {
                if (fge::IsVertexInCone(line1, line2, polygonVertices[index], origin))
                {
                    return index;
                }

                if (alternateIndex == std::numeric_limits<std::size_t>::max())
                {
                    alternateIndex = index;
                }
            }
        }

        if (alternateIndex != std::numeric_limits<std::size_t>::max())
        {
            return alternateIndex;
        }

        float minDistance = std::numeric_limits<float>::max();
        auto closest = indices.front();
        for (auto index: indices)
        {
            float const currDistance = fge::DotSquare(polygonVertices[index] - origin);
            if (currDistance < minDistance)
            {
                minDistance = currDistance;
                closest = index;
            }
        }

        return closest;
    }

    return std::nullopt;
}

std::optional<std::size_t> ConcavePolygon::findFirstReflexVertex(VertexArray const& polygon)
{
    for (std::size_t i = 0; i < polygon.size(); ++i)
    {
        float const handedness =
                fge::GetHandedness(polygon[(i - 1) % polygon.size()], polygon[i], polygon[(i + 1) % polygon.size()]);
        if (handedness < 0.0f)
        {
            return i;
        }
    }

    return std::nullopt;
}

void ConcavePolygon::flipPolygon()
{
    return;
    std::size_t iMax = this->g_vertices.size() / 2;

    if (this->g_vertices.size() % 2 != 0)
    {
        iMax += 1;
    }

    for (std::size_t i = 1; i < iMax; ++i)
    {
        std::swap(this->g_vertices[i], this->g_vertices[this->g_vertices.size() - i]);
    }
}

ConcavePolygon::VertexIndexMap
ConcavePolygon::cullByDistance(VertexIndexMap const& input, fge::Vector2f const& origin, std::size_t maxVerticesToKeep)
{
    struct DistanceVertex
    {
        DistanceVertex() = default;
        explicit DistanceVertex(float distance, VertexIndexMap::const_iterator it) :
                _distance{distance},
                _itInput{it}
        {}

        inline bool operator<(DistanceVertex const& r) const { return this->_distance < r._distance; }

        float _distance{};
        VertexIndexMap::const_iterator _itInput{};
    };

    if (maxVerticesToKeep >= input.size())
    {
        return input;
    }

    std::priority_queue<DistanceVertex> distanceVertices;

    for (auto it = input.begin(); it != input.end(); ++it)
    {
        distanceVertices.emplace(fge::DotSquare(it->second - origin), it);
    }

    VertexIndexMap result;
    for (std::size_t i = 0; i < maxVerticesToKeep; ++i)
    {
        result.insert({distanceVertices.top()._itInput->first, distanceVertices.top()._itInput->second});
        distanceVertices.pop();
    }

    return result;
}

ConcavePolygon::VertexIndexMap ConcavePolygon::verticesAlongLineSegment(fge::Line const& segment,
                                                                        VertexArray const& vertices)
{
    VertexIndexMap result;

    std::size_t lastIndex = std::numeric_limits<std::size_t>::max();

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        fge::Line const verticesSegment{vertices[i], vertices[(i + 1) % vertices.size()]};

        auto intersectionResult =
                fge::CheckIntersection(segment, verticesSegment, fge::IntersectionOptions::I_NORM_LIMITS);

        if (!intersectionResult)
        {
            continue;
        }

        if (lastIndex == (i - 1) % vertices.size())
        { //Avoid point duplication check
            if (fge::DotSquare(result[lastIndex] - intersectionResult->_point) < 1e-5f)
            {
                continue;
            }
        }

        lastIndex = i;
        result.insert({i, intersectionResult->_point});
    }

    return result;
}
ConcavePolygon::VertexIndexMap ConcavePolygon::verticesAlongLineSegment(fge::Vector2f const& position,
                                                                        fge::Vector2f const& direction,
                                                                        VertexArray const& vertices)
{
    VertexIndexMap result;

    std::size_t lastIndex = std::numeric_limits<std::size_t>::max();

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        fge::Line const verticesSegment{vertices[i], vertices[(i + 1) % vertices.size()]};

        auto intersectionResult =
                fge::CheckIntersection(position, direction, verticesSegment, fge::IntersectionOptions::I_NORM_LIMITS);

        if (!intersectionResult)
        {
            continue;
        }

        if (lastIndex == (i - 1) % vertices.size())
        { //Avoid point duplication check
            if (fge::DotSquare(result[lastIndex] - intersectionResult->_point) < 1e-5f)
            {
                continue;
            }
        }

        lastIndex = i;
        result.insert({i, intersectionResult->_point});
    }

    return result;
}

} // namespace fge
