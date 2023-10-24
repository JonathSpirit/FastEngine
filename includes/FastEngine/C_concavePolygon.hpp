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

#ifndef _FGE_C_CONCAVEPOLYGON_HPP_INCLUDED
#define _FGE_C_CONCAVEPOLYGON_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_vector.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include <vector>

/*
 * Original from : https://github.com/mjjq/ConvexDecomposition
 * Copyright (C) mjjq
 * License MIT
 *
 * Altered/Modified by Guillaume Guillet
 */

namespace fge
{

inline float GetHandedness(fge::Vector2f const& v1, fge::Vector2f const& v2, fge::Vector2f const& v3)
{
    auto const edge1 = v2 - v1;
    auto const edge2 = v3 - v2;

    return fge::Cross2d(edge1, edge2);
}

inline float Square(fge::Vector2f const& vec)
{
    return glm::dot(vec, vec);
}

class FGE_API ConcavePolygon
{
public:
    using VertexArray = std::vector<fge::Vector2f>;
    using PolygonArray = std::vector<ConcavePolygon>;

    ConcavePolygon() = default;
    explicit ConcavePolygon(VertexArray const& vertices);
    explicit ConcavePolygon(VertexArray&& vertices);
    ConcavePolygon(ConcavePolygon const& r) = default;
    ConcavePolygon(ConcavePolygon&& r) noexcept = default;

    ConcavePolygon& operator=(ConcavePolygon const& r) = default;
    ConcavePolygon& operator=(ConcavePolygon&& r) noexcept = default;

    [[nodiscard]] bool checkIfRightHanded();

    void slicePolygon(std::size_t vertexIndex1, std::size_t vertexIndex2);
    void slicePolygon(fge::Line const& segment);

    void convexDecomposition();

    [[nodiscard]] std::optional<fge::Vector2f> getPoint(std::size_t index) const;
    [[nodiscard]] std::size_t getPointCount() const;

    [[nodiscard]] inline VertexArray& vertices() { return this->g_vertices; }
    [[nodiscard]] inline VertexArray const& vertices() const { return this->g_vertices; }

    [[nodiscard]] inline ConcavePolygon const& subPolygon(std::size_t subPolyIndex) const
    {
        return this->g_subPolygons[subPolyIndex];
    }
    [[nodiscard]] inline std::size_t subPolygonCount() const { return this->g_subPolygons.size(); }

    void returnLowestLevelPolys(PolygonArray& returnArray);

    void clear();

private:
    VertexArray g_vertices;
    PolygonArray g_subPolygons;

    using VertexIndexMap = std::map<std::size_t, fge::Vector2f>;
    using Indices = std::vector<std::size_t>;

    [[nodiscard]] bool
    isVertexInCone(fge::Line const& ls1, fge::Line const& ls2, fge::Vector2f const& origin, fge::Vector2f const& vert);

    [[nodiscard]] Indices findVerticesInCone(fge::Line const& ls1,
                                             fge::Line const& ls2,
                                             fge::Vector2f const& origin,
                                             VertexArray const& inputVerts);

    [[nodiscard]] bool checkVisibility(fge::Vector2f const& originalPosition,
                                       fge::Vector2f const& vert,
                                       VertexArray const& polygonVertices);

    [[nodiscard]] std::optional<std::size_t>
    getBestVertexToConnect(Indices const& indices, VertexArray const& polygonVertices, fge::Vector2f const& origin);

    [[nodiscard]] std::optional<std::size_t> findFirstReflexVertex(VertexArray const& vertices);

    void flipPolygon();

    [[nodiscard]] VertexIndexMap
    cullByDistance(VertexIndexMap const& input, fge::Vector2f const& origin, std::size_t maxVertsToKeep);

    [[nodiscard]] VertexIndexMap verticesAlongLineSegment(fge::Line const& segment, VertexArray const& vertices);
};

} // namespace fge

#endif //_FGE_C_CONCAVEPOLYGON_HPP_INCLUDED