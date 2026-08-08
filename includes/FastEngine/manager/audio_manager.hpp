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

#ifndef _FGE_AUDIO_MANAGER_HPP_INCLUDED
#define _FGE_AUDIO_MANAGER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"

#include "FastEngine/manager/C_baseManager.hpp"
#include "SDL3_mixer/SDL_mixer.h"
#include <limits>

#define FGE_AUDIO_BAD FGE_MANAGER_BAD
#define FGE_AUDIO_AVAILABLE_TRACK std::numeric_limits<std::size_t>::max()

namespace fge::audio
{

struct MixerChunkDeleter
{
    inline void operator()(MIX_Audio* audio) const { MIX_DestroyAudio(audio); }
};

struct DataBlock final : manager::BaseDataBlock<MIX_Audio>
{
    [[nodiscard]] inline bool duplicate([[maybe_unused]] BaseDataBlock& block) const override
    {
        return false; //TODO: query audio data, and copy it
    }
};

/**
 * \class AudioManager
 * \ingroup audio
 * \brief Manage audio data
 *
 * \see TextureManager
 */
class FGE_API AudioManager final : public manager::BaseManager<MIX_Audio, DataBlock>
{
public:
    using BaseManager::BaseManager;

    bool initialize() override;
    void uninitialize() override;

    /**
     * \brief Load the audio data with the given name from the given file path
     *
     * \param name The name of the texture to load
     * \param path The path of the file to load
     * \return \b true if the texture was loaded, \b false otherwise
     */
    bool loadFromFile(std::string_view name, std::filesystem::path const& path);

    [[nodiscard]] inline MIX_Mixer* getDefaultMixer() const { return this->g_defaultMixer; }

    /**
     * \brief Retrieve a manager handled track
     *
     * Tracks is generated from the default mixer. By default, if an id doesn't exist, it will be
     * created automatically.
     *
     * You can pass FGE_AUDIO_AVAILABLE_TRACK to retrieve a track that is not currently playing, or nullptr if none is available.
     *
     * \param id The ID of the track to retrieve
     * \param create Whether to create the track if it doesn't exist
     * \return A pointer to the retrieved track, or nullptr if not available
     */
    [[nodiscard]] MIX_Track* retrieveTrack(std::size_t id, bool create = true);
    void destroyTrack(std::size_t id);
    void destroyTracks();
    bool setTrackGain(std::size_t id, float gain) const;
    /**
     * \brief Generate a range of tracks ID
     *
     * No verification is made on the created tracks if there are valid or not.
     *
     * \param start The starting range value
     * \param end The ending range value
     * \return \b true if creating range as been applied, \b false if arguments error
     */
    bool generateTracks(std::size_t start, std::size_t end);

private:
    MIX_Mixer* g_defaultMixer{nullptr};
    std::unordered_map<std::size_t, MIX_Track*> g_tracks;
};

/**
 * \ingroup managers
 * \brief The global audio manager
 */
FGE_API extern AudioManager gManager;

} // namespace fge::audio

#endif // _FGE_AUDIO_MANAGER_HPP_INCLUDED
