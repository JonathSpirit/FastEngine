/*
 * Copyright 2024 Guillaume Guillet
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

#include "FastEngine/fge_extern.hpp"
#include "C_identity.hpp"
#include "C_packet.hpp"
#include "FastEngine/C_callback.hpp"
#include "FastEngine/C_dataAccessor.hpp"
#include "FastEngine/C_propertyList.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#define FGE_NET_WAITING_UPDATE_DELAY std::chrono::milliseconds(800)

namespace fge
{

class Scene;
class TagList;

namespace net
{

enum PerClientConfigs : uint8_t
{
    CLIENTCONFIG_MODIFIED_FLAG = 1 << 0,               ///< The value has been modified and must be updated
    CLIENTCONFIG_REQUIRE_EXPLICIT_UPDATE_FLAG = 1 << 1 ///< The client require an explicit update
};

struct PerClientConfig
{
    std::underlying_type_t<PerClientConfigs> _config{0};
    void* _customData{nullptr};
};

using NetworkPerClientModificationTable =
        std::unordered_map<fge::net::Identity, fge::net::PerClientConfig, fge::net::IdentityHash>;

class ClientList;

/**
 * \class NetworkTypeBase
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
    virtual void const* getSource() const = 0;

    /**
     * \brief Apply the data packed by the same network type from a server
     *
     * \param pck The packet containing the data
     * \return \b true if the data has been applied, \b false otherwise
     */
    virtual bool applyData(fge::net::Packet const& pck) = 0;
    /**
     * \brief Pack the data into a packet and reset the modification flag of the identity
     *
     * \param pck The packet to pack the data into
     * \param id The identity of the client to pack the data for
     */
    virtual void packData(fge::net::Packet& pck, fge::net::Identity const& id) = 0;
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
     * \param force \b true to force the checkup from the complete list of clients
     * \return \b true if there was a change in the value, \b false otherwise
     */
    virtual bool clientsCheckup(fge::net::ClientList const& clients, bool force);

    /**
     * \brief Check if the modification flag is set for the specified client identity
     *
     * \param id The client identity to check
     * \return \b true if the modification flag is set, \b false otherwise
     */
    virtual bool checkClient(fge::net::Identity const& id) const;
    /**
     * \brief Force the modification flag to be set for the specified client identity
     *
     * \param id The client identity to force the modification flag for
     */
    virtual void forceCheckClient(fge::net::Identity const& id);
    /**
     * \brief Reset the modification flag for the specified client identity
     *
     * \param id The client identity to reset the modification flag for
     */
    virtual void forceUncheckClient(fge::net::Identity const& id);
    /**
     * \brief Ask for an explicit update of the value for the specified client identity
     *
     * A network type can discard the explicit update if it is not necessary or unimplemented.
     *
     * \param id The client identity to ask for an explicit update
     */
    virtual void requireExplicitUpdateClient(fge::net::Identity const& id);

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

    void clearExplicitUpdateFlag();
    /**
     * \brief Tell that this network type need an explicit update from the server
     */
    void needExplicitUpdate();
    bool isNeedingExplicitUpdate() const;

    void clearWaitingUpdateFlag();
    /**
     * \brief Tell that this network type is waiting for an update
     *
     * When a network type is waiting for an update, it will wait for the next update to be applied until
     * a certain delay (FGE_NET_WAITING_UPDATE_DELAY).
     * After this delay, the network type will automatically request an explicit update.
     *
     * When this method is called, the last update time is reset only if the network type is not already waiting.
     * \see setLastUpdateTime()
     */
    void waitingUpdate();
    bool isWaitingUpdate() const;

    [[nodiscard]] std::chrono::microseconds getLastUpdateTime() const;
    void setLastUpdateTime();

    /**
     * \brief Callback called when the value have been applied
     */
    fge::CallbackHandler<> _onApplied;

