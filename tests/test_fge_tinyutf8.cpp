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

/*
 * Ported from the tiny-utf8 unit tests (https://github.com/DuffsDevice/tiny-utf8)
 * Copyright (c) 2015-2021 Jakob Riedle (DuffsDevice), BSD-3-Clause
 */

#include "doctest/doctest.h"
#include "tinyutf8.h"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>

namespace
{

template<class TString>
class SSOCapacity : public TString
{
public:
    static constexpr typename TString::size_type get() noexcept { return TString::get_sso_capacity(); }
};

bool CodepointEqual(tiny_utf8::string::value_type const& a, tiny_utf8::string::value_type const& b)
{
    return static_cast<uint64_t>(a) == static_cast<uint64_t>(b);
}

} // namespace

// Construction

TEST_CASE("testing tiny_utf8 constructor taking a literal")
{
    tiny_utf8::string str(U"TEST: ツ♫");

    CHECK(str.length() == 8);
    CHECK(str.size() == 12);
    CHECK(str.requires_unicode());
    CHECK(str.sso_active());
    CHECK_FALSE(str.lut_active());
    CHECK(static_cast<uint64_t>(str[6]) == 12484);
}

TEST_CASE("testing tiny_utf8 constructor taking a literal with and without SSO")
{
    using test_utf8_string = tiny_utf8::basic_string<char32_t, char, std::allocator<char>>;

    constexpr uint32_t TEST_LITERAL_U_LENGTH = 1;
    constexpr uint32_t TEST_STRING_LENGTH = 100;

    char const TEST_LITERAL_T = 'T';
    char const TEST_LITERAL_U[TEST_LITERAL_U_LENGTH] = {'U'};
    char const TEST_STRING[TEST_STRING_LENGTH] =
            "This is a test string...This is a test string...This is a test string...This is a test string...";

    // Constructor that fills the string with the supplied character
    test_utf8_string str_literal(TEST_LITERAL_T, std::allocator<char>());
    CHECK(str_literal.length() == 1);
    CHECK(str_literal.size() == 1);
    CHECK(static_cast<uint64_t>(str_literal[0]) == static_cast<uint64_t>(U'T'));

    // Constructor taking an utf8 char literal (SSO)
    REQUIRE(TEST_LITERAL_U_LENGTH < SSOCapacity<test_utf8_string>::get());
    test_utf8_string str_literal_sso(TEST_LITERAL_U, std::allocator<char>());
    CHECK(str_literal_sso.length() == TEST_LITERAL_U_LENGTH);
    CHECK(str_literal_sso.size() == TEST_LITERAL_U_LENGTH);
    CHECK(str_literal_sso.sso_active());

    // Constructor taking an utf8 char literal (NO SSO)
    REQUIRE(TEST_STRING_LENGTH > SSOCapacity<test_utf8_string>::get());
    test_utf8_string str_literal_nosso(TEST_STRING, std::allocator<char>());
    CHECK(str_literal_nosso.length() == TEST_STRING_LENGTH - 1); // -1 accounts for null terminator
    CHECK(str_literal_nosso.size() == TEST_STRING_LENGTH - 1);
    CHECK_FALSE(str_literal_nosso.sso_active());
}

TEST_CASE("testing tiny_utf8 constructor taking a literal with max code points")
{
    tiny_utf8::string str(U"ツ♫", 1);

    CHECK(str.length() == 1);
    CHECK(str.size() == 3);
    CHECK(str.requires_unicode());
    CHECK(str.sso_active());
    CHECK_FALSE(str.lut_active());
    CHECK(static_cast<uint64_t>(str[0]) == 12484);
}

TEST_CASE("testing tiny_utf8 constructor taking an ansi string")
{
    std::string const ansi_str("Loewen, Boeren, Voegel und Koefer sind Tiere.");
    tiny_utf8::string str(ansi_str);

    CHECK(ansi_str.length() == 45);
    CHECK(ansi_str.size() == 45);
    CHECK(str.length() == 45);
    CHECK(str.size() == 45);

    CHECK_FALSE(str.requires_unicode());
    CHECK_FALSE(str.sso_active());
    CHECK(str.lut_active());
}

TEST_CASE("testing tiny_utf8 constructor taking a string view")
{
    std::string const str_orig("Löwen, Bären, Vögel und Käfer sind Tiere.");
    std::string_view const str_view(str_orig);
    tiny_utf8::string str(str_view);

    CHECK(str_view.length() == 45);
    CHECK(str_view.size() == 45);
    CHECK(str.length() == 41);
    CHECK(str.size() == 45);

    CHECK(str.requires_unicode());
    CHECK_FALSE(str.sso_active());
    CHECK(str.lut_active());
}

TEST_CASE("testing tiny_utf8 copy constructor")
{
    tiny_utf8::string str_orig(U"Hello  ツ  World");
    tiny_utf8::string str(str_orig);

    CHECK(str == str_orig);
    CHECK(str.length() == 15);
    CHECK(str.size() == 17);
    CHECK(str.requires_unicode());
    CHECK(str.sso_active());
    CHECK_FALSE(str.lut_active());
    CHECK(static_cast<uint64_t>(str[8]) == 32);
}

