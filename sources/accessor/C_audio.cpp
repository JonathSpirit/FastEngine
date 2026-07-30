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

#include "FastEngine/accessor/C_soundBuffer.hpp"

namespace fge
{

bool SoundBuffer::playAudio() const
{
    return MIX_PlayAudio(this->retrieveManager().getDefaultMixer(),
                         const_cast<SharedType::element_type*>(this->retrieve()));
}
bool SoundBuffer::setTrackAudio(std::size_t trackID, bool create) const
{
    auto* track = this->retrieveManager().retrieveTrack(trackID, create);
    if (track == nullptr)
    {
        return false;
    }
    return MIX_SetTrackAudio(track, const_cast<SharedType::element_type*>(this->retrieve()));
}
bool SoundBuffer::setTrackAudio(MIX_Track* track) const
{
    if (track == nullptr)
    {
        return false;
    }
    return MIX_SetTrackAudio(track, const_cast<SharedType::element_type*>(this->retrieve()));
}
bool SoundBuffer::setTrackGain(std::size_t trackID, float gain) const
{
    auto* track = this->retrieveManager().retrieveTrack(trackID, true);
    if (track == nullptr)
    {
        return false;
    }
    return MIX_SetTrackGain(track, gain);
}

} // namespace fge
