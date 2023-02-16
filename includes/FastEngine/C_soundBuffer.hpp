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

#ifndef _FGE_C_SOUNDBUFFER_HPP_INCLUDED
#define _FGE_C_SOUNDBUFFER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include "FastEngine/manager/audio_manager.hpp"

namespace fge
{

class FGE_API SoundBuffer
{
public:
    SoundBuffer();
    SoundBuffer(const std::string& name);
    SoundBuffer(const char* name);
    SoundBuffer(const fge::audio::AudioDataPtr& data);

    void clear();

    bool valid() const;

    const fge::audio::AudioDataPtr& getData() const;
    const std::string& getName() const;

    void operator=(const std::string& name);
    void operator=(const char* name);
    void operator=(const fge::audio::AudioDataPtr& data);

    operator Mix_Chunk*();
    operator const Mix_Chunk*() const;

    operator std::string&();
    operator const std::string&() const;

private:
    fge::audio::AudioDataPtr g_data;
    std::string g_name;
};

} // namespace fge

#endif // _FGE_C_SOUNDBUFFER_HPP_INCLUDED
