#ifndef _FGE_C_OBJECT_HPP_INCLUDED
#define _FGE_C_OBJECT_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_tagList.hpp>
#include <FastEngine/C_networkType.hpp>
#include <FastEngine/C_event.hpp>
#include <FastEngine/C_packet.hpp>
#include <FastEngine/C_guiElement.hpp>
#include <SFML/Graphics.hpp>
#include <json.hpp>

#include <string>
#include <chrono>

#define FGE_OBJ_BADCLASSNAME "NULL"
#define FGE_OBJ_NOSCENE nullptr
#define FGE_OBJ_DEFAULT_COPYMETHOD(objClass) fge::Object* copy() override { return new objClass(*this); }

namespace fge
{

class Scene;

class ObjectData;
using ObjectDataShared = std::shared_ptr<fge::ObjectData>;

class FGE_API Object : public sf::Drawable, public sf::Transformable
{
public:
    Object() = default;
    ~Object() override = default;

    virtual fge::Object* copy();

    virtual void first(fge::Scene* scene_ptr);
    virtual void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr);
    virtual void update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    virtual void networkRegister();
    virtual void removed(fge::Scene* scene_ptr);

    virtual void save(nlohmann::json& jsonObject, fge::Scene* scene_ptr);
    virtual void load(nlohmann::json& jsonObject, fge::Scene* scene_ptr);
    virtual void pack(fge::net::Packet& pck);
    virtual void unpack(fge::net::Packet& pck);

    virtual std::string getClassName() const;
    virtual std::string getReadableClassName() const;

    virtual sf::FloatRect getGlobalBounds() const;
    virtual sf::FloatRect getLocalBounds() const;

    bool saveInFile(const std::string& path);
    bool loadFromFile(const std::string& path);
    static fge::Object* LoadFromFile(const std::string& path);

    ///Data

    fge::TagList _tags;

 	///Network

    fge::net::NetworkTypeContainer _netList;

    ///Scene control

    fge::ObjectDataShared _myObjectData = nullptr;
    bool _alwaysDrawed = false;
};

}//end fge

#endif // _FGE_C_OBJECT_HPP_INCLUDED
