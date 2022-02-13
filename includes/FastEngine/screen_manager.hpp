#ifndef _FGE_SCREEN_MANAGER_HPP_INCLUDED
#define _FGE_SCREEN_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <string>
#include <memory>

namespace fge
{
namespace screen
{

FGE_API void Uninit();

FGE_API void Close(const std::string& name);

FGE_API std::size_t GetScreenSize();

FGE_API std::shared_ptr<sf::RenderWindow> Get(const std::string& name);

FGE_API bool Check(const std::string& name);

FGE_API std::shared_ptr<sf::RenderWindow> New(const std::string& name);

}//end screen
}//end fge


#endif // _FGE_SCREEN_MANAGER_HPP_INCLUDED
