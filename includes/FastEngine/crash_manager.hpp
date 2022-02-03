#ifndef _FGE_CRASH_MANAGER_HPP_INCLUDED
#define _FGE_CRASH_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <SFML/Graphics.hpp>
#include <string>

namespace fge
{
namespace crash
{

void FGE_API Init(sf::RenderWindow& screen, sf::Font& font);

void FGE_API SetInfoText(const std::string& txt);

}//end crash
}//end fge

#endif // _FGE_CRASH_MANAGER_HPP_INCLUDED
