/*
 * Copyright 2024 Guillaume Guillet
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

#include "FastEngine/C_soundBuffer.hpp"

namespace fge
{

SoundBuffer::SoundBuffer() :
        g_data(fge::audio::GetBadAudio()),
        g_name(FGE_AUDIO_BAD)
{}
SoundBuffer::SoundBuffer(std::string const& name) :
        g_data(fge::audio::GetAudio(name)),
        g_name(name)
{}
SoundBuffer::SoundBuffer(char const* name) :
        g_data(fge::audio::GetAudio(std::string(name))),
        g_name(name)
{}
SoundBuffer::SoundBuffer(fge::audio::AudioDataPtr const& data) :
        g_data(data),
        g_name(FGE_AUDIO_BAD)
{}

void SoundBuffer::clear()
{
    this->g_data = fge::audio::GetBadAudio();
    this->g_name = FGE_AUDIO_BAD;
}

bool SoundBuffer::valid() const
{
    return this->g_data->_valid;
}

fge::audio::AudioDataPtr const& SoundBuffer::getData() const
{
    return this->g_data;
}
std::string const& SoundBuffer::getName() const
{
    return this->g_name;
}

void SoundBuffer::operator=(std::string const& name)
{
    this->g_name = name;
    this->g_data = fge::audio::GetAudio(name);
}
void SoundBuffer::operator=(char const* name)
{
    this->g_name = std::string(name);
    this->g_data = fge::audio::GetAudio(this->g_name);
}
void SoundBuffer::operator=(fge::audio::AudioDataPtr const& data)
{
    this->g_name = FGE_AUDIO_BAD;
    this->g_data = data;
}

SoundBuffer::operator Mix_Chunk*()
{
    return this->g_data->_audio.get();
}
SoundBuffer::operator Mix_Chunk const*() const
{
    return this->g_data->_audio.get();
}

SoundBuffer::operator std::string&()
{
    return this->g_name;
}
SoundBuffer::operator std::string const&() const
{
    return this->g_name;
}

} // namespace fge
