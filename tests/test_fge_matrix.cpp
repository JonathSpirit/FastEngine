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

#include "doctest/doctest.h"
#include "FastEngine/C_matrix.hpp"

TEST_CASE("testing matrix<int>")
{
    fge::Matrix<int> matrix;

    matrix.setSize(3,3);
    REQUIRE(matrix.getSizeX() == 3);
    REQUIRE(matrix.getSizeY() == 3);
    REQUIRE(matrix.getTotalSize() == 9);

    SUBCASE("filling matrix with 0")
    {
        std::size_t count = 0;
        matrix.fill(0);

        for (int& n : matrix)
        {
            REQUIRE(n == 0);
            ++count;
        }

        REQUIRE(count == 9);
    }

    SUBCASE("setting matrix with values")
    {
        int value = 0;

        for (std::size_t y=0; y<matrix.getSizeY(); ++y)
        {
            for (std::size_t x=0; x<matrix.getSizeY(); ++x)
            {
                matrix[x][y] = value;
                REQUIRE(matrix.get(x,y) == value);
                ++value;
            }
        }

        SUBCASE("toVector")
        {
            std::vector<int> vector;
            matrix.toVector(vector);

            SUBCASE("sum up all values")
            {
                int sum = 0;

                for (int n : vector)
                {
                    sum += n;
                }

                REQUIRE(sum == 0+1+2+3+4+5+6+7+8);
            }
        }

        SUBCASE("sum up all values")
        {
            int sum = 0;

            for (int n : matrix)
            {
                sum += n;
            }

            REQUIRE(sum == 0+1+2+3+4+5+6+7+8);
        }

        SUBCASE("rotate clockwise")
        {
            matrix.rotateClockwise();

            REQUIRE(matrix.get(0,0) == 6);
            REQUIRE(matrix.get(2,0) == 0);
            REQUIRE(matrix.get(0,2) == 8);
            REQUIRE(matrix.get(2,2) == 2);

            REQUIRE(matrix.get(1,1) == 4);

            SUBCASE("rotate counter clockwise")
            {
                matrix.rotateCounterClockwise();

                REQUIRE(matrix.get(0,0) == 0);
                REQUIRE(matrix.get(2,0) == 2);
                REQUIRE(matrix.get(0,2) == 6);
                REQUIRE(matrix.get(2,2) == 8);

                REQUIRE(matrix.get(1,1) == 4);
            }
        }

        SUBCASE("flip horizontally")
        {
            matrix.flipHorizontally();

            REQUIRE(matrix.get(0,0) == 2);
            REQUIRE(matrix.get(2,0) == 0);
            REQUIRE(matrix.get(0,2) == 8);
            REQUIRE(matrix.get(2,2) == 6);

            REQUIRE(matrix.get(1,1) == 4);

            SUBCASE("flip vertically")
            {
                matrix.flipVertically();

                REQUIRE(matrix.get(0,0) == 8);
                REQUIRE(matrix.get(2,0) == 6);
                REQUIRE(matrix.get(0,2) == 2);
                REQUIRE(matrix.get(2,2) == 0);

                REQUIRE(matrix.get(1,1) == 4);
            }
        }
    }
}
