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

#include "FastEngine/manager/audio_manager.hpp"

namespace fge::audio
{

namespace
{

fge::audio::AudioDataPtr _dataAudioBad;
fge::audio::AudioDataType _dataAudio;
std::mutex _dataMutex;

} // namespace

void Init()
{
    if (_dataAudioBad == nullptr)
    {
        Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_MID | MIX_INIT_OPUS);

        Mix_OpenAudioDevice(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048, nullptr,
                            SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);

        //Taken from https://cable.ayra.ch/empty/
        uint8_t emptyWaveFile[] = {0x52, 0x49, 0x46, 0x46, 0x25, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45,
                                   0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
                                   0x44, 0xAC, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00, 0x02, 0x00, 0x10, 0x00,
                                   0x64, 0x61, 0x74, 0x61, 0x74, 0x00, 0x00, 0x00, 0x00};

        _dataAudioBad = std::make_shared<fge::audio::AudioData>();
        _dataAudioBad->_audio = std::shared_ptr<Mix_Chunk>(Mix_QuickLoad_WAV(emptyWaveFile), MixerChunkDeleter());
        _dataAudioBad->_valid = false;
    }
}
bool IsInit()
{
    return _dataAudioBad != nullptr;
}
void Uninit()
{
    _dataAudio.clear();
    _dataAudioBad = nullptr;

    Mix_CloseAudio();
    Mix_Quit();
}

std::size_t GetAudioSize()
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    return _dataAudio.size();
}

std::mutex& GetMutex()
{
    return _dataMutex;
}

fge::audio::AudioDataType::const_iterator GetCBegin()
{
    return _dataAudio.cbegin();
}
fge::audio::AudioDataType::const_iterator GetCEnd()
{
    return _dataAudio.cend();
}

fge::audio::AudioDataPtr const& GetBadAudio()
{
    return _dataAudioBad;
}
fge::audio::AudioDataPtr GetAudio(std::string const& name)
{
    if (name == FGE_AUDIO_BAD)
    {
        return _dataAudioBad;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataAudio.find(name);

    if (it != _dataAudio.end())
    {
        return it->second;
    }
    return _dataAudioBad;
}

bool Check(std::string const& name)
{
    if (name == FGE_AUDIO_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataAudio.find(name);

    if (it != _dataAudio.end())
    {
        return true;
    }
    return false;
}

bool LoadFromFile(std::string const& name, std::string const& path)
{
    if (name == FGE_AUDIO_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataAudio.find(name);

    if (it != _dataAudio.end())
    {
        return false;
    }

    Mix_Chunk* tmpAudio = Mix_LoadWAV(path.c_str());

    if (tmpAudio == nullptr)
    {
        return false;
    }

    fge::audio::AudioDataPtr buff = std::make_shared<fge::audio::AudioData>();
    buff->_audio = std::shared_ptr<Mix_Chunk>(tmpAudio, MixerChunkDeleter());
    buff->_valid = true;
    buff->_path = path;

    _dataAudio[name] = std::move(buff);
    return true;
}

bool Unload(std::string const& name)
{
    if (name == FGE_AUDIO_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataAudio.find(name);

    if (it != _dataAudio.end())
    {
        it->second->_valid = false;
        it->second->_audio = _dataAudioBad->_audio;
        _dataAudio.erase(it);
        return true;
    }
    return false;
}
void UnloadAll()
{
    std::lock_guard<std::mutex> lck(_dataMutex);

    for (auto& data: _dataAudio)
    {
        data.second->_valid = false;
        data.second->_audio = _dataAudioBad->_audio;
    }
    _dataAudio.clear();
}

bool Push(std::string const& name, fge::audio::AudioDataPtr const& data)
{
    if (name == FGE_AUDIO_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    if (fge::audio::Check(name))
    {
        return false;
    }

    _dataAudio.emplace(name, data);
    return true;
}

} // namespace fge::audio
