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

#include "FastEngine/manager/audio_manager.hpp"

namespace fge::audio
{

bool AudioManager::initialize()
{
    if (this->isInitialized())
    {
        return true;
    }

    if (!MIX_Init())
    {
        return false;
    }

    this->g_defaultMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (this->g_defaultMixer == nullptr)
    {
        return false;
    }

    //Taken from https://cable.ayra.ch/empty/
    uint8_t emptyWaveFile[] = {0x52, 0x49, 0x46, 0x46, 0x25, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45,
                               0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
                               0x44, 0xAC, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00, 0x02, 0x00, 0x10, 0x00,
                               0x64, 0x61, 0x74, 0x61, 0x74, 0x00, 0x00, 0x00, 0x00};

    this->_g_badElement = std::make_shared<DataBlockType>();
    this->_g_badElement->_ptr = std::shared_ptr<MIX_Audio>(
            MIX_LoadRawAudio(this->g_defaultMixer, emptyWaveFile, std::size(emptyWaveFile), nullptr),
            MixerChunkDeleter());
    this->_g_badElement->_valid = false;
    return true;
}
void AudioManager::uninitialize()
{
    BaseManager::uninitialize();
    MIX_DestroyMixer(this->g_defaultMixer);
    MIX_Quit();
}

bool AudioManager::loadFromFile(std::string_view name, std::filesystem::path const& path)
{
    if (name.empty())
    {
        return false;
    }

    MIX_Audio* tmpAudio = MIX_LoadAudio(this->g_defaultMixer, path.string().c_str(), true);

    if (tmpAudio == nullptr)
    {
        return false;
    }

    DataBlockPointer block = std::make_shared<DataBlockType>();
    block->_ptr = std::shared_ptr<MIX_Audio>(tmpAudio, MixerChunkDeleter());
    block->_valid = true;
    block->_path = path;

    return this->push(name, std::move(block));
}

AudioManager gManager;

} // namespace fge::audio
