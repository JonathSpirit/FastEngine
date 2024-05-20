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

#include "doctest/doctest.h"
#include "FastEngine/extra/extra_string.hpp"
#include <utility>

TEST_CASE("testing IsValidUtf8String")
{
    //https://stackoverflow.com/questions/1301402/example-invalid-utf8-string
    const std::pair<std::string, bool> utf8sequences[] =
    {
        {"a", true}, // Valid ASCII
        {"\xc3\xb1", true}, // Valid 2 Octet Sequence
        {"\xc3\x28", false}, // Invalid 2 Octet Sequence
        {"\xa0\xa1", false}, // Invalid Sequence Identifier
        {"\xe2\x82\xa1", true}, // Valid 3 Octet Sequence
        {"\xe2\x28\xa1", false}, // Invalid 3 Octet Sequence (in 2nd Octet)
        {"\xe2\x82\x28", false}, // Invalid 3 Octet Sequence (in 3rd Octet)
        {"\xf0\x90\x8c\xbc", true}, // Valid 4 Octet Sequence
        {"\xf0\x28\x8c\xbc", false}, // Invalid 4 Octet Sequence (in 2nd Octet)
        {"\xf0\x90\x28\xbc", false}, // Invalid 4 Octet Sequence (in 3rd Octet)
        {"\xf0\x28\x8c\x28", false}, // Invalid 4 Octet Sequence (in 4th Octet)
        {"\xf8\xa1\xa1\xa1\xa1", false}, // Valid 5 Octet Sequence (but not Unicode!)
        {"\xfc\xa1\xa1\xa1\xa1\xa1", false} // Valid 6 Octet Sequence (but not Unicode!)
    };

    const std::size_t utf8sequencesSize = 13;

    for (std::size_t i=0; i<utf8sequencesSize; ++i)
    {
        bool result = fge::string::IsValidUtf8String(utf8sequences[i].first);
        REQUIRE(result == utf8sequences[i].second);
    }
}

