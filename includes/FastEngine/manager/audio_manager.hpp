/*
 * Copyright 2022 Guillaume Guillet
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

#include "FastEngine/fastengine_extern.hpp"

#include "SFML/Audio/SoundBuffer.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#define FGE_AUDIO_DEFAULT FGE_AUDIO_BAD
#define FGE_AUDIO_BAD ""

namespace fge::audio
{

/**
 * \struct AudioData
 * \ingroup audio
 * \brief Structure containing the shared data of an SFML sound buffer
 */
struct AudioData
{
    std::shared_ptr<sf::SoundBuffer> _audio; ///< Shared pointer to the SFML sound buffer
    bool _valid;                             ///< Indicates if the audio data is valid
    std::string _path;                       ///< Path to the audio file
};

using AudioDataPtr = std::shared_ptr<fge::audio::AudioData>;
using AudioDataType = std::unordered_map<std::string, fge::audio::AudioDataPtr>;

/**
 * \ingroup audio
 * @{
 */

/**
 * \brief Initializes the audio manager
 *
 * This function must be called before any other function of the audio manager.
 * The audio manager is global thread safe storage for SFML sound buffers.
 */
FGE_API void Init();
/**
 * \brief Checks if the audio manager is initialized
 *
 * \return \b true if the audio manager is initialized, \b false otherwise
 */
FGE_API bool IsInit();
/**
 * \brief Un-initializes the audio manager
 */
FGE_API void Uninit();

/**
 * \brief Get the total number of audio data stored in the audio manager
 *
 * \return The number of audio data stored in the audio manager
 */
FGE_API std::size_t GetAudioSize();

/**
 * \brief Get the mutex of the audio manager
 *
 * \return The mutex of the audio manager
 */
FGE_API std::mutex& GetMutex();
/**
 * \brief Get the begin iterator of the audio manager
 *
 * \return The begin iterator of the audio manager
 */
FGE_API fge::audio::AudioDataType::const_iterator GetCBegin();
/**
 * \brief Get the end iterator of the audio manager
 *
 * \return The end iterator of the audio manager
 */
FGE_API fge::audio::AudioDataType::const_iterator GetCEnd();

/**
 * \brief Get the bad audio
 *
 * A bad audio is not valid and is default returned when an audio data is not found.
 *
 * \return The bad audio
 */
FGE_API const fge::audio::AudioDataPtr& GetBadAudio();
/**
 * \brief Get the audio data with the given name
 *
 * \param name The name of the audio data
 * \return The audio data with the given name or bad audio if not found
 */
FGE_API fge::audio::AudioDataPtr GetAudio(const std::string& name);

/**
 * \brief Check if the audio data with the given name exist
 *
 * \param name The name of the audio data
 * \return \b true if the audio data with the given name exist, \b false otherwise
 */
FGE_API bool Check(const std::string& name);

/**
 * \brief Load the audio data with the given name from the given file path
 *
 * \param name The name of the audio data
 * \param path The path to the audio file
 * \return \b true if the audio data was loaded, \b false otherwise
 */
FGE_API bool LoadFromFile(const std::string& name, const std::string& path);
/**
 * \brief Unload the audio data with the given name
 *
 * \param name The name of the audio data
 * \return \b true if the audio data was unloaded, \b false otherwise
 */
FGE_API bool Unload(const std::string& name);
/**
 * \brief Unload all the audio data
 */
FGE_API void UnloadAll();

/**
 * \brief Push a user handled audio data to the audio manager
 *
 * \param name The name of the audio data to push
 * \param data The audio data to push
 * \return \b true if the audio is pushed, \b false otherwise
 */
FGE_API bool Push(const std::string& name, const fge::audio::AudioDataPtr& data);

/**
 * @}
 */

} // namespace fge::audio

#endif // _FGE_AUDIO_MANAGER_HPP_INCLUDED
