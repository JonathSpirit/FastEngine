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

#ifndef _FGE_GRAPHIC_C_GLYPH_HPP_INCLUDED
#define _FGE_GRAPHIC_C_GLYPH_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/C_rect.hpp"

namespace fge
{

class Glyph
{
public:
    Glyph() :
            _advance(0.0f)
    {}

    float _advance = 0.0f;     //!< Offset to move horizontally to the next character
    int _lsbDelta = 0;         //!< Left offset after forced autohint. Internally used by getKerning()
    int _rsbDelta = 0;         //!< Right offset after forced autohint. Internally used by getKerning()
    fge::RectFloat _bounds;    //!< Bounding rectangle of the glyph, in coordinates relative to the baseline
    fge::RectInt _textureRect; //!< Texture coordinates of the glyph inside the font's texture
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_GLYPH_HPP_INCLUDED
