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
 *
 * This file must stay in its own test executable: defining TINY_UTF8_NOEXCEPT changes
 * the noexcept specifications of tiny_utf8, mixing it with other translation units would be an ODR violation.
 */

#include "doctest/doctest.h"
#include <cstddef>
#include <new>
#include <type_traits>

#define TINY_UTF8_NOEXCEPT true
#include "tinyutf8.h"

namespace
{

template<typename T = char>
class ThrowAllocator
{
public:
    using value_type = T;

    template<class Other>
    struct rebind
    {
        using other = ThrowAllocator<Other>;
    };

    ThrowAllocator() {}
    ThrowAllocator(ThrowAllocator const&) {}
    ThrowAllocator(ThrowAllocator&&) {}
    template<typename U>
    ThrowAllocator([[maybe_unused]] ThrowAllocator<U> const& r)
    {}
    template<typename U>
    ThrowAllocator([[maybe_unused]] ThrowAllocator<U>&& r)
    {}

    T* allocate([[maybe_unused]] std::size_t n) { throw std::bad_alloc(); }
    void deallocate([[maybe_unused]] T* p, [[maybe_unused]] std::size_t n) {}
};

template<typename T, typename U>
constexpr bool operator==(ThrowAllocator<T> const&, ThrowAllocator<U> const&)
{
    return true;
}
template<typename T, typename U>
constexpr bool operator!=(ThrowAllocator<T> const&, ThrowAllocator<U> const&)
{
    return false;
}

template<typename T = char>
class NoThrowAllocator
{
public:
    using value_type = T;

    template<class Other>
    struct rebind
    {
        using other = NoThrowAllocator<Other>;
    };

    NoThrowAllocator() noexcept {}
    NoThrowAllocator(NoThrowAllocator const&) noexcept {}
    NoThrowAllocator(NoThrowAllocator&&) noexcept {}
    template<typename U>
    NoThrowAllocator([[maybe_unused]] NoThrowAllocator<U> const& r) noexcept
    {}
    template<typename U>
    NoThrowAllocator([[maybe_unused]] NoThrowAllocator<U>&& r) noexcept
    {}

    T* allocate([[maybe_unused]] std::size_t n) noexcept { return nullptr; }
    void deallocate([[maybe_unused]] T* p, [[maybe_unused]] std::size_t n) noexcept {}
};

template<typename T, typename U>
constexpr bool operator==(NoThrowAllocator<T> const&, NoThrowAllocator<U> const&) noexcept
{
    return true;
}
template<typename T, typename U>
constexpr bool operator!=(NoThrowAllocator<T> const&, NoThrowAllocator<U> const&) noexcept
{
    return false;
}

} // namespace

TEST_CASE("testing tiny_utf8 noexcept with a throwing allocator")
{
    // Make sure that the mock allocator can throw an exception
    REQUIRE_FALSE(std::is_nothrow_default_constructible_v<ThrowAllocator<char>>);
    REQUIRE_FALSE(std::is_nothrow_copy_constructible_v<ThrowAllocator<char>>);
    REQUIRE_FALSE(std::is_nothrow_move_constructible_v<ThrowAllocator<char>>);

    using alloc_utf8_string = tiny_utf8::basic_string<char32_t, char, ThrowAllocator<char>>;

    char const TEST_LITERAL[1] = {'R'};

    // Default and move constructors forward the allocator noexcept specification
    CHECK_FALSE(std::is_nothrow_default_constructible_v<alloc_utf8_string>);
    CHECK_FALSE(std::is_nothrow_move_constructible_v<alloc_utf8_string>);
    // The copy constructor is only driven by TINY_UTF8_NOEXCEPT
    CHECK(std::is_nothrow_copy_constructible_v<alloc_utf8_string>);

    // Small literals use SSO and never call the allocator
    alloc_utf8_string str(TEST_LITERAL, ThrowAllocator<char>());
    CHECK(str.sso_active());
    CHECK(str.length() == 1);
    CHECK(static_cast<uint64_t>(str[0]) == static_cast<uint64_t>(U'R'));
}

TEST_CASE("testing tiny_utf8 noexcept with a non-throwing allocator")
{
    // Make sure that the mock allocator will NOT throw an exception
    REQUIRE(std::is_nothrow_default_constructible_v<NoThrowAllocator<char>>);
    REQUIRE(std::is_nothrow_copy_constructible_v<NoThrowAllocator<char>>);
    REQUIRE(std::is_nothrow_move_constructible_v<NoThrowAllocator<char>>);

    using alloc_utf8_string = tiny_utf8::basic_string<char32_t, char, NoThrowAllocator<char>>;

    char const TEST_LITERAL[1] = {'R'};

    CHECK(std::is_nothrow_default_constructible_v<alloc_utf8_string>);
    CHECK(std::is_nothrow_copy_constructible_v<alloc_utf8_string>);
    CHECK(std::is_nothrow_move_constructible_v<alloc_utf8_string>);

    // Small literals use SSO and never call the allocator
    alloc_utf8_string str(TEST_LITERAL, NoThrowAllocator<char>());
    CHECK(str.sso_active());
    CHECK(str.length() == 1);
    CHECK(static_cast<uint64_t>(str[0]) == static_cast<uint64_t>(U'R'));
}
