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
