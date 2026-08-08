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

#include "FastEngine/accessor/C_audio.hpp"

namespace fge
{

bool Audio::playAudio() const
{
    return MIX_PlayAudio(this->retrieveManager().getDefaultMixer(),
                         const_cast<SharedType::element_type*>(this->retrieve()));
}
MIX_Track* Audio::playAudio(std::size_t trackID, int loop, float gain, bool create) const
{
    auto* track = this->retrieveManager().retrieveTrack(trackID, create);
    if (track == nullptr)
    {
        return nullptr;
    }

    if (!MIX_SetTrackAudio(track, const_cast<SharedType::element_type*>(this->retrieve())))
    {
        return nullptr;
    }

    MIX_SetTrackGain(track, gain);

    auto properties = SDL_CreateProperties();
    SDL_SetNumberProperty(properties, MIX_PROP_PLAY_LOOPS_NUMBER, loop);
    bool result = MIX_PlayTrack(track, properties);
    SDL_DestroyProperties(properties);
    return result ? track : nullptr;
}
MIX_Track* Audio::setTrackAudio(std::size_t trackID, bool create) const
{
    auto* track = this->retrieveManager().retrieveTrack(trackID, create);
    if (track == nullptr)
    {
        return nullptr;
    }
    bool result = MIX_SetTrackAudio(track, const_cast<SharedType::element_type*>(this->retrieve()));
    return result ? track : nullptr;
}

} // namespace fge
