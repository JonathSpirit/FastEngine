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
{}
ConcavePolygon::ConcavePolygon(VertexArray&& vertices) :
        g_vertices{std::move(vertices)}
{}

bool ConcavePolygon::checkIfRightHanded()
{
    return ConcavePolygon::checkIfRightHanded(this->g_vertices);
}
bool ConcavePolygon::checkIfRightHanded(VertexArray const& vertices)
{
    if (vertices.size() < 3)
    {
        return false;
    }

    float signedArea = 0.0f;

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        auto const& vert1 = vertices[i];
        auto const& vert2 = vertices[(i + 1) % vertices.size()];
        signedArea += (vert2.x - vert1.x) * (vert2.y + vert1.y);
    }

    return signedArea < 0.0f;
}

void ConcavePolygon::convexDecomposition()
{
    if (!this->g_subPolygons.empty())
    {
        return;
    }

    std::queue<VertexArray> polygonQueue;
    polygonQueue.emplace(this->g_vertices);

    if (polygonQueue.back().size() > 2)
    {
        if (!ConcavePolygon::checkIfRightHanded(polygonQueue.back()))
        {
            ConcavePolygon::flipPolygon(polygonQueue.back());
        }
    }

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

        auto const& prevVertPos = polygon[(reflexIndex.value() == 0) ? polygon.size() - 1 : reflexIndex.value() - 1];
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
                auto startIndex = reflexIndex.value();

                auto polygons = ConcavePolygon::slicePolygon(startIndex, bestVert.value(), polygon);
                if (!polygons.first.empty())
                {
                    polygonQueue.emplace(std::move(polygons.first));
                }
                if (!polygons.second.empty())
                {
                    polygonQueue.emplace(std::move(polygons.second));
                }
            }
        }
        if (verticesInCone.empty() || !bestVert)
        {
            auto const direction = -glm::normalize((line1._start - line1._end) + (line2._start - line2._end));

            auto startIndex = reflexIndex.value();
            auto endIndex = ConcavePolygon::addNewVertex(startIndex, direction, polygon);

            if (!endIndex)
            {
                continue;
            }

            auto polygons = ConcavePolygon::slicePolygon(startIndex, endIndex.value(), polygon);
            if (!polygons.first.empty())
            {
                polygonQueue.emplace(std::move(polygons.first));
            }
            if (!polygons.second.empty())
            {
                polygonQueue.emplace(std::move(polygons.second));
            }
        }
    } while (!polygonQueue.empty());
}

void ConcavePolygon::setVertices(VertexArray const& vertices)
{
    this->g_subPolygons.clear();
    this->g_vertices = vertices;
}
void ConcavePolygon::setVertices(VertexArray&& vertices)
{
    this->g_subPolygons.clear();
    this->g_vertices = std::move(vertices);
}

void ConcavePolygon::clear()
{
    this->g_subPolygons.clear();
}

std::pair<ConcavePolygon::VertexArray, ConcavePolygon::VertexArray>
ConcavePolygon::slicePolygon(std::size_t const& startVertexIndex,
                             std::size_t const& stopVertexIndex,
                             VertexArray const& vertices)
{
    if (startVertexIndex == stopVertexIndex || startVertexIndex >= vertices.size() ||
        stopVertexIndex >= vertices.size())
    {
        return {};
    }

    VertexArray leftVertices;
    VertexArray rightVertices;

    bool left = false;

    for (std::size_t i = startVertexIndex; i < startVertexIndex + vertices.size() + 1; ++i)
    {
        auto const a = i % vertices.size();

        if (left)
        {
            leftVertices.push_back(vertices[a]);
        }
        else
        {
            rightVertices.push_back(vertices[a]);
            if (a == stopVertexIndex)
            {
                if ((a + 1) % vertices.size() == startVertexIndex)
                {
                    break;
                }
                leftVertices.push_back(vertices[a]);
                left = true;
            }
        }
    }

    return {std::move(leftVertices), std::move(rightVertices)};
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

            auto const prevVert = polygonVertices[(index == 0) ? vertSize - 1 : index - 1];
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
        float const handedness = fge::GetHandedness(polygon[(i == 0) ? polygon.size() - 1 : i - 1], polygon[i],
                                                    polygon[(i + 1) % polygon.size()]);
        if (handedness < 0.0f)
        {
            return i;
        }
    }

    return std::nullopt;
}

void ConcavePolygon::flipPolygon(VertexArray& vertices)
{
    std::size_t iMax = vertices.size() / 2;

    if (vertices.size() % 2 != 0)
    {
        iMax += 1;
    }

    for (std::size_t i = 1; i < iMax; ++i)
    {
        std::swap(vertices[i], vertices[vertices.size() - i]);
    }
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

std::optional<std::size_t>
ConcavePolygon::addNewVertex(std::size_t& positionIndex, fge::Vector2f const& direction, VertexArray& vertices)
{
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        fge::Line const segment{vertices[i], vertices[(i + 1) % vertices.size()]};

        auto intersectionResult = fge::CheckIntersection(vertices[positionIndex], direction, segment);

        if (!intersectionResult)
        {
            continue;
        }

        if (intersectionResult->_normA < 1e-5f)
        {
            continue;
        }

        if (i < positionIndex)
        {
            positionIndex += 1;
        }

        vertices.insert(vertices.begin() + (i + 1), intersectionResult->_point);
        return i + 1;
    }

    return std::nullopt;
}

} // namespace fge