TEST_CASE("String to number conversion tests") {
    SUBCASE("ToUint8 converts valid string to uint8_t correctly") {
        REQUIRE(fge::string::ToUint8("255") == 255);
    }
    SUBCASE("ToUint8 returns 0 for invalid string") {
        REQUIRE(fge::string::ToUint8("invalid") == 0);
    }

    SUBCASE("ToUint16 converts valid string to uint16_t correctly") {
        REQUIRE(fge::string::ToUint16("65535") == 65535);
    }
    SUBCASE("ToUint16 returns 0 for invalid string") {
        REQUIRE(fge::string::ToUint16("invalid") == 0);
    }

    SUBCASE("ToUint32 converts valid string to uint32_t correctly") {
        REQUIRE(fge::string::ToUint32("4294967295") == 4294967295);
    }
    SUBCASE("ToUint32 returns 0 for invalid string") {
        REQUIRE(fge::string::ToUint32("invalid") == 0);
    }

    SUBCASE("ToUint64 converts valid string to uint64_t correctly") {
        REQUIRE(fge::string::ToUint64("18446744073709551615") == 18446744073709551615ULL);
    }
    SUBCASE("ToUint64 returns 0 for invalid string") {
        REQUIRE(fge::string::ToUint64("invalid") == 0);
    }

    SUBCASE("ToInt8 converts valid string to int8_t correctly") {
        REQUIRE(fge::string::ToInt8("-128") == -128);
    }
    SUBCASE("ToInt8 returns 0 for invalid string") {
        REQUIRE(fge::string::ToInt8("invalid") == 0);
    }

    SUBCASE("ToInt16 converts valid string to int16_t correctly") {
        REQUIRE(fge::string::ToInt16("-32768") == -32768);
    }
    SUBCASE("ToInt16 returns 0 for invalid string") {
        REQUIRE(fge::string::ToInt16("invalid") == 0);
    }

    SUBCASE("ToInt32 converts valid string to int32_t correctly") {
        REQUIRE(fge::string::ToInt32("-2147483648") == -2147483648);
    }
    SUBCASE("ToInt32 returns 0 for invalid string") {
        REQUIRE(fge::string::ToInt32("invalid") == 0);
    }

    SUBCASE("ToInt64 converts valid string to int64_t correctly") {
        REQUIRE(fge::string::ToInt64("-9223372036854775808") == -9223372036854775808ULL);
    }
    SUBCASE("ToInt64 returns 0 for invalid string") {
        REQUIRE(fge::string::ToInt64("invalid") == 0);
    }

    SUBCASE("ToFloat converts valid string to float correctly") {
        REQUIRE(fge::string::ToFloat("3.14") == doctest::Approx(3.14));
    }
    SUBCASE("ToFloat returns 0 for invalid string") {
        REQUIRE(fge::string::ToFloat("invalid") == 0);
    }

    SUBCASE("ToDouble converts valid string to double correctly") {
        REQUIRE(fge::string::ToDouble("3.14") == doctest::Approx(3.14));
    }
    SUBCASE("ToDouble returns 0 for invalid string") {
        REQUIRE(fge::string::ToDouble("invalid") == 0);
    }

    SUBCASE("ToBool converts valid string to bool correctly") {
        REQUIRE(fge::string::ToBool("true") == true);
        REQUIRE(fge::string::ToBool("TrUe") == true);
        REQUIRE(fge::string::ToBool("TRUE") == true);
        REQUIRE(fge::string::ToBool("1") == true);
        REQUIRE(fge::string::ToBool("false") == false);
        REQUIRE(fge::string::ToBool("FaLsE") == false);
        REQUIRE(fge::string::ToBool("FALSE") == false);
        REQUIRE(fge::string::ToBool("0") == false);
    }
    SUBCASE("ToBool returns false for invalid string") {
        REQUIRE(fge::string::ToBool("invalid") == false);
    }

    SUBCASE("ToPtr converts valid string to pointer correctly") {
        if constexpr (sizeof(void*) == 8)
        {
            void* ptr = reinterpret_cast<void*>(0x4242424242424242);
            REQUIRE(fge::string::ToPtr("0x4242424242424242") == ptr);
        }
        else
        {
            void* ptr = reinterpret_cast<void*>(0x42424242);
            REQUIRE(fge::string::ToPtr("0x42424242") == ptr);
        }
    }

    SUBCASE("ToVec2f converts valid string to Vector2f correctly") {
        fge::Vector2f result = fge::string::ToVec2f("3.14 2.71");
        REQUIRE(result.x == doctest::Approx(3.14));
        REQUIRE(result.y == doctest::Approx(2.71));
    }
    SUBCASE("ToVec2f returns (0,0) for invalid string") {
        fge::Vector2f result = fge::string::ToVec2f("invalid");
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
    }
    SUBCASE("ToVec2f returns (0,0) for partially invalid string") {
        fge::Vector2f result = fge::string::ToVec2f("3.14 invalid");
        REQUIRE(result.x == doctest::Approx(3.14));
        REQUIRE(result.y == 0);
    }

    SUBCASE("ToVec2u converts valid string to Vector2u correctly") {
        fge::Vector2u result = fge::string::ToVec2u("42 24");
        REQUIRE(result.x == 42);
        REQUIRE(result.y == 24);
    }
    SUBCASE("ToVec2u returns (0,0) for invalid string") {
        fge::Vector2u result = fge::string::ToVec2u("invalid");
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
    }
    SUBCASE("ToVec2u returns (0,0) for partially invalid string") {
        fge::Vector2u result = fge::string::ToVec2u("42 invalid");
        REQUIRE(result.x == 42);
        REQUIRE(result.y == 0);
    }

    SUBCASE("ToVec2i converts valid string to Vector2i correctly") {
        fge::Vector2i result = fge::string::ToVec2i("-42 24");
        REQUIRE(result.x == -42);
        REQUIRE(result.y == 24);
    }
    SUBCASE("ToVec2i returns (0,0) for invalid string") {
        fge::Vector2i result = fge::string::ToVec2i("invalid");
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
    }
    SUBCASE("ToVec2i returns (0,0) for partially invalid string") {
        fge::Vector2i result = fge::string::ToVec2i("-42 invalid");
        REQUIRE(result.x == -42);
        REQUIRE(result.y == 0);
    }
}