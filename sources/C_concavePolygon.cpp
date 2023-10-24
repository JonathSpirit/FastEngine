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

    auto slicedVertices = this->verticesAlongLineSegment(segment, this->g_vertices);
    slicedVertices = this->cullByDistance(slicedVertices, segment._start, 2);

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

    auto reflexIndex = this->findFirstReflexVertex(this->g_vertices);
    if (!reflexIndex)
    {
        return;
    }

    auto prevVertPos = this->g_vertices[(reflexIndex.value() - 1) % this->g_vertices.size()];
    auto currVertPos = this->g_vertices[reflexIndex.value()];
    auto nextVertPos = this->g_vertices[(reflexIndex.value() + 1) % this->g_vertices.size()];

    fge::Line const ls1(prevVertPos, currVertPos);
    fge::Line const ls2(nextVertPos, currVertPos);

    auto vertsInCone = this->findVerticesInCone(ls1, ls2, currVertPos, this->g_vertices);

    std::optional<std::size_t> bestVert = std::nullopt;

    if (!vertsInCone.empty())
    {
        bestVert = this->getBestVertexToConnect(vertsInCone, this->g_vertices, currVertPos);
        if (bestVert)
        {
            fge::Line const newLine(currVertPos, this->g_vertices[bestVert.value()]);

            this->slicePolygon(newLine);
        }
    }
    if (vertsInCone.empty() || !bestVert)
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
    if (!this->g_subPolygons.empty())
    {
        this->g_subPolygons[0].clear();
        this->g_subPolygons[1].clear();
        this->g_subPolygons.clear();
    }
}

bool ConcavePolygon::isVertexInCone(fge::Line const& ls1,
                                    fge::Line const& ls2,
                                    fge::Vector2f const& origin,
                                    fge::Vector2f const& vert)
{
    auto const relativePos = vert - origin;

    auto const ls1Product = fge::Cross2d(relativePos, ls1.getDirection());
    auto const ls2Product = fge::Cross2d(relativePos, ls2.getDirection());

    return ls1Product < 0.0f && ls2Product > 0.0f;
}

ConcavePolygon::Indices ConcavePolygon::findVerticesInCone(fge::Line const& ls1,
                                                           fge::Line const& ls2,
                                                           fge::Vector2f const& origin,
                                                           VertexArray const& inputVerts)
{
    Indices result;

    for (std::size_t i = 0; i < inputVerts.size(); ++i)
    {
        if (isVertexInCone(ls1, ls2, origin, inputVerts[i]))
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
    auto intersectingVerts = verticesAlongLineSegment(ls, polygonVertices);

    return intersectingVerts.size() <= 3;
}


std::optional<std::size_t> ConcavePolygon::getBestVertexToConnect(Indices const& indices,
                                                                  VertexArray const& polygonVertices,
                                                                  fge::Vector2f const& origin)
{
    if (indices.size() == 1)
    {
        if (checkVisibility(origin, polygonVertices[indices[0]], polygonVertices))
        {
            return indices[0];
        }
    }
    else if (indices.size() > 1)
    {
        for (std::size_t i = 0; i < indices.size(); ++i)
        {
            auto index = indices[i];
            auto vertSize = polygonVertices.size();

            auto prevVert = polygonVertices[(index - 1) % vertSize];
            auto currVert = polygonVertices[index];
            auto nextVert = polygonVertices[(index + 1) % vertSize];

            fge::Line const ls1(prevVert, currVert);
            fge::Line const ls2(nextVert, currVert);

            if ((fge::GetHandedness(prevVert, currVert, nextVert) < 0.0f) &&
                isVertexInCone(ls1, ls2, polygonVertices[index], origin) &&
                checkVisibility(origin, polygonVertices[index], polygonVertices))
            {
                return index;
            }
        }

        for (std::size_t i = 0; i < indices.size(); ++i)
        {
            auto index = indices[i];
            auto vertSize = polygonVertices.size();

            auto prevVert = polygonVertices[(index - 1) % vertSize];
            auto currVert = polygonVertices[index];
            auto nextVert = polygonVertices[(index + 1) % vertSize];

            fge::Line const ls1(prevVert, currVert);
            fge::Line const ls2(nextVert, currVert);

            if ((fge::GetHandedness(prevVert, currVert, nextVert) < 0.0f) &&
                checkVisibility(origin, polygonVertices[index], polygonVertices))
            {
                return index;
            }
        }


        float minDistance = 1e+15f;
        auto closest = indices[0];
        for (std::size_t i = 0; i < indices.size(); ++i)
        {
            auto index = indices[i];
            float const currDistance = fge::Square(polygonVertices[index] - origin);
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

std::optional<std::size_t> ConcavePolygon::findFirstReflexVertex(VertexArray const& vertices)
{
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        float const handedness = fge::GetHandedness(vertices[(i - 1) % vertices.size()], vertices[i],
                                                    vertices[(i + 1) % vertices.size()]);
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
ConcavePolygon::cullByDistance(VertexIndexMap const& input, fge::Vector2f const& origin, std::size_t maxVertsToKeep)
{
    struct SliceVertex : public fge::Vector2f
    {
        SliceVertex() = default;
        SliceVertex(fge::Vector2f const& position) :
                fge::Vector2f{position}
        {}

        std::size_t _index;
        float _distanceToSlice;
    };

    if (maxVertsToKeep >= input.size())
    {
        return input;
    }

    std::vector<SliceVertex> sliceVertices;

    for (auto it = input.begin(); it != input.end(); ++it)
    {
        SliceVertex vert(it->second);
        vert._index = it->first;
        vert._distanceToSlice = fge::Square(it->second - origin);

        sliceVertices.push_back(vert);
    }

    for (std::size_t i = 1; i < sliceVertices.size(); ++i)
    {
        for (std::size_t j = i; j > 0 && sliceVertices[j]._distanceToSlice < sliceVertices[j - 1]._distanceToSlice; --j)
        {
            std::swap(sliceVertices[j], sliceVertices[j - 1]);
        }
    }

    sliceVertices.erase(sliceVertices.begin() + maxVertsToKeep, sliceVertices.end());

    for (std::size_t i = 1; i < sliceVertices.size(); ++i)
    {
        for (std::size_t j = i; j > 0 && sliceVertices[j]._index < sliceVertices[j - 1]._index; --j)
        {
            std::swap(sliceVertices[j], sliceVertices[j - 1]);
        }
    }

    VertexIndexMap result;
    for (std::size_t i = 0; i < sliceVertices.size(); ++i)
    {
        result.insert({sliceVertices[i]._index, fge::Vector2f(sliceVertices[i])});
    }

    return result;
}

ConcavePolygon::VertexIndexMap ConcavePolygon::verticesAlongLineSegment(fge::Line const& segment,
                                                                        VertexArray const& vertices)
{
    VertexIndexMap result;

    fge::Line tempSegment;

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        tempSegment._start = vertices[i];
        tempSegment._end = vertices[(i + 1) % vertices.size()];

        auto intersectionResult = fge::CheckIntersection(segment, tempSegment);

        if (intersectionResult)
        {
            result.insert({i, intersectionResult.value()._point});
        }
    }

    return result;
}

} // namespace fge
