/*
 * Copyright 2022 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FGE_C_NETWORKTYPE_HPP_INCLUDED
#define _FGE_C_NETWORKTYPE_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_packet.hpp>
#include <FastEngine/C_propertyList.hpp>
#include <FastEngine/C_callback.hpp>
#include <FastEngine/C_identity.hpp>
#include <FastEngine/C_smoothFloat.hpp>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace fge
{

class Scene;
class TagList;

namespace net
{

using NetworkPerClientConfigByte = uint8_t;
enum NetworkPerClientConfigByteMasks : NetworkPerClientConfigByte
{
    CONFIG_BYTE_MODIFIED_CHECK = 1 << 0, ///< The value has been modified and must be updated
    CONFIG_BYTE_EXPLICIT_UPDATE = 1 << 1 ///< The client require an explicit update
};

using NetworkPerClientModificationTable = std::unordered_map<fge::net::Identity, fge::net::NetworkPerClientConfigByte, fge::net::IdentityHash>;

class ClientList;

/**
 * \class NetworkTypebase
 * \ingroup network
 * \brief Base class for a network type
 *
 * A network type is a class that can be serialized and deserialized by the network.
 *
 * The general idea is to provide a pointer of a variable to a NetworkType class.
 * A clone of the variable type is made internally to be compared with the original one for change detection.
 */
class FGE_API NetworkTypeBase
{
protected:
    NetworkTypeBase() = default;

public:
    virtual ~NetworkTypeBase() = default;

    /**
     * \brief Get the source pointer that have been used to create this network type
     *
     * \return A pointer to the source object
     */
    virtual const void* getSource() const = 0;

    /**
     * \brief Apply the data packed by the same network type from a server
     *
     * \param pck The packet containing the data
     * \return \b true if the data has been applied, \b false otherwise
     */
    virtual bool applyData(fge::net::Packet& pck) = 0;
    /**
     * \brief Pack the data into a packet and reset the modification flag of the identity
     *
     * \param pck The packet to pack the data into
     * \param id The identity of the client to pack the data for
     */
    virtual void packData(fge::net::Packet& pck, const fge::net::Identity& id) = 0;
    /**
     * \brief Pack the data without any client identity
     *
     * \param pck The packet to pack the data into
     */
    virtual void packData(fge::net::Packet& pck) = 0;

    /**
     * \brief Do a clients checkup with the specified client list
     *
     * The first step of the checkup is to add client in a table if they are not already in it
     * and remove clients that are not in the list anymore.
     *
     * Then the checkup check if the value have been modified and apply a modification flag to all clients.
     *
     * \param clients The client list to checkup with
     * \return \b true if there was a change in the value, \b false otherwise
     */
    virtual bool clientsCheckup(const fge::net::ClientList& clients);

    /**
     * \brief Check if the modification flag is set for the specified client identity
     *
     * \param id The client identity to check
     * \return \b true if the modification flag is set, \b false otherwise
     */
    virtual bool checkClient(const fge::net::Identity& id) const;
    /**
     * \brief Force the modification flag to be set for the specified client identity
     *
     * \param id The client identity to force the modification flag for
     */
    virtual void forceCheckClient(const fge::net::Identity& id);
    /**
     * \brief Reset the modification flag for the specified client identity
     *
     * \param id The client identity to reset the modification flag for
     */
    virtual void forceUncheckClient(const fge::net::Identity& id);
    /**
     * \brief Ask for an explicit update of the value for the specified client identity
     *
     * A network type can discard the explicit update if it is not necessary or unimplemented.
     *
     * \param id The client identity to ask for an explicit update
     */
    virtual void requireExplicitUpdateClient(const fge::net::Identity& id);

    /**
     * \brief Check if the value have been modified
     *
     * \return \b true if the value have been modified, \b false otherwise
     */
    virtual bool check() const = 0;
    /**
     * \brief Force the value to be modified (even if it is not)
     */
    virtual void forceCheck() = 0;
    /**
     * \brief Remove the forced modification of the value
     */
    virtual void forceUncheck() = 0;
    /**
     * \brief Check if the value is forced to be modified
     *
     * \return \b true if the value is forced to be modified, \b false otherwise
     */
    [[nodiscard]] bool isForced() const;

