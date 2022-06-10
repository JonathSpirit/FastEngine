#ifndef _FGE_ARBITRARYJSONTYPES_HPP_INCLUDED
#define _FGE_ARBITRARYJSONTYPES_HPP_INCLUDED

#include <SFML/System.hpp>
#include <json.hpp>

namespace sf
{

template<class T>
void to_json(nlohmann::json& j, const sf::Vector2<T>& p);
template<class T>
void from_json(const nlohmann::json& j, sf::Vector2<T>& p);

template<class T>
void to_json(nlohmann::json& j, const sf::Vector3<T>& p);
template<class T>
void from_json(const nlohmann::json& j, sf::Vector3<T>& p);

void to_json(nlohmann::json& j, const sf::Color& p);
void from_json(const nlohmann::json& j, sf::Color& p);

}//end sc

#include <FastEngine/arbitraryJsonTypes.inl>

#endif // _FGE_ARBITRARYJSONTYPES_HPP_INCLUDED
