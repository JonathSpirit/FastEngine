#ifndef _FGE_C_SOUNDBUFFER_HPP_INCLUDED
#define _FGE_C_SOUNDBUFFER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/audio_manager.hpp>

namespace fge
{

class FGE_API SoundBuffer
{
public:
    SoundBuffer();
    SoundBuffer( const std::string& name );
    SoundBuffer( const char* name );
    SoundBuffer( const fge::audio::AudioDataPtr& data );

    void clear();

    bool valid() const;

    const fge::audio::AudioDataPtr& getData() const;
    const std::string& getName() const;

    void operator =( const std::string& name );
    void operator =( const char* name );
    void operator =( const fge::audio::AudioDataPtr& data );

    explicit operator sf::SoundBuffer*();
    explicit operator const sf::SoundBuffer*() const;

    operator sf::SoundBuffer&();
    operator const sf::SoundBuffer&() const;

    operator std::string&();
    operator const std::string&() const;

private:
    fge::audio::AudioDataPtr g_data;
    std::string g_name;
};

}//end fge

#endif // _FGE_C_SOUNDBUFFER_HPP_INCLUDED
