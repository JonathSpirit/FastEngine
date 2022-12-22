/*
 * Copyright 2022 Guillaume Guillet
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

#include <string>

namespace fge::priv
{

struct string_hash
{
    using is_transparent = void;

    [[nodiscard]] std::size_t operator()(const char* str) const { return std::hash<std::string_view>{}(str); }

    [[nodiscard]] std::size_t operator()(std::string_view str) const { return std::hash<std::string_view>{}(str); }

    [[nodiscard]] std::size_t operator()(const std::string& str) const { return std::hash<std::string>{}(str); }
};

} // namespace fge::priv