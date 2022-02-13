#include "FastEngine/audio_manager.hpp"

namespace fge
{
namespace audio
{

namespace
{

fge::audio::AudioDataPtr _dataAudioBad;
fge::audio::AudioDataType _dataAudio;
std::mutex _dataMutex;

}//end

void Init()
{
    if ( _dataAudioBad == nullptr )
    {
        _dataAudioBad = std::make_shared<fge::audio::AudioData>();
        _dataAudioBad->_audio = std::make_shared<sf::SoundBuffer>();
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

const fge::audio::AudioDataPtr& GetBadAudio()
{
    return _dataAudioBad;
}
fge::audio::AudioDataPtr GetAudio(const std::string& name)
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

bool Check(const std::string& name)
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

bool LoadFromFile(const std::string& name, const std::string& path)
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

    _dataAudio[name] = std::move(buff);
    return true;
}

bool Unload(const std::string& name)
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

    for (auto & data : _dataAudio)
    {
        data.second->_valid = false;
        data.second->_audio = _dataAudioBad->_audio;
    }
    _dataAudio.clear();
}

bool Push(const std::string& name, const fge::audio::AudioDataPtr& data)
{
    if (name == FGE_AUDIO_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    if ( fge::audio::Check(name) )
    {
        return false;
    }

    _dataAudio.emplace(name, data);
    return true;
}

}//end audio
}//end fge
