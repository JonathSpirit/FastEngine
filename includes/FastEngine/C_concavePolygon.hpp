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
    using PolygonArray = std::vector<ConcavePolygon>;

    ConcavePolygon() = default;
    explicit ConcavePolygon(VertexArray const& vertices);
    explicit ConcavePolygon(VertexArray&& vertices);
    ConcavePolygon(ConcavePolygon const& r) = default;
    ConcavePolygon(ConcavePolygon&& r) noexcept = default;

    ConcavePolygon& operator=(ConcavePolygon const& r) = default;
    ConcavePolygon& operator=(ConcavePolygon&& r) noexcept = default;

    [[nodiscard]] bool checkIfRightHanded();

    void slicePolygon(fge::Line const& segment);

    void convexDecomposition();

    [[nodiscard]] inline VertexArray& vertices() { return this->g_vertices; }
    [[nodiscard]] inline VertexArray const& vertices() const { return this->g_vertices; }

    [[nodiscard]] inline VertexArray const& subPolygon(std::size_t subPolyIndex) const
    {
        return this->g_subPolygons[subPolyIndex];
    }
    [[nodiscard]] inline std::size_t subPolygonCount() const { return this->g_subPolygons.size(); }

    void clear();

private:
    VertexArray g_vertices;
    std::vector<VertexArray> g_subPolygons;

    using VertexIndexMap = std::map<std::size_t, fge::Vector2f>;
    using Indices = std::vector<std::size_t>;

    [[nodiscard]] static std::optional<std::pair<VertexArray, VertexArray>>
    slicePolygon(fge::Line const& segment, VertexArray const& inputVertices);

    [[nodiscard]] static std::pair<bool, fge::Vector2f> intersects(fge::Line s1, fge::Line s2);

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

    void flipPolygon();

    [[nodiscard]] static VertexIndexMap
    cullByDistance(VertexIndexMap const& input, fge::Vector2f const& origin, std::size_t maxVertsToKeep);

    [[nodiscard]] static VertexIndexMap verticesAlongLineSegment(fge::Line const& segment, VertexArray const& vertices);
};

} // namespace fge

#endif //_FGE_C_CONCAVEPOLYGON_HPP_INCLUDED