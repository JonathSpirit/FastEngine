/*
 * Copyright 2024 Guillaume Guillet
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
#include <map>
#include <vector>

/*
 * Original from : https://github.com/mjjq/ConvexDecomposition
 * Copyright (C) mjjq
 * License MIT
 * The algorithm for decomposing concave polygons to convex can be found here:
 * https://mpen.ca/406/bayazit (Mark Bayazit).
 *
 * Altered/Modified by Guillaume Guillet
 */

namespace fge
{

class FGE_API ConcavePolygon
{
public:
    using VertexArray = std::vector<fge::Vector2f>;

    ConcavePolygon() = default;
    explicit ConcavePolygon(VertexArray const& vertices);
    explicit ConcavePolygon(VertexArray&& vertices);
    ConcavePolygon(ConcavePolygon const& r) = default;
    ConcavePolygon(ConcavePolygon&& r) noexcept = default;

    ConcavePolygon& operator=(ConcavePolygon const& r) = default;
    ConcavePolygon& operator=(ConcavePolygon&& r) noexcept = default;

    [[nodiscard]] bool checkIfRightHanded();

    void convexDecomposition();

    void setVertices(VertexArray const& vertices);
    void setVertices(VertexArray&& vertices);
    [[nodiscard]] inline fge::Vector2f& vertex(std::size_t index) { return this->g_vertices[index]; }
    [[nodiscard]] inline fge::Vector2f const& vertex(std::size_t index) const { return this->g_vertices[index]; }
    [[nodiscard]] inline VertexArray const& vertices() const { return this->g_vertices; }

    [[nodiscard]] inline VertexArray const& subPolygon(std::size_t subPolyIndex) const
    {
        return this->g_subPolygons[subPolyIndex];
    }
    [[nodiscard]] inline std::size_t subPolygonCount() const { return this->g_subPolygons.size(); }

    [[nodiscard]] inline std::size_t totalVertexCount() const { return this->g_totalVertexCount; }

    void clear();

private:
    VertexArray g_vertices;
    std::vector<VertexArray> g_subPolygons;
    std::size_t g_totalVertexCount = 0;

    using VertexIndexMap = std::map<std::size_t, fge::Vector2f>;
    using Indices = std::vector<std::size_t>;

    [[nodiscard]] static bool checkIfRightHanded(VertexArray const& vertices);

    [[nodiscard]] static std::pair<ConcavePolygon::VertexArray, ConcavePolygon::VertexArray>
    slicePolygon(std::size_t const& startVertexIndex,
                 std::size_t const& stopVertexIndex,
                 VertexArray const& inputVertices);

    [[nodiscard]] static Indices findVerticesInCone(fge::Line const& line1,
                                                    fge::Line const& line2,
                                                    fge::Vector2f const& origin,
                                                    VertexArray const& inputVertices);

    [[nodiscard]] static bool checkVisibility(fge::Vector2f const& originalPosition,
                                              fge::Vector2f const& vert,
                                              VertexArray const& polygonVertices);

    [[nodiscard]] static std::optional<std::size_t>
    getBestVertexToConnect(Indices const& indices, VertexArray const& polygonVertices, fge::Vector2f const& origin);

    [[nodiscard]] static std::optional<std::size_t> findFirstReflexVertex(VertexArray const& polygon);

    static void flipPolygon(VertexArray& vertices);

    [[nodiscard]] static VertexIndexMap verticesAlongLineSegment(fge::Line const& segment, VertexArray const& vertices);

    [[nodiscard]] static std::optional<std::size_t>
    addNewVertex(std::size_t& positionIndex, fge::Vector2f const& direction, VertexArray& vertices);
};

} // namespace fge

#endif //_FGE_C_CONCAVEPOLYGON_HPP_INCLUDED