#include "FastEngine/C_soundBuffer.hpp"

namespace fge
{

FGE_API SoundBuffer::SoundBuffer() :
    g_data(fge::audio::GetBadAudio()),
    g_name(FGE_AUDIO_BAD)
{
}
FGE_API SoundBuffer::SoundBuffer( const std::string& name ) :
    g_data(fge::audio::GetAudio(name)),
    g_name(name)
{
}
FGE_API SoundBuffer::SoundBuffer( const char* name ) :
    g_data(fge::audio::GetAudio(std::string(name))),
    g_name(name)
{
}
FGE_API SoundBuffer::SoundBuffer( const fge::audio::AudioDataPtr& data ) :
    g_data(data),
    g_name(FGE_AUDIO_BAD)
{
}

void FGE_API SoundBuffer::clear()
{
    this->g_data = fge::audio::GetBadAudio();
    this->g_name = FGE_AUDIO_BAD;
}

bool FGE_API SoundBuffer::valid() const
{
    return this->g_data->_valid;
}

const fge::audio::AudioDataPtr& FGE_API SoundBuffer::getData() const
{
    return this->g_data;
}
const std::string& FGE_API SoundBuffer::getName() const
{
    return this->g_name;
}

void FGE_API SoundBuffer::operator =( const std::string& name )
{
    this->g_name = name;
    this->g_data = fge::audio::GetAudio(name);
}
void FGE_API SoundBuffer::operator =( const char* name )
{
    this->g_name = std::string(name);
    this->g_data = fge::audio::GetAudio(this->g_name);
}
void FGE_API SoundBuffer::operator =( const fge::audio::AudioDataPtr& data )
{
    this->g_name = FGE_AUDIO_BAD;
    this->g_data = data;
}

FGE_API SoundBuffer::operator sf::SoundBuffer*()
{
    return this->g_data->_audio.get();
}
FGE_API SoundBuffer::operator const sf::SoundBuffer*() const
{
    return this->g_data->_audio.get();
}

FGE_API SoundBuffer::operator sf::SoundBuffer&()
{
    return *this->g_data->_audio;
}
FGE_API SoundBuffer::operator const sf::SoundBuffer&() const
{
    return *this->g_data->_audio;
}

FGE_API SoundBuffer::operator std::string&()
{
    return this->g_name;
}
FGE_API SoundBuffer::operator const std::string&() const
{
    return this->g_name;
}

}//end fge
