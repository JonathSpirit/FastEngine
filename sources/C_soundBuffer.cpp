#include "FastEngine/C_soundBuffer.hpp"

namespace fge
{

SoundBuffer::SoundBuffer() :
    g_data(fge::audio::GetBadAudio()),
    g_name(FGE_AUDIO_BAD)
{
}
SoundBuffer::SoundBuffer( const std::string& name ) :
    g_data(fge::audio::GetAudio(name)),
    g_name(name)
{
}
SoundBuffer::SoundBuffer( const char* name ) :
    g_data(fge::audio::GetAudio(std::string(name))),
    g_name(name)
{
}
SoundBuffer::SoundBuffer( const fge::audio::AudioDataPtr& data ) :
    g_data(data),
    g_name(FGE_AUDIO_BAD)
{
}

void SoundBuffer::clear()
{
    this->g_data = fge::audio::GetBadAudio();
    this->g_name = FGE_AUDIO_BAD;
}

bool SoundBuffer::valid() const
{
    return this->g_data->_valid;
}

const fge::audio::AudioDataPtr& SoundBuffer::getData() const
{
    return this->g_data;
}
const std::string& SoundBuffer::getName() const
{
    return this->g_name;
}

void SoundBuffer::operator =( const std::string& name )
{
    this->g_name = name;
    this->g_data = fge::audio::GetAudio(name);
}
void SoundBuffer::operator =( const char* name )
{
    this->g_name = std::string(name);
    this->g_data = fge::audio::GetAudio(this->g_name);
}
void SoundBuffer::operator =( const fge::audio::AudioDataPtr& data )
{
    this->g_name = FGE_AUDIO_BAD;
    this->g_data = data;
}

SoundBuffer::operator sf::SoundBuffer*()
{
    return this->g_data->_audio.get();
}
SoundBuffer::operator const sf::SoundBuffer*() const
{
    return this->g_data->_audio.get();
}

SoundBuffer::operator sf::SoundBuffer&()
{
    return *this->g_data->_audio;
}
SoundBuffer::operator const sf::SoundBuffer&() const
{
    return *this->g_data->_audio;
}

SoundBuffer::operator std::string&()
{
    return this->g_name;
}
SoundBuffer::operator const std::string&() const
{
    return this->g_name;
}

}//end fge
