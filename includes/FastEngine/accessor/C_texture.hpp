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

#ifndef _FGE_C_TEXTURE_HPP_INCLUDED
#define _FGE_C_TEXTURE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_vector.hpp"
#include "FastEngine/manager/C_baseManager.hpp"
#include "FastEngine/manager/texture_manager.hpp"

namespace fge
{

/**
 * \class Texture
 * \ingroup graphics
 * \brief This class is a wrapper for the texture manager to allow easy manipulation
 */
class FGE_API Texture : public manager::BaseDataAccessor<
                                manager::GlobalDataAccessorManagerInfo<texture::TextureManager, &texture::gManager>,
                                manager::DataAccessorOptions::ALLOW_VARIANT_OF_DATAPOINTER_AND_BLOCKPOINTER>
{
public:
    using BaseDataAccessor::BaseDataAccessor;
    using BaseDataAccessor::operator=;

    /**
     * \brief Get the texture size
     *
     * \return The texture size
     */
    [[nodiscard]] fge::Vector2u getTextureSize() const;
    [[nodiscard]] fge::RectInt getTextureRect() const;

    /**
     * \brief Retrieve a sub-texture inside this texture group
     *
     * \param index The index of the sub-texture
     * \return The sub-texture type pointer or \b nullptr if the index is out of range
     */
    [[nodiscard]] SharedType::element_type* retrieveGroup(std::size_t index);
    [[nodiscard]] SharedType::element_type const* retrieveGroup(std::size_t index) const;
    [[nodiscard]] std::size_t groupSize() const;
};

} // namespace fge

#endif // _FGE_C_TEXTURE_HPP_INCLUDED