    /**
     * \brief Clear the need for an explicit update
     */
    void clearNeedUpdateFlag();
    /**
     * \brief Tell that this network type need an explicit update from the server
     */
    void needUpdate();
    /**
     * \brief Check if this network type need an explicit update from the server
     *
     * \return \b true if this network type need an explicit update from the server, \b false otherwise
     */
    bool isNeedingUpdate() const;

    /**
     * \brief Callback called when the value have been applied
     */
    fge::CallbackHandler<> _onApplied;

protected:
    fge::net::NetworkPerClientModificationTable _g_tableId;
    bool _g_needUpdate{false};
    bool _g_force{false};
};

/**
 * \class NetworkType
 * \ingroup network
 * \brief The default network type for most trivial types
 *
 * \tparam T The type of the value
 */
template<class T>
class NetworkType : public NetworkTypeBase
{
public:
    NetworkType(T* source);
    ~NetworkType() override = default;

    const void* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, const fge::net::Identity& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

private:
    T g_typeCopy;
    T* g_typeSource;
};

/**
 * \class NetworkTypeScene
 * \ingroup network
 * \brief The network type for a scene
 */
class FGE_API NetworkTypeScene : public NetworkTypeBase
{
public:
    NetworkTypeScene(fge::Scene* source);
    ~NetworkTypeScene() override = default;

    const void* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, const fge::net::Identity& id) override;
    void packData(fge::net::Packet& pck) override;

    bool clientsCheckup(const fge::net::ClientList& clients) override;

    bool checkClient(const fge::net::Identity& id) const override;
    void forceCheckClient(const fge::net::Identity& id) override;
    void forceUncheckClient(const fge::net::Identity& id) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

private:
    fge::Scene* g_typeSource;
};

/**
 * \class NetworkTypeTag
 * \ingroup network
 * \brief The network type for a tag
 */
class FGE_API NetworkTypeTag : public NetworkTypeBase
{
public:
    NetworkTypeTag(fge::TagList* source, std::string tag);
    ~NetworkTypeTag() override = default;

    const void* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, const fge::net::Identity& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

private:
    fge::TagList* g_typeSource;
    std::string g_tag;
};

/**
 * \class NetworkTypeSmoothVec2FloatSetter
 * \ingroup network
 * \brief The network type for a source Vector2f that can only be set with a setter method
 */
class FGE_API NetworkTypeSmoothVec2FloatSetter : public NetworkTypeBase
{
public:
    NetworkTypeSmoothVec2FloatSetter(const sf::Vector2f* source, std::function<void(const sf::Vector2f&)> setter, float errorRange);
    ~NetworkTypeSmoothVec2FloatSetter() override = default;

    const void* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, const fge::net::Identity& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

    const sf::Vector2f& getCache() const;
    void setErrorRange(float range);
    float getErrorRange() const;

private:
    const sf::Vector2f* g_typeSource;
    sf::Vector2f g_typeCopy;
    std::function<void(const sf::Vector2f&)> g_setter;
    float g_errorRange;
};
/**
 * \class NetworkTypeSmoothFloatGetterSetter
 * \ingroup network
 * \brief The network type for a float that can only be get/set by a getter/setter method
 */
class FGE_API NetworkTypeSmoothFloatGetterSetter : public NetworkTypeBase
{
public:
    NetworkTypeSmoothFloatGetterSetter(std::function<float(void)> getter, std::function<void(float)> setter, float errorRange);
    ~NetworkTypeSmoothFloatGetterSetter() override = default;

    const void* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, const fge::net::Identity& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

    float getCache() const;
    void setErrorRange(float range);
    float getErrorRange() const;

private:
    std::function<float()> g_getter;
    float g_typeCopy;
    std::function<void(float)> g_setter;
    float g_errorRange;
};

/**
 * \class NetworkTypeSmoothVec2Float
 * \ingroup network
 * \brief The network type for a smooth vector2 float
 */
