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

#ifndef _FGE_C_AUDIO_HPP_INCLUDED
#define _FGE_C_AUDIO_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/manager/C_baseManager.hpp"
#include "FastEngine/manager/audio_manager.hpp"

namespace fge
{

/**
 * \class Audio
 * \ingroup audio
 * \brief This class is a wrapper for the audio manager to allow easy manipulation
 */
class FGE_API Audio : public manager::BaseDataAccessor<
                              manager::GlobalDataAccessorManagerInfo<audio::AudioManager, &audio::gManager>,
                              manager::DataAccessorOptions::ALLOW_VARIANT_OF_DATAPOINTER_AND_BLOCKPOINTER>
{
public:
    using BaseDataAccessor::BaseDataAccessor;
    using BaseDataAccessor::operator=;

    /**
     * \brief Play it using MIX_PlayAudio()
     *
     * Played using a default mixer and, like said in MIX_PlayAudio, using temporary track.
     *
     * \return \b True if the audio was successfully played, \b false otherwise
     */
    bool playAudio() const;
    /**
     * \brief Play the audio on a specific track
     *
     * \param trackID The ID of the track to play the audio on
     * \param loop The number of times to loop the audio (-1 for infinite loop)
     * \param gain The gain (volume) of the audio
     * \param create Whether to create the track if it doesn't exist
     * \return A pointer to the track if successful, nullptr otherwise
     */
    MIX_Track* playAudio(std::size_t trackID, int loop, float gain, bool create) const;
    /**
     * \brief Call MIX_SetTrackAudio on a specific track id
     *
     * \param trackID The ID of the track to set the audio on
     * \param create Whether to create the track if it doesn't exist
     * \return A pointer to the track if successful, nullptr otherwise
     */
    MIX_Track* setTrackAudio(std::size_t trackID, bool create) const;
};

} // namespace fge

#endif // _FGE_C_AUDIO_HPP_INCLUDED
