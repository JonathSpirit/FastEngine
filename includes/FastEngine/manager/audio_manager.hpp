/*
 * Copyright 2025 Guillaume Guillet
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
#include "SDL_mixer.h"

#define FGE_AUDIO_BAD FGE_MANAGER_BAD

namespace fge::audio
{

struct MixerChunkDeleter
{
    inline void operator()(Mix_Chunk* chunk) const { Mix_FreeChunk(chunk); }
};

struct DataBlock final : manager::BaseDataBlock<Mix_Chunk>
{};

/**
 * \class AudioManager
 * \ingroup audio
 * \brief Manage audio data
 *
 * \see TextureManager
 */
class FGE_API AudioManager final : public manager::BaseManager<Mix_Chunk, DataBlock>
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
};

/**
 * \ingroup managers
 * \brief The global audio manager
 */
FGE_API extern AudioManager gManager;

} // namespace fge::audio

#endif // _FGE_AUDIO_MANAGER_HPP_INCLUDED