protected:
    virtual void createClientCustomData([[maybe_unused]] void*& ptr) const {}
    virtual void destroyClientCustomData([[maybe_unused]] void*& ptr) const {}
    virtual void applyClientCustomData([[maybe_unused]] void*& ptr) const {}

    fge::net::NetworkPerClientModificationTable _g_tableId;
    bool _g_needExplicitUpdate{false};
    bool _g_waitingUpdate{false};
    bool _g_force{false};
    std::chrono::microseconds _g_lastUpdateTime{0};
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
    NetworkType(fge::DataAccessor<T> source);
    ~NetworkType() override = default;

    void const* getSource() const override;

    bool applyData(fge::net::Packet const& pck) override;
    void packData(fge::net::Packet& pck, fge::net::Identity const& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

private:
    T g_typeCopy;
    fge::DataAccessor<T> g_typeSource;
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

    void const* getSource() const override;

    bool applyData(fge::net::Packet const& pck) override;
    void packData(fge::net::Packet& pck, fge::net::Identity const& id) override;
    void packData(fge::net::Packet& pck) override;

    bool clientsCheckup(fge::net::ClientList const& clients, bool force) override;

    bool checkClient(fge::net::Identity const& id) const override;
    void forceCheckClient(fge::net::Identity const& id) override;
    void forceUncheckClient(fge::net::Identity const& id) override;

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

    void const* getSource() const override;

    bool applyData(fge::net::Packet const& pck) override;
    void packData(fge::net::Packet& pck, fge::net::Identity const& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

private:
    fge::TagList* g_typeSource;
    std::string g_tag;
};

/**
 * \class NetworkTypeSmoothVec2Float
 * \ingroup network
 * \brief The network type for a vector2 float that wait for the error threshold in order to set the value
 * (useful for smooth position sync)
 */
class FGE_API NetworkTypeSmoothVec2Float : public NetworkTypeBase
{
public:
    NetworkTypeSmoothVec2Float(fge::DataAccessor<fge::Vector2f> source, float errorRange);
    ~NetworkTypeSmoothVec2Float() override = default;

    void const* getSource() const override;

    bool applyData(fge::net::Packet const& pck) override;
    void packData(fge::net::Packet& pck, fge::net::Identity const& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

    fge::Vector2f const& getCache() const;
    void setErrorRange(float range);
    float getErrorRange() const;

private:
    fge::Vector2f g_typeCopy;
    fge::DataAccessor<fge::Vector2f> g_typeSource;
    float g_errorRange;
};
/**
 * \class NetworkTypeSmoothFloat
 * \ingroup network
 * \brief The network type for a float that wait for the error threshold in order to set the value
 */
class FGE_API NetworkTypeSmoothFloat : public NetworkTypeBase
{
public:
    NetworkTypeSmoothFloat(fge::DataAccessor<float> source, float errorRange);
    ~NetworkTypeSmoothFloat() override = default;

    void const* getSource() const override;

    bool applyData(fge::net::Packet const& pck) override;
    void packData(fge::net::Packet& pck, fge::net::Identity const& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

    float getCache() const;
    void setErrorRange(float range);
    float getErrorRange() const;

private:
    float g_typeCopy;
    fge::DataAccessor<float> g_typeSource;
    float g_errorRange;
};

/**
 * \class NetworkTypeProperty
 * \ingroup network
 * \brief The network type for a property
 *
 * \tparam T The type of the property
 */
template<class T>
class NetworkTypeProperty : public NetworkTypeBase
{
public:
    NetworkTypeProperty(fge::Property* source);
    ~NetworkTypeProperty() override = default;

    void const* getSource() const override;

    bool applyData(fge::net::Packet const& pck) override;
    void packData(fge::net::Packet& pck, fge::net::Identity const& id) override;
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
template<class T>
class NetworkTypePropertyList : public NetworkTypeBase
{
public:
    NetworkTypePropertyList(fge::PropertyList* source, std::string const& vname);
    ~NetworkTypePropertyList() override = default;

    void const* getSource() const override;

    bool applyData(fge::net::Packet const& pck) override;
    void packData(fge::net::Packet& pck, fge::net::Identity const& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

    std::string const& getValueName() const;

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

    void const* getSource() const override;

    bool applyData(fge::net::Packet const& pck) override;
    void packData(fge::net::Packet& pck, fge::net::Identity const& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

    void trigger();

private:
    T* g_typeSource;
    bool g_trigger;
};

template<class T>
class RecordedVector;

template<class T>
Packet& operator<<(Packet& pck, RecordedVector<T> const& vec);
template<class T>
Packet const& operator>>(Packet const& pck, RecordedVector<T>& vec);

enum class RecordedEventTypes : uint8_t
{
    ADD,
    REMOVE,
    REMOVE_ALL,
    MODIFY
};
struct RecordedEvent
{
    RecordedEventTypes _type;
    SizeType _index;
};

inline Packet& operator<<(Packet& pck, RecordedEvent const& event);
inline Packet const& operator>>(Packet const& pck, RecordedEvent& event);

template<class T>
class RecordedVector
{
public:
    using const_iterator = typename std::vector<T>::const_iterator;
    using iterator = typename std::vector<T>::iterator;
    using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;
    using const_reference = typename std::vector<T>::const_reference;
    using reference = typename std::vector<T>::reference;

    using EventQueue = std::vector<RecordedEvent>;

    RecordedVector() = default;
    ~RecordedVector() = default;

    [[nodiscard]] const_reference at(SizeType index) const;
    [[nodiscard]] const_reference operator[](SizeType index) const;
    [[nodiscard]] const_reference front() const;
    [[nodiscard]] const_reference back() const;
    [[nodiscard]] T const* data() const;

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
    [[nodiscard]] const_iterator cbegin() const;
    [[nodiscard]] const_iterator cend() const;
    [[nodiscard]] const_reverse_iterator rbegin() const;
    [[nodiscard]] const_reverse_iterator rend() const;
    [[nodiscard]] const_reverse_iterator crbegin() const;
    [[nodiscard]] const_reverse_iterator crend() const;

    [[nodiscard]] SizeType size() const;
    [[nodiscard]] bool empty() const;

    void reserve(SizeType n);

    void clear();
    iterator insert(const_iterator pos, T const& value);
    iterator insert(const_iterator pos, T&& value);
    template<class... TArgs>
    iterator emplace(const_iterator pos, TArgs&&... value);
    template<class TArg>
    void push_back(TArg&& arg);
    template<class... TArgs>
    reference emplace_back(TArgs&&... arg);
    const_iterator erase(const_iterator pos);
    void pop_back();

    [[nodiscard]] reference modify(SizeType index);
    [[nodiscard]] reference modify(const_iterator pos);

    void clearEvents();
    [[nodiscard]] SizeType eventsSize() const;
    [[nodiscard]] EventQueue const& getEventQueue() const;
    [[nodiscard]] bool isRegisteringEvents() const;
    void registerEvents(bool enable);

private:
    void pushEvent(RecordedEvent event);

    std::vector<T> g_container;
    EventQueue g_events;
#ifdef FGE_DEF_SERVER
    bool g_registerEvents{true};
#else
    bool g_registerEvents{false};
#endif

    friend Packet& operator<< <T>(Packet& pck, RecordedVector const& vec);
    friend Packet const& operator>> <T>(Packet const& pck, RecordedVector& vec);
};

/**
 * \class NetworkTypeVector
 * \ingroup network
 * \brief The network type for a vector
 */
template<class T>
class NetworkTypeVector : public NetworkTypeBase
{
public:
    NetworkTypeVector(RecordedVector<T>* source);
    ~NetworkTypeVector() override;

    void const* getSource() const override;

    bool applyData(fge::net::Packet const& pck) override;
    void packData(fge::net::Packet& pck, fge::net::Identity const& id) override;
    void packData(fge::net::Packet& pck) override;

    void forceCheckClient(fge::net::Identity const& id) override;
    void forceUncheckClient(fge::net::Identity const& id) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

private:
    void createClientCustomData(void*& ptr) const override;
    void destroyClientCustomData(void*& ptr) const override;
    void applyClientCustomData(void*& ptr) const override;

    enum class PackTypes : uint8_t
    {
        FULL,
        PARTIAL
    };

    RecordedVector<T>* g_typeSource;
};

/**
 * \class NetworkTypeHandler
 * \ingroup network
 * \brief A regroupment of network types
 *
 * In order to synchronize a certain number of variables with the network, you can use a NetworkTypeHandler.
 * It will regroup all the network types and provide a way to update them all at once.
 */
class FGE_API NetworkTypeHandler
{
public:
    NetworkTypeHandler() = default;
    ~NetworkTypeHandler() = default;

    //Copy function that does nothing
    NetworkTypeHandler([[maybe_unused]] NetworkTypeHandler const& n) {}
    NetworkTypeHandler& operator=([[maybe_unused]] NetworkTypeHandler const& n) { return *this; }

    void clear();

    void clientsCheckup(fge::net::ClientList const& clients, bool force = false) const;
    void forceCheckClient(fge::net::Identity const& id) const;
    void forceUncheckClient(fge::net::Identity const& id) const;

    fge::net::NetworkTypeBase* push(std::unique_ptr<fge::net::NetworkTypeBase>&& newNet);
    template<class T, class... TArgs>
    T* push(TArgs&&... args)
    {
        static_assert(std::is_base_of_v<fge::net::NetworkTypeBase, T>, "T must inherit from fge::net::NetworkTypeBase");
        return static_cast<T*>(this->push(std::make_unique<T>(std::forward<TArgs>(args)...)));
    }
    template<class T, class... TArgs>
    fge::net::NetworkType<T>* pushTrivial(TArgs&&... args)
    {
        return static_cast<fge::net::NetworkType<T>*>(this->push(
                std::make_unique<fge::net::NetworkType<T>>(fge::DataAccessor<T>{std::forward<TArgs>(args)...})));
    }

    std::size_t packNeededUpdate(fge::net::Packet& pck) const;
    void unpackNeededUpdate(fge::net::Packet const& pck, fge::net::Identity const& id) const;

    [[nodiscard]] inline std::size_t size() const { return this->g_data.size(); }
    [[nodiscard]] inline fge::net::NetworkTypeBase* get(std::size_t index) const { return this->g_data[index].get(); }
    template<class T>
    [[nodiscard]] inline T* get(std::size_t index) const
    {
        static_assert(std::is_base_of_v<fge::net::NetworkTypeBase, T>, "T must inherit from fge::net::NetworkTypeBase");
        return static_cast<T*>(this->g_data[index].get());
    }
    [[nodiscard]] inline fge::net::NetworkTypeBase* operator[](std::size_t index) const
    {
        return this->g_data[index].get();
    }

private:
    std::vector<std::unique_ptr<fge::net::NetworkTypeBase>> g_data;
};

} // namespace net
} // namespace fge

#include "C_networkType.inl"

#endif // _FGE_C_NETWORKTYPE_HPP_INCLUDED
