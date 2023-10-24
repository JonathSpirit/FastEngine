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

#ifndef _FGE_C_SCENEUPDATECACHE_HPP_INCLUDED
#define _FGE_C_SCENEUPDATECACHE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"

#include <memory>
#include <queue>

#define FGE_SCENE_UPDATECACHE_LIMIT 10

namespace fge
{

namespace net
{

class FluxPacket;
using FluxPacketPtr = std::unique_ptr<fge::net::FluxPacket>;

} // namespace net

class Scene;

struct UpdateCountRange
{
    uint16_t _last;
    uint16_t _now;
};

/**
 * \class SceneUpdateCache
 * \ingroup network
 * \brief A cache for scene updates
 *
 * This class is used to cache scene updates and retrieve them later.
 */
class FGE_API SceneUpdateCache
{
public:
    struct Data
    {
        UpdateCountRange _updateCountRange;
        fge::net::FluxPacketPtr _fluxPacket;

        inline bool operator>(Data const& r) const { return this->_updateCountRange._last > r._updateCountRange._last; }
    };

    SceneUpdateCache() = default;
    SceneUpdateCache(SceneUpdateCache const& r) = delete;
    SceneUpdateCache(SceneUpdateCache&& r) noexcept = default;
    ~SceneUpdateCache() = default;

    SceneUpdateCache& operator=(SceneUpdateCache const& r) = delete;
    SceneUpdateCache& operator=(SceneUpdateCache&& r) noexcept = default;

    void clear();

    void push(UpdateCountRange updateCountRange, fge::net::FluxPacketPtr fluxPacket);
    [[nodiscard]] bool isRetrievable(uint16_t sceneActualUpdateCount) const;
    [[nodiscard]] Data pop();

    [[nodiscard]] bool isForced() const;

private:
    std::priority_queue<Data, std::vector<Data>, std::greater<>> g_cache;
    mutable bool g_forceRetrievable{false};
};

} // namespace fge

#endif // _FGE_C_SCENEUPDATECACHE_HPP_INCLUDED