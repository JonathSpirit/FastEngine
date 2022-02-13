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

FGE_API void Init();
FGE_API bool IsInit();
FGE_API void Uninit();

FGE_API std::size_t GetAudioSize();

FGE_API std::mutex& GetMutex();
FGE_API fge::audio::AudioDataType::const_iterator GetCBegin();
FGE_API fge::audio::AudioDataType::const_iterator GetCEnd();

FGE_API const fge::audio::AudioDataPtr& GetBadAudio();
FGE_API fge::audio::AudioDataPtr GetAudio(const std::string& name);

FGE_API bool Check(const std::string& name);

FGE_API bool LoadFromFile(const std::string& name, const std::string& path);
FGE_API bool Unload(const std::string& name);
FGE_API void UnloadAll();

FGE_API bool Push(const std::string& name, const fge::audio::AudioDataPtr& data);

}//end audio
}//end fge


#endif // _FGE_AUDIO_MANAGER_HPP_INCLUDED
