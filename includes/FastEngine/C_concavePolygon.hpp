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

#ifndef _FGE_GRAPHIC_C_RECT_HPP_INCLUDED
#define _FGE_GRAPHIC_C_RECT_HPP_INCLUDED

#include "FastEngine/C_vector.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include <vector>
#include <iostream>

/*
 * Original from : https://github.com/mjjq/ConvexDecomposition
 * Copyright (C) mjjq
 * License MIT
 *
 * Altered/Modified by Guillaume Guillet
 */

namespace fge
{

inline float GetHandedness(fge::Vector2f const & v1,
                           fge::Vector2f const & v2,
                           fge::Vector2f const & v3)
{
    auto const edge1 = v2-v1;
    auto const edge2 = v3-v2;

    return fge::Cross2d(edge1, edge2);
}

inline float Square(fge::Vector2f const & vec)
{
    return glm::dot(vec, vec);
}


struct SliceVertex : public fge::Vector2f
{
    SliceVertex() = default;
    SliceVertex(fge::Vector2f const & position) : fge::Vector2f{position} {}

    std::size_t index;
    float distanceToSlice;
};

class ConcavePolygon
{
    using VertexArray = std::vector<fge::Vector2f>;
    using PolygonArray = std::vector<ConcavePolygon>;

    VertexArray _vertices;
    PolygonArray _subPolygons;


    void flipPolygon(VertexArray& vertices)
    {
        std::size_t iMax = vertices.size() / 2;

        if (vertices.size() % 2 != 0)
        {
            iMax += 1;
        }

        for (std::size_t i=1; i<iMax; ++i)
        {
            std::swap(vertices[i], vertices[vertices.size()-i]);
        }
    }

    bool checkIfRightHanded(VertexArray const& vertices)
    {
        if(vertices.size() < 3)
        {
            return false;
        }

        float signedArea = 0.0f;

        for (std::size_t i=0; i<vertices.size(); ++i)
        {
            auto const& vert1 = vertices[i];
            auto const& vert2 = vertices[(i+1) % vertices.size()];
            signedArea += (vert2.x - vert1.x) * (vert2.y + vert1.y);
        }

        return signedArea < 0.0f;
    }



    bool isVertexInCone(fge::Line const& ls1,
                        fge::Line const& ls2,
                        fge::Vector2f const & origin,
                        fge::Vector2f const & vert)
    {
        auto const relativePos = vert - origin;

        auto const ls1Product = fge::Cross2d(relativePos, ls1.getDirection());
        auto const ls2Product = fge::Cross2d(relativePos, ls2.getDirection());

        return ls1Product < 0.0f && ls2Product > 0.0f;
    }

    using Indices = std::vector<std::size_t>;

    Indices findVerticesInCone(fge::Line const & ls1,
                                fge::Line const & ls2,
                                fge::Vector2f const & origin,
                                VertexArray const & inputVerts)
    {
        Indices result;

        for(std::size_t i=0; i<inputVerts.size(); ++i)
        {
            if (isVertexInCone(ls1, ls2, origin, inputVerts[i]))
            {
                result.push_back(i);
            }
        }
        return result;
    }

    bool checkVisibility(fge::Vector2f const & originalPosition,
                         fge::Vector2f const & vert,
                         VertexArray const & polygonVertices)
    {
        fge::Line const ls(originalPosition, vert);
        auto intersectingVerts = verticesAlongLineSegment(ls, polygonVertices);

        std::cout << intersectingVerts.size() << " intverts\n";

        return intersectingVerts.size() <= 3;
    }


    std::optional<std::size_t> getBestVertexToConnect(Indices const & indices,
                               VertexArray const & polygonVertices,
                               fge::Vector2f const & origin)
    {
        if (indices.size()==1)
        {
            if(checkVisibility(origin, polygonVertices[indices[0]], polygonVertices))
            {
                return indices[0];
            }
        }
        else if(indices.size() > 1)
        {
            for (std::size_t i=0; i<indices.size(); ++i)
            {
                auto index = indices[i];
                auto vertSize = polygonVertices.size();

                auto prevVert = polygonVertices[(index-1) % vertSize];
                auto currVert = polygonVertices[index];
                auto nextVert = polygonVertices[(index+1) %vertSize];

                fge::Line ls1(prevVert, currVert);
                fge::Line ls2(nextVert, currVert);

                if((fge::GetHandedness(prevVert, currVert, nextVert) < 0.0f) &&
                    isVertexInCone(ls1, ls2, polygonVertices[index], origin) &&
                    checkVisibility(origin, polygonVertices[index], polygonVertices))
                {
                    return index;
                }
            }

            for(std::size_t i=0; i<indices.size(); ++i)
            {
                auto index = indices[i];
                auto vertSize = polygonVertices.size();

                auto prevVert = polygonVertices[(index-1) % vertSize];
                auto currVert = polygonVertices[index];
                auto nextVert = polygonVertices[(index+1) % vertSize];

                fge::Line ls1(prevVert, currVert);
                fge::Line ls2(nextVert, currVert);

                if((fge::GetHandedness(prevVert, currVert, nextVert) < 0.0f) &&
                    checkVisibility(origin, polygonVertices[index], polygonVertices))
                {
                    return index;
                }
            }


            float minDistance = 1e+15f;
            auto closest = indices[0];
            for(std::size_t i=0; i<indices.size(); ++i)
            {
                auto index = indices[i];
                float currDistance = fge::Square(polygonVertices[index] - origin);
                if(currDistance < minDistance)
                {
                    minDistance = currDistance;
                    closest = index;
                }
            }

            return closest;
        }


        return std::nullopt;
    }

