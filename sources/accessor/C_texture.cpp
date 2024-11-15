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

#include "FastEngine/accessor/C_texture.hpp"

namespace fge
{

fge::Vector2u Texture::getTextureSize() const
{
    return this->retrieve()->getSize();
}

Texture::SharedType::element_type* Texture::retrieveGroup(std::size_t index)
{
    auto const& data = this->getSharedBlock();

    if (!data->_valid)
    {
        return nullptr;
    }

    return index < data->_group.size() ? data->_group[index].get() : nullptr;
}
Texture::SharedType::element_type const* Texture::retrieveGroup(std::size_t index) const
{
    auto const& data = this->getSharedBlock();

    if (!data->_valid)
    {
        return nullptr;
    }
    return index < data->_group.size() ? data->_group[index].get() : nullptr;
}
std::size_t Texture::groupSize() const
{
    auto const& data = this->getSharedBlock();

    if (!data->_valid)
    {
        return 0;
    }
    return data->_group.size();
}

} // namespace fge