class FGE_API NetworkTypeSmoothVec2Float : public NetworkTypeBase
{
public:
    NetworkTypeSmoothVec2Float(fge::net::SmoothVec2Float* source);
    ~NetworkTypeSmoothVec2Float() override = default;

    const void* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, const fge::net::Identity& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

private:
    fge::net::SmoothVec2Float* g_typeSource;
    sf::Vector2f g_typeCopy;
};

/**
 * \class NetworkTypeSmoothFloat
 * \ingroup network
 * \brief The network type for a smooth float
 */
class FGE_API NetworkTypeSmoothFloat : public NetworkTypeBase
{
public:
    NetworkTypeSmoothFloat(fge::net::SmoothFloat* source);
    ~NetworkTypeSmoothFloat() override = default;

    const void* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, const fge::net::Identity& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

private:
    fge::net::SmoothFloat* g_typeSource;
    float g_typeCopy;
};

/**
 * \class NetworkTypeProperty
 * \ingroup network
 * \brief The network type for a property
 *
 * \tparam T The type of the property
 */
template <class T>
class NetworkTypeProperty : public NetworkTypeBase
{
public:
    NetworkTypeProperty(fge::Property* source);
    ~NetworkTypeProperty() override = default;

    const void* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, const fge::net::Identity& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

private:
    fge::Property* g_typeSource;
};

/**
 * \class NetworkTypePropertyList
 * \ingroup network
 * \brief The network type for a property inside a list
 *
 * \tparam T The type of the property
 */
template <class T>
class NetworkTypePropertyList : public NetworkTypeBase
{
public:
    NetworkTypePropertyList(fge::PropertyList* source, const std::string& vname);
    ~NetworkTypePropertyList() override = default;

    const void* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, const fge::net::Identity& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

    const std::string& getValueName() const;

private:
    fge::PropertyList* g_typeSource;
    std::string g_vname;
};

/**
 * \class NetworkTypeManual
 * \ingroup network
 * \brief The network type for a trivial type but triggered manually
 *
 * \tparam T The type of the value
 */
template<class T>
class NetworkTypeManual : public NetworkTypeBase
{
public:
    NetworkTypeManual(T* source);
    ~NetworkTypeManual() override = default;

    const void* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, const fge::net::Identity& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

    void trigger();

private:
    T* g_typeSource;
    bool g_trigger;
};

/**
 * \class NetworkTypeContainer
 * \ingroup network
 * \brief The network type for a container
 */
class FGE_API NetworkTypeContainer
{
public:
    NetworkTypeContainer() = default;
    ~NetworkTypeContainer() = default;

    //Copy function that does nothing
    NetworkTypeContainer(const NetworkTypeContainer& n){};
    NetworkTypeContainer(NetworkTypeContainer& n){};
    NetworkTypeContainer& operator=(const NetworkTypeContainer& n){return *this;};
    NetworkTypeContainer& operator=(NetworkTypeContainer& n){return *this;};

    void clear();

    void clientsCheckup(const fge::net::ClientList& clients);
    void forceCheckClient(const fge::net::Identity& id);
    void forceUncheckClient(const fge::net::Identity& id);

    void push(fge::net::NetworkTypeBase* newNet);

    void reserve(size_t n);

    std::size_t packNeededUpdate(fge::net::Packet& pck);
    void unpackNeededUpdate(fge::net::Packet& pck, const fge::net::Identity& id);

    inline size_t size() const
    {
        return this->g_data.size();
    }
    inline fge::net::NetworkTypeBase* at(size_t index)
    {
        return this->g_data.at(index).get();
    }
    inline fge::net::NetworkTypeBase* operator[](size_t index)
    {
        return this->g_data[index].get();
    }
    inline fge::net::NetworkTypeBase* back()
    {
        return this->g_data.back().get();
    }
    inline fge::net::NetworkTypeBase* front()
    {
        return this->g_data.front().get();
    }

private:
    std::vector<std::unique_ptr<fge::net::NetworkTypeBase> > g_data;
};

}//end net
}//end fge

#include <FastEngine/C_networkType.inl>

#endif // _FGE_C_NETWORKTYPE_HPP_INCLUDED
