#include "FastEngine/audio_manager.hpp"

namespace fge
{
namespace audio
{

namespace
{

fge::audio::AudioDataPtr __dataAudioBad;
fge::audio::AudioDataType __dataAudio;
std::mutex __dataMutex;

}//end

void FGE_API Init()
{
    if ( __dataAudioBad == nullptr )
    {
        __dataAudioBad = std::make_shared<fge::audio::AudioData>();
        __dataAudioBad->_audio = std::make_shared<sf::SoundBuffer>();
        __dataAudioBad->_valid = false;
    }
}
bool FGE_API IsInit()
{
    return __dataAudioBad != nullptr;
}
void FGE_API Uninit()
{
    __dataAudio.clear();
    __dataAudioBad = nullptr;
}

std::size_t FGE_API GetAudioSize()
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    return __dataAudio.size();
}

std::mutex& FGE_API GetMutex()
{
    return __dataMutex;
}

fge::audio::AudioDataType::const_iterator FGE_API GetCBegin()
{
    return __dataAudio.cbegin();
}
fge::audio::AudioDataType::const_iterator FGE_API GetCEnd()
{
    return __dataAudio.cend();
}

const fge::audio::AudioDataPtr& FGE_API GetBadAudio()
{
    return __dataAudioBad;
}
fge::audio::AudioDataPtr FGE_API GetAudio(const std::string& name)
{
    if (name == FGE_AUDIO_BAD)
    {
        return __dataAudioBad;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::audio::AudioDataType::iterator it = __dataAudio.find(name);

    if (it != __dataAudio.end())
    {
        return it->second;
    }
    return __dataAudioBad;
}

bool FGE_API Check(const std::string& name)
{
    if (name == FGE_AUDIO_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::audio::AudioDataType::iterator it = __dataAudio.find(name);

    if (it != __dataAudio.end())
    {
        return true;
    }
    return false;
}

bool FGE_API LoadFromFile(const std::string& name, const std::string& path)
{
    if (name == FGE_AUDIO_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::audio::AudioDataType::iterator it = __dataAudio.find(name);

    if (it != __dataAudio.end())
    {
        return false;
    }

    sf::SoundBuffer* tmpAudio = new sf::SoundBuffer();

    if ( !tmpAudio->loadFromFile(path) )
    {
        delete tmpAudio;
        return false;
    }

    fge::audio::AudioDataPtr buff = std::make_shared<fge::audio::AudioData>();
    buff->_audio = std::move( std::shared_ptr<sf::SoundBuffer>(tmpAudio) );
    buff->_valid = true;
    buff->_path = path;

    __dataAudio[name] = std::move(buff);
    return true;
}

bool FGE_API Unload(const std::string& name)
{
    if (name == FGE_AUDIO_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::audio::AudioDataType::iterator it = __dataAudio.find(name);

    if (it != __dataAudio.end())
    {
        it->second->_valid = false;
        it->second->_audio = __dataAudioBad->_audio;
        __dataAudio.erase(it);
        return true;
    }
    return false;
}
void FGE_API UnloadAll()
{
    std::lock_guard<std::mutex> lck(__dataMutex);

    for (fge::audio::AudioDataType::iterator it=__dataAudio.begin(); it!=__dataAudio.end(); ++it)
    {
        it->second->_valid = false;
        it->second->_audio = __dataAudioBad->_audio;
    }
    __dataAudio.clear();
}

bool FGE_API Push(const std::string& name, const fge::audio::AudioDataPtr& data)
{
    if (name == FGE_AUDIO_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    if ( fge::audio::Check(name) )
    {
        return false;
    }

    __dataAudio.emplace(name, data);
    return true;
}

}//end audio
}//end fge
