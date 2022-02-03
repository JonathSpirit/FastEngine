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

void FGE_API Uninit();

void FGE_API Close(const std::string& name);

std::size_t FGE_API GetScreenSize();

std::shared_ptr<sf::RenderWindow> FGE_API Get(const std::string& name);

bool FGE_API Check(const std::string& name);

std::shared_ptr<sf::RenderWindow> FGE_API New(const std::string& name);

}//end screen
}//end fge


#endif // _FGE_SCREEN_MANAGER_HPP_INCLUDED
