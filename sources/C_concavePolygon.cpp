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

void ConcavePolygon::slicePolygon(std::size_t vertexIndex1, std::size_t vertexIndex2)
{
    if (vertexIndex1 == vertexIndex2 || vertexIndex2 == vertexIndex1 + 1 || vertexIndex2 == vertexIndex1 - 1)
    {
        return;
    }

    if (vertexIndex1 > vertexIndex2)
    {
        std::swap(vertexIndex1, vertexIndex2);
    }

    VertexArray returnVerts;
    VertexArray newVerts;
    for (std::size_t i = 0; i < this->g_vertices.size(); ++i)
    {
        if (i == vertexIndex1 || i == vertexIndex2)
        {
            returnVerts.push_back(this->g_vertices[i]);
            newVerts.push_back(this->g_vertices[i]);
        }
        else if (i > vertexIndex1 && i < vertexIndex2)
        {
            returnVerts.push_back(this->g_vertices[i]);
        }
        else
        {
            newVerts.push_back(this->g_vertices[i]);
        }
    }

    this->g_subPolygons.emplace_back(std::move(returnVerts));
    this->g_subPolygons.emplace_back(std::move(newVerts));
}
void ConcavePolygon::slicePolygon(fge::Line const& segment)
{
    if (!this->g_subPolygons.empty())
    {
        this->g_subPolygons[0].slicePolygon(segment);
        this->g_subPolygons[1].slicePolygon(segment);
        return;
    }

    float const TOLERANCE = 1e-5f;

    auto slicedVertices = ConcavePolygon::verticesAlongLineSegment(segment, this->g_vertices);
    slicedVertices = ConcavePolygon::cullByDistance(slicedVertices, segment._start, 2);

    if (slicedVertices.size() < 2)
    {
        return;
    }

    VertexArray leftVerts;
    VertexArray rightVerts;

    for (std::size_t i = 0; i < this->g_vertices.size(); ++i)
    {
        auto relativePosition = this->g_vertices[i] - segment._start;

        auto it = slicedVertices.begin();

        float const perpDistance = std::abs(fge::Cross2d(relativePosition, segment.getDirection()));

        if (perpDistance > TOLERANCE || (perpDistance <= TOLERANCE && (slicedVertices.find(i) == slicedVertices.end())))
        {
            if ((i > it->first) && (i <= (++it)->first))
            {
                leftVerts.push_back(this->g_vertices[i]);
            }
            else
            {
                rightVerts.push_back(this->g_vertices[i]);
            }
        }

        if (slicedVertices.find(i) != slicedVertices.end())
        {
            rightVerts.push_back(slicedVertices[i]);
            leftVerts.push_back(slicedVertices[i]);
        }
    }

    this->g_subPolygons.emplace_back(std::move(leftVerts));
    this->g_subPolygons.emplace_back(std::move(rightVerts));
}

void ConcavePolygon::convexDecomposition()
{
    if (this->g_vertices.size() <= 3)
    {
        return;
    }

    if (!this->g_subPolygons.empty())
    {
        return;
    }

    auto reflexIndex = this->findFirstReflexVertex();
    if (!reflexIndex)
    {
        return;
    }

    auto prevVertPos = this->g_vertices[(reflexIndex.value() - 1) % this->g_vertices.size()];
    auto currVertPos = this->g_vertices[reflexIndex.value()];
    auto nextVertPos = this->g_vertices[(reflexIndex.value() + 1) % this->g_vertices.size()];

    fge::Line const ls1(prevVertPos, currVertPos);
    fge::Line const ls2(nextVertPos, currVertPos);

    auto const verticesInCone = ConcavePolygon::findVerticesInCone(ls1, ls2, currVertPos, this->g_vertices);

    std::optional<std::size_t> bestVert{};

    if (!verticesInCone.empty())
    {
        bestVert = this->getBestVertexToConnect(verticesInCone, this->g_vertices, currVertPos);
        if (bestVert)
        {
            fge::Line const newLine(currVertPos, this->g_vertices[bestVert.value()]);

            this->slicePolygon(newLine);
        }
    }
    else
    {
        fge::Line const newLine(currVertPos, (ls1.getDirection() + ls2.getDirection()) * 1e+10f);
        this->slicePolygon(newLine);
    }

    for (auto& polygon: this->g_subPolygons)
    {
        polygon.convexDecomposition();
    }
}

std::optional<fge::Vector2f> ConcavePolygon::getPoint(std::size_t index) const
{
    if (index < this->g_vertices.size())
    {
        return this->g_vertices[index];
    }

    return std::nullopt;
}
std::size_t ConcavePolygon::getPointCount() const
{
    return this->g_vertices.size();
}

void ConcavePolygon::returnLowestLevelPolys(PolygonArray& returnArray)
{
    if (!this->g_subPolygons.empty())
    {
        this->g_subPolygons[0].returnLowestLevelPolys(returnArray);
        this->g_subPolygons[1].returnLowestLevelPolys(returnArray);
    }
    else
    {
        returnArray.push_back(*this);
    }
}

void ConcavePolygon::clear()
{
    this->g_subPolygons.clear();
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
        if (this->checkVisibility(origin, polygonVertices[indices.front()], polygonVertices))
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
                this->checkVisibility(origin, polygonVertices[index], polygonVertices))
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

std::optional<std::size_t> ConcavePolygon::findFirstReflexVertex()
{
    for (std::size_t i = 0; i < this->g_vertices.size(); ++i)
    {
        float const handedness =
                fge::GetHandedness(this->g_vertices[(i - 1) % this->g_vertices.size()], this->g_vertices[i],
                                   this->g_vertices[(i + 1) % this->g_vertices.size()]);
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
