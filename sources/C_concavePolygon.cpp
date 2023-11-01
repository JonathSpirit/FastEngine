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
    if (this->g_convexPolygons.empty())
    {
        auto polygons = ConcavePolygon::slicePolygon(segment, this->g_vertices);
        if (polygons)
        {
            this->g_convexPolygons.push_back(std::move(polygons.value().first));
            this->g_convexPolygons.push_back(std::move(polygons.value().second));
        }
        return;
    }

    auto oldSize = this->g_convexPolygons.size();
    for (std::size_t i = 0; i < oldSize; ++i)
    {
        auto vertices = ConcavePolygon::slicePolygon(segment, this->g_convexPolygons[i]);
        if (vertices)
        {
            this->g_convexPolygons.push_back(std::move(vertices.value().first));
            this->g_convexPolygons.push_back(std::move(vertices.value().second));
        }
    }
    this->g_convexPolygons.erase(this->g_convexPolygons.begin(), this->g_convexPolygons.begin() + oldSize);
}

void ConcavePolygon::convexDecomposition()
{
    if (!this->g_convexPolygons.empty())
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
            this->g_convexPolygons.push_back(std::move(polygon));
            continue;
        }

        auto reflexIndex = ConcavePolygon::findFirstReflexVertex(polygon);
        if (!reflexIndex)
        {
            this->g_convexPolygons.push_back(std::move(polygon));
            continue;
        }

        auto prevVertPos = polygon[(reflexIndex.value() - 1) % polygon.size()];
        auto currVertPos = polygon[reflexIndex.value()];
        auto nextVertPos = polygon[(reflexIndex.value() + 1) % polygon.size()];

        fge::Line const ls1(prevVertPos, currVertPos);
        fge::Line const ls2(nextVertPos, currVertPos);

        auto const verticesInCone = ConcavePolygon::findVerticesInCone(ls1, ls2, currVertPos, polygon);

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
        else
        {
            fge::Line const newLine(currVertPos, (ls1.getDirection() + ls2.getDirection()) * 1e+10f);

            auto polygons = ConcavePolygon::slicePolygon(newLine, polygon);
            if (polygons)
            {
                polygonQueue.emplace(std::move(polygons.value().first));
                polygonQueue.emplace(std::move(polygons.value().second));
            }
        }
    } while (!polygonQueue.empty());
}

void ConcavePolygon::clear()
{
    this->g_convexPolygons.clear();
}

std::optional<std::pair<ConcavePolygon::VertexArray, ConcavePolygon::VertexArray>>
ConcavePolygon::slicePolygon(fge::Line const& segment, VertexArray const& inputVertices)
{
    float const TOLERANCE = 1e-5f;

    auto slicedVertices = ConcavePolygon::verticesAlongLineSegment(segment, inputVertices);
    slicedVertices = ConcavePolygon::cullByDistance(slicedVertices, segment._start, 2);

    if (slicedVertices.size() < 2)
    {
        return std::nullopt;
    }

    VertexArray leftVertices;
    VertexArray rightVertices;

    for (std::size_t i = 0; i < inputVertices.size(); ++i)
    {
        auto relativePosition = inputVertices[i] - segment._start;

        auto it = slicedVertices.begin();

        float const perpDistance = std::abs(fge::Cross2d(relativePosition, segment.getDirection()));

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


std::pair<bool, fge::Vector2f> ConcavePolygon::intersects(fge::Line s1, fge::Line s2)
{
    float const TOLERANCE = 1e-2f;

    auto const p1 = s1._start;
    auto const p2 = s2._start;
    auto const d1 = s1.getDirection();
    auto const d2 = s2.getDirection();

    if (std::abs(fge::Cross2d(d1, d2)) < 1e-30f)
    {
        return {false, {0.0f, 0.0f}};
    }

    float const t1 = fge::Cross2d(p2 - p1, d2) / fge::Cross2d(d1, d2);

    if ((t1 < (0.0f - TOLERANCE)) || (t1 > (1.0f + TOLERANCE)))
    {
        return {false, {0.0f, 0.0f}};
    }

    auto pIntersect = p1 + d1 * t1;

    float const t2 = glm::dot(pIntersect - p2, s2._end - p2);

    if (t2 < (0.0f - TOLERANCE) || t2 / fge::DotSquare(s2._end - p2) >= 1.0f - TOLERANCE)
    {
        return {false, {0.0f, 0.0f}};
    }

    return {true, pIntersect};
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
                fge::IsVertexInCone(line1, line2, polygonVertices[index], origin) &&
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

        float minDistance = 1e+15f;
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
    struct SliceVertex : public fge::Vector2f
    {
        SliceVertex() = default;
        explicit SliceVertex(fge::Vector2f const& position, std::size_t index, float distanceToSlice) :
                fge::Vector2f{position},
                _index{index},
                _distanceToSlice{distanceToSlice}
        {}

        std::size_t _index{};
        float _distanceToSlice{};
    };

    if (maxVerticesToKeep >= input.size())
    {
        return input;
    }

    std::vector<SliceVertex> sliceVertices;
    sliceVertices.reserve(input.size());

    for (auto const& [index, vertex]: input)
    {
        sliceVertices.emplace_back(vertex, index, fge::DotSquare(vertex - origin));
    }

    for (std::size_t i = 1; i < sliceVertices.size(); ++i)
    {
        for (std::size_t j = i; j > 0 && sliceVertices[j]._distanceToSlice < sliceVertices[j - 1]._distanceToSlice; --j)
        {
            std::swap(sliceVertices[j], sliceVertices[j - 1]);
        }
    }

    sliceVertices.erase(sliceVertices.begin() + maxVerticesToKeep, sliceVertices.end());

    for (std::size_t i = 1; i < sliceVertices.size(); ++i)
    {
        for (std::size_t j = i; j > 0 && sliceVertices[j]._index < sliceVertices[j - 1]._index; --j)
        {
            std::swap(sliceVertices[j], sliceVertices[j - 1]);
        }
    }

    VertexIndexMap result;
    for (auto const& sliceVertex: sliceVertices)
    {
        result.insert({sliceVertex._index, static_cast<fge::Vector2f>(sliceVertex)});
    }

    return result;
}

ConcavePolygon::VertexIndexMap ConcavePolygon::verticesAlongLineSegment(fge::Line const& segment,
                                                                        VertexArray const& vertices)
{
    VertexIndexMap result;

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        fge::Line const tempSegment{vertices[i], vertices[(i + 1) % vertices.size()]};

        auto intersectionResult = ConcavePolygon::intersects(segment, tempSegment);

        if (intersectionResult.first)
        {
            result.insert({i, intersectionResult.second});
        }
    }

    return result;
}

} // namespace fge