    void convexDecomp(VertexArray const & vertices)
    {
        if (!this->_subPolygons.empty())
        {
            return;
        }

        auto reflexIndex = findFirstReflexVertex(vertices);
        if (!reflexIndex)
        {
            return;
        }

        auto prevVertPos = vertices[(reflexIndex.value()-1) % vertices.size()];
        auto currVertPos = vertices[reflexIndex.value()];
        auto nextVertPos = vertices[(reflexIndex.value()+1) % vertices.size()];

        fge::Line ls1(prevVertPos, currVertPos);
        fge::Line ls2(nextVertPos, currVertPos);

        auto vertsInCone = findVerticesInCone(ls1, ls2, currVertPos, vertices);

        std::optional<std::size_t> bestVert = std::nullopt;

        if(!vertsInCone.empty())
        {
            bestVert = getBestVertexToConnect(vertsInCone, vertices, currVertPos);
            if(bestVert)
            {
                fge::Line newLine(currVertPos, vertices[bestVert.value()]);

                slicePolygon(newLine);
            }
        }
        if(vertsInCone.empty() || !bestVert)
        {
            fge::Line newLine(currVertPos, (ls1.getDirection() + ls2.getDirection()) * 1e+10f);
            slicePolygon(newLine);
        }

        for(std::size_t i=0; i<this->_subPolygons.size(); ++i)
        {
            this->_subPolygons[i].convexDecomp();
        }
    }

    std::optional<std::size_t> findFirstReflexVertex(VertexArray const & vertices)
    {
        for(std::size_t i=0; i<vertices.size(); ++i)
        {
            float handedness = fge::GetHandedness(vertices[(i-1) % vertices.size()],
                                                     vertices[i],
                                                     vertices[(i+1) % vertices.size()]);
            if(handedness < 0.0f)
            {
                return i;
            }
        }

        return std::nullopt;
    }

    void flipPolygon()
    {
        flipPolygon(this->_vertices);
    }

    using VertexIntMap = std::map<std::size_t, fge::Vector2f>;
    using VertexIntPair = std::pair<std::size_t, fge::Vector2f>;


    VertexIntMap cullByDistance(VertexIntMap const & input,
                                fge::Vector2f const & origin,
                                std::size_t maxVertsToKeep)
    {
        if(maxVertsToKeep >= input.size())
        {
            return input;
        }

        std::vector<SliceVertex> sliceVertices;

        for(auto it = input.begin(); it != input.end(); ++it)
        {
            SliceVertex vert(it->second);
            vert.index = it->first;
            vert.distanceToSlice = fge::Square(it->second - origin);

            sliceVertices.push_back(vert);
        }

        for(std::size_t i=1; i<sliceVertices.size(); ++i)
        {
            for (std::size_t j = i; j > 0 && sliceVertices[j].distanceToSlice < sliceVertices[j - 1].distanceToSlice; --j)
            {
                std::swap(sliceVertices[j], sliceVertices[j - 1]);
            }
        }

        sliceVertices.erase(sliceVertices.begin()+maxVertsToKeep, sliceVertices.end());

        for(std::size_t i=1; i<sliceVertices.size(); ++i)
        {
            for (std::size_t j = i; j > 0 && sliceVertices[j].index < sliceVertices[j - 1].index; --j)
            {
                std::swap(sliceVertices[j], sliceVertices[j - 1]);
            }
        }

        VertexIntMap result;
        for(std::size_t i=0; i<sliceVertices.size(); ++i)
        {
            result.insert({sliceVertices[i].index, fge::Vector2f(sliceVertices[i])});
        }

        return result;
    }

    VertexIntMap verticesAlongLineSegment(fge::Line const & segment,
                                          VertexArray const & vertices)
    {
        VertexIntMap result;

        fge::Line tempSegment;

        for(std::size_t i=0; i<vertices.size(); ++i)
        {
            tempSegment._start = vertices[i];
            tempSegment._end = vertices[(i+1) % vertices.size()];

            auto intersectionResult = fge::CheckIntersection(segment, tempSegment);

            if(intersectionResult)
            {
                result.insert({i, intersectionResult.value()._point});
            }
        }

        return result;
    }

public:
    ConcavePolygon(VertexArray const & vertices) : _vertices{vertices}
    {
        if(vertices.size() > 2)
        {
            if(!checkIfRightHanded())
            {
                flipPolygon();
            }
        }
    }
    ConcavePolygon() {}

