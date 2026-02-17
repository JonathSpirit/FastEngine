/*
 * Copyright 2026 Guillaume Guillet
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

#include "FastEngine/C_quad.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

bool Quad::contains(Vector2f const& point) const
{
    auto base1 = glm::length(this->_points[1] - this->_points[0]);
    auto base2 = glm::length(this->_points[2] - this->_points[1]);
    auto base3 = glm::length(this->_points[3] - this->_points[2]);
    auto base4 = glm::length(this->_points[0] - this->_points[3]);

    auto distance1 = fge::GetShortestDistanceBetween(point, this->_points[0], this->_points[1]);
    auto distance2 = fge::GetShortestDistanceBetween(point, this->_points[1], this->_points[2]);
    auto distance3 = fge::GetShortestDistanceBetween(point, this->_points[2], this->_points[3]);
    auto distance4 = fge::GetShortestDistanceBetween(point, this->_points[3], this->_points[0]);

    auto computedArea = (base1 * distance1 + base2 * distance2 + base3 * distance3 + base4 * distance4) / 2.0f;

    auto base = glm::length(this->_points[1] - this->_points[3]);

    distance1 = fge::GetShortestDistanceBetween(this->_points[0], this->_points[3], this->_points[1]);
    distance2 = fge::GetShortestDistanceBetween(this->_points[2], this->_points[3], this->_points[1]);

    auto area = (base * distance1 + base * distance2) / 2.0f;

    return std::abs(computedArea - area) / std::max(computedArea, area) <= 0.02f;
}
bool Quad::intersects(Quad const& quad) const
{
    //Checking all lines intersection
    for (std::size_t ia = 0; ia < this->_points.size(); ++ia)
    {
        fge::Line const lineA{this->_points[ia], this->_points[(ia + 1) % this->_points.size()]};
        for (std::size_t ib = 0; ib < this->_points.size(); ++ib)
        {
            fge::Line const lineB{quad[ib], quad[(ib + 1) % this->_points.size()]};
            if (fge::CheckIntersection(lineA, lineB))
            {
                return true;
            }
        }
    }

    //Great nothing intersect but the quad can always be completely inside this->_points
    //(or the opposite)
    return this->contains(quad[0]) || quad.contains(this->_points[0]);
}

} // namespace fge