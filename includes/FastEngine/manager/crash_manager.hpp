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

#ifndef _FGE_CRASH_MANAGER_HPP_INCLUDED
#define _FGE_CRASH_MANAGER_HPP_INCLUDED
#ifndef FGE_DEF_SERVER

    #include "FastEngine/fastengine_extern.hpp"
    #include <string>

namespace fge::crash
{

FGE_API void Init(sf::RenderWindow& screen, sf::Font& font);

FGE_API void SetInfoText(const std::string& txt);

} // namespace fge::crash

#endif //FGE_DEF_SERVER
#endif // _FGE_CRASH_MANAGER_HPP_INCLUDED