    bool checkIfRightHanded()
    {
        return checkIfRightHanded(this->_vertices);
    }

    void slicePolygon(std::size_t vertex1, std::size_t vertex2)
    {
        if(vertex1 == vertex2 ||
            vertex2 == vertex1+1 ||
            vertex2 == vertex1-1)
        {
            return;
        }

        if(vertex1 > vertex2)
        {
            std::swap(vertex1, vertex2);
        }

        VertexArray returnVerts;
        VertexArray newVerts;
        for(std::size_t i=0; i<this->_vertices.size(); ++i)
        {
            if(i==vertex1 || i==vertex2)
            {
                returnVerts.push_back(this->_vertices[i]);
                newVerts.push_back(this->_vertices[i]);
            }
            else if(i > vertex1 && i <vertex2)
            {
                returnVerts.push_back(this->_vertices[i]);
            }
            else
            {
                newVerts.push_back(this->_vertices[i]);
            }
        }

        this->_subPolygons.push_back(ConcavePolygon(returnVerts));
        this->_subPolygons.push_back(ConcavePolygon(newVerts));
    }

    void slicePolygon(fge::Line const& segment)
    {
        if(!this->_subPolygons.empty())
        {
            this->_subPolygons[0].slicePolygon(segment);
            this->_subPolygons[1].slicePolygon(segment);
            return;
        }

        float const TOLERANCE = 1e-5f;

        VertexIntMap slicedVertices = verticesAlongLineSegment(segment, this->_vertices);
        slicedVertices = cullByDistance(slicedVertices, segment._start, 2);

        if(slicedVertices.size() < 2)
        {
            return;
        }

        VertexArray leftVerts;
        VertexArray rightVerts;

        for(std::size_t i=0; i<this->_vertices.size(); ++i)
        {
            auto relativePosition = this->_vertices[i] - segment._start;

            auto it = slicedVertices.begin();

            float perpDistance = std::abs(fge::Cross2d(relativePosition, segment.getDirection()));

            if( perpDistance > TOLERANCE ||
                ( perpDistance <= TOLERANCE && (slicedVertices.find(i)==slicedVertices.end()) )
            )
            {
                //std::cout << relCrossProd << ", i: " << i << "\n";
                if((i > it->first) && (i <= (++it)->first))
                {
                    leftVerts.push_back(this->_vertices[i]);
                    //std::cout << i << " leftVertAdded\n";
                }
                else
                {
                    rightVerts.push_back(this->_vertices[i]);
                    //std::cout << i << " rightVertAdded\n";
                }

            }

            if(slicedVertices.find(i) != slicedVertices.end())
            {
                rightVerts.push_back(slicedVertices[i]);
                leftVerts.push_back(slicedVertices[i]);
            }
        }

        this->_subPolygons.push_back(ConcavePolygon(leftVerts));
        this->_subPolygons.push_back(ConcavePolygon(rightVerts));
    }

    void convexDecomp()
    {
        if(this->_vertices.size() > 3)
        {
            convexDecomp(this->_vertices);
        }
    }

    VertexArray const & getVertices() const
    {
        return this->_vertices;
    }

    ConcavePolygon const & getSubPolygon(std::size_t subPolyIndex) const
    {
        if(!this->_subPolygons.empty() && subPolyIndex < this->_subPolygons.size())
        {
            return this->_subPolygons[subPolyIndex];
        }

        return *this;
    }

    std::size_t getNumberSubPolys() const
    {
        return this->_subPolygons.size();
    }

    void returnLowestLevelPolys(std::vector<ConcavePolygon > & returnArr)
    {
        if(this->_subPolygons.size() > 0)
        {
            this->_subPolygons[0].returnLowestLevelPolys(returnArr);
            this->_subPolygons[1].returnLowestLevelPolys(returnArr);
        }
        else
        {
            returnArr.push_back(*this);
        }
    }

    void reset()
    {
        if(this->_subPolygons.size() > 0)
        {
            this->_subPolygons[0].reset();
            this->_subPolygons[1].reset();
            this->_subPolygons.clear();
        }
    }

    fge::Vector2f getPoint(std::size_t index) const
    {
        if(index >= 0 && index < this->_vertices.size())
        {
            return this->_vertices[index];
        }

        return {0.0f, 0.0f};
    }

    std::size_t getPointCount() const
    {
        return this->_vertices.size();
    }
};

} // namespace fge

#endif //_FGE_GRAPHIC_C_RECT_HPP_INCLUDED