// Conversion

TEST_CASE("testing tiny_utf8 to wide literal")
{
    tiny_utf8::string str(std::string("Löwen, Bären, Vögel und Käfer sind Tiere."));

    std::unique_ptr<char32_t[]> ptr{new char32_t[str.length() + 1]};
    str.to_wide_literal(ptr.get());

    for (std::size_t chcount = 0; chcount < str.length(); ++chcount)
    {
        CHECK(static_cast<uint64_t>(str[chcount]) == static_cast<uint64_t>(ptr[chcount]));
    }
    CHECK(ptr[str.length()] == U'\0');
}

TEST_CASE("testing tiny_utf8 to cpp string")
{
    std::string const str_orig("Löwen, Bären, Vögel und Käfer sind Tiere.");
    tiny_utf8::string str(str_orig);

    CHECK(str.cpp_str() == str_orig);
    CHECK(str.cpp_str_view() == str_orig);
}

// Iterators

TEST_CASE("testing tiny_utf8 iterator access")
{
    tiny_utf8::string const str = U"Hello ツ World ♫";

    CHECK(str.requires_unicode());
    CHECK(str.sso_active());

    tiny_utf8::string str_fwd;
    tiny_utf8::string str_rev;

    std::copy(str.begin(), str.end(), std::back_inserter(str_fwd));   // Copy using forward iterators
    std::copy(str.rbegin(), str.rend(), std::back_inserter(str_rev)); // Copy using reverse iterators

    REQUIRE(str_fwd.length() == str.length());
    REQUIRE(str_rev.length() == str.length());

    CHECK(std::equal(str.begin(), str.end(), str_fwd.begin(), CodepointEqual));

    tiny_utf8::string::const_iterator it_src = str.end();
    tiny_utf8::string::const_iterator it_rev = str_rev.begin();
    while (it_src != str.begin())
    {
        --it_src;
        CHECK(static_cast<uint64_t>(*it_src) == static_cast<uint64_t>(*it_rev));
        ++it_rev;
    }

    tiny_utf8::string::const_iterator it_fwd = str.begin();
    for (std::size_t chcount = 0; chcount < str.length(); ++chcount)
    {
        CHECK(static_cast<uint64_t>(str[chcount]) == static_cast<uint64_t>(*it_fwd));
        ++it_fwd;
    }
}

// Manipulation

TEST_CASE("testing tiny_utf8 append string")
{
    tiny_utf8::string str = U"Hello ツ";
    str.append(U" ♫ World");

    CHECK(str.length() == 15);
    CHECK(str.size() == 19);
    CHECK(str.requires_unicode());
    CHECK(str.sso_active());
    CHECK_FALSE(str.lut_active());

    tiny_utf8::string tmp = U" ♫ World";
    str.append(tmp);

    CHECK(str.length() == 23);
    CHECK(str.size() == 29);
    CHECK(str.requires_unicode());
    CHECK(str.sso_active());
    CHECK_FALSE(str.lut_active());
}

TEST_CASE("testing tiny_utf8 append and shrink string")
{
    tiny_utf8::string str(U"TEST: ツ♫");

    CHECK(str.capacity() == 31);

    for (uint64_t iteration = 0; iteration < 5; ++iteration)
    {
        CAPTURE(iteration);
        str.append(str); // self-append

        switch (iteration)
        {
        case 0:
            CHECK(str.capacity() == 31);
            CHECK(str.length() == 16);
            CHECK(str.size() == 24);
            CHECK_FALSE(str.lut_active());
            break;
        case 1:
            CHECK(str.capacity() == 72);
            CHECK(str.length() == 32);
            CHECK(str.size() == 48);
            CHECK(str.lut_active());
            break;
        case 2:
            CHECK(str.capacity() == 72);
            CHECK(str.length() == 64);
            CHECK(str.size() == 96);
            CHECK(str.lut_active());
            break;
        case 3:
            CHECK(str.capacity() == 231);
            CHECK(str.length() == 128);
            CHECK(str.size() == 192);
            CHECK(str.lut_active());
            break;
        case 4:
            CHECK(str.capacity() == 519);
            CHECK(str.length() == 256);
            CHECK(str.size() == 384);
            CHECK(str.lut_active());
            break;
        default:
            break;
        }
    }

    str.shrink_to_fit();

    CHECK(str.capacity() == 259);
    CHECK(str.length() == 256);
    CHECK(str.size() == 384);
    CHECK(str.lut_active());
}

TEST_CASE("testing tiny_utf8 erase string")
{
    tiny_utf8::string str = U"Hello ツ World ♫";

    CHECK(str.length() == 15);
    CHECK(str.size() == 19);
    CHECK(str.requires_unicode());
    CHECK(str.sso_active());
    CHECK_FALSE(str.lut_active());

    // Erasing by index returns a reference to the string itself
    auto& res1 = str.erase(14);

    CHECK(&res1 == &str);
    CHECK(str == U"Hello ツ World ");
    CHECK(str.length() == 14);
    CHECK(str.size() == 16);
    CHECK(str.requires_unicode());
    CHECK(str.sso_active());

    auto res2 = str.erase(str.begin(), str.begin() + 9);

    CHECK(res2 == str.begin());
    CHECK(str.length() == 5);
    CHECK(str.size() == 5);
    CHECK_FALSE(str.requires_unicode());
    CHECK(str.sso_active());

    // Erasing a single codepoint returns a raw_iterator, compare it with the same iterator type
    // (raw_iterator and iterator are implicitly convertible to each other, a mixed comparison is ambiguous)
    tiny_utf8::string::raw_iterator res3 = str.erase(str.begin() + 2);

    CHECK(res3 == tiny_utf8::string::raw_iterator(str.begin() + 2));
    CHECK(str.length() == 4);
    CHECK(str.size() == 4);
    CHECK_FALSE(str.requires_unicode());
    CHECK(str.sso_active());
}

TEST_CASE("testing tiny_utf8 sub string")
{
    tiny_utf8::string const fullstr(U"Hello ツ World rg rth rt he rh we gxgre");

    tiny_utf8::string const str = fullstr.substr(3, 16);
    tiny_utf8::string const substr(U"lo ツ World rg rt");

    REQUIRE(str.length() == substr.length());
    CHECK(std::equal(substr.begin(), substr.end(), str.begin(), CodepointEqual));

    CHECK(str.sso_active());
    CHECK_FALSE(str.lut_active());

    CHECK(static_cast<uint64_t>(str[6]) == static_cast<uint64_t>(U'o'));
}

TEST_CASE("testing tiny_utf8 replace string")
{
    tiny_utf8::string str(U"Hello ツ World");

    std::string const str_repl1("Hello World");
    tiny_utf8::string const str_repl2(U"Hello~ 🤝 ~World");

    char32_t const ch_repl1 = U'ツ';
    char32_t const ch_repl2 = U'🤝';

    CHECK(static_cast<uint64_t>(str[6]) == static_cast<uint64_t>(ch_repl1));
    CHECK(str.length() == 13);
    CHECK(str.requires_unicode());
    CHECK(str.sso_active());
    CHECK_FALSE(str.lut_active());

    str[6] = ch_repl2;
    CHECK(static_cast<uint64_t>(str[6]) == static_cast<uint64_t>(ch_repl2));
    CHECK(str.length() == 13);
    CHECK(str.requires_unicode());

    str.replace(5, 3, " ");
    REQUIRE(str.length() == str_repl1.length());
    CHECK(std::equal(str.begin(), str.end(), str_repl1.begin(),
                     [](tiny_utf8::string::value_type const& a, char const& b) {
                         return static_cast<uint64_t>(a) == static_cast<uint64_t>(b);
                     }));
    CHECK(str.length() == 11);
    CHECK_FALSE(str.requires_unicode());

    str.replace(str.begin() + 5, str.begin() + 6, U"~ 🤝 ~");
    REQUIRE(str.length() == str_repl2.length());
    CHECK(std::equal(str.begin(), str.end(), str_repl2.begin(), CodepointEqual));
    CHECK(str.length() == 15);
    CHECK(str.requires_unicode());
}

// Search

TEST_CASE("testing tiny_utf8 find sub string")
{
    tiny_utf8::string str = U"Hello World ツ♫";

    char32_t const* find_last_not_of = U"ツ♫";
    char32_t const* find_last_of = U"e";
    char32_t const find = U'l';
    char32_t const rfind = U'l';

    CHECK(str.find_last_not_of(find_last_not_of) == 11);
    CHECK(str.find_last_of(find_last_of) == 1);
    CHECK(str.find(find) == 2);
    CHECK(str.rfind(rfind) == 9);
}

TEST_CASE("testing tiny_utf8 starts/ends with")
{
    tiny_utf8::string str = U"Hello World ツ♫";

    char32_t const* ends_with_positive = U"ツ♫";
    char32_t const* ends_with_negative = U"e";
    char32_t const* starts_with_positive = U"Hello ";
    char32_t const* starts_with_negative = U"Hell ";

    CHECK(str.ends_with(ends_with_positive));
    CHECK_FALSE(str.ends_with(ends_with_negative));
    CHECK(str.ends_with(tiny_utf8::string(ends_with_positive)));
    CHECK_FALSE(str.ends_with(tiny_utf8::string(ends_with_negative)));
    CHECK(str.starts_with(starts_with_positive));
    CHECK_FALSE(str.starts_with(starts_with_negative));
    CHECK(str.starts_with(tiny_utf8::string(starts_with_positive)));
    CHECK_FALSE(str.starts_with(tiny_utf8::string(starts_with_negative)));
}
