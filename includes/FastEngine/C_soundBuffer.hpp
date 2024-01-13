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

#ifndef _FGE_C_SOUNDBUFFER_HPP_INCLUDED
#define _FGE_C_SOUNDBUFFER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/manager/audio_manager.hpp"

namespace fge
{

class FGE_API SoundBuffer
{
public:
    SoundBuffer();
    SoundBuffer(std::string const& name);
    SoundBuffer(char const* name);
    SoundBuffer(fge::audio::AudioDataPtr const& data);

    void clear();

    bool valid() const;

    fge::audio::AudioDataPtr const& getData() const;
    std::string const& getName() const;

    void operator=(std::string const& name);
    void operator=(char const* name);
    void operator=(fge::audio::AudioDataPtr const& data);

    operator Mix_Chunk*();
    operator Mix_Chunk const*() const;

    operator std::string&();
    operator std::string const&() const;

private:
    fge::audio::AudioDataPtr g_data;
    std::string g_name;
};

} // namespace fge

#endif // _FGE_C_SOUNDBUFFER_HPP_INCLUDED
