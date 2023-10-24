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

#include "FastEngine/network/C_sceneUpdateCache.hpp"
#include "FastEngine/network/C_server.hpp"

namespace fge
{

//SceneUpdateCache

void SceneUpdateCache::clear()
{
    this->g_forceRetrievable = false;
    decltype(this->g_cache)().swap(this->g_cache);
}

void SceneUpdateCache::push(UpdateCountRange updateCountRange, fge::net::FluxPacketPtr fluxPacket)
{
    this->g_cache.push(SceneUpdateCache::Data{updateCountRange, std::move(fluxPacket)});
    if (this->g_cache.size() >= FGE_SCENE_UPDATECACHE_LIMIT)
    {
        //We force the cache to be retrievable
        //We can consider that some packets are lost if the cache is full
        this->g_forceRetrievable = true;
    }
}
bool SceneUpdateCache::isRetrievable(uint16_t sceneActualUpdateCount) const
{
    if (this->g_cache.empty())
    {
        this->g_forceRetrievable = false;
        return false;
    }

    if (this->g_forceRetrievable)
    {
        return true;
    }

    if (this->g_cache.top()._updateCountRange._last == sceneActualUpdateCount)
    {
        return true;
    }

    return false;
}
SceneUpdateCache::Data SceneUpdateCache::pop()
{
    auto data = std::move(const_cast<SceneUpdateCache::Data&>(this->g_cache.top()));
    this->g_cache.pop();
    return data;
}

bool SceneUpdateCache::isForced() const
{
    return this->g_forceRetrievable;
}

} // namespace fge
