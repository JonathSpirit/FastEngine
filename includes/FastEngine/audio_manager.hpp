#ifndef _FGE_AUDIO_MANAGER_HPP_INCLUDED
#define _FGE_AUDIO_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>

#define FGE_AUDIO_DEFAULT FGE_AUDIO_BAD
#define FGE_AUDIO_BAD ""

namespace fge
{
namespace audio
{

struct AudioData
{
    std::shared_ptr<sf::SoundBuffer> _audio;
    bool _valid;
    std::string _path;
};

using AudioDataPtr = std::shared_ptr<fge::audio::AudioData>;
using AudioDataType = std::unordered_map<std::string, fge::audio::AudioDataPtr>;

void FGE_API Init();
bool FGE_API IsInit();
void FGE_API Uninit();

std::size_t FGE_API GetAudioSize();

std::mutex& GetMutex();
fge::audio::AudioDataType::const_iterator FGE_API GetCBegin();
fge::audio::AudioDataType::const_iterator FGE_API GetCEnd();

const fge::audio::AudioDataPtr& FGE_API GetBadAudio();
fge::audio::AudioDataPtr FGE_API GetAudio(const std::string& name);

bool FGE_API Check(const std::string& name);

bool FGE_API LoadFromFile(const std::string& name, const std::string& path);
bool FGE_API Unload(const std::string& name);
void FGE_API UnloadAll();

bool FGE_API Push(const std::string& name, const fge::audio::AudioDataPtr& data);

}//end audio
}//end fge


#endif // _FGE_AUDIO_MANAGER_HPP_INCLUDED
