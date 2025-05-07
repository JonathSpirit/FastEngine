/*
 * Copyright 2025 Guillaume Guillet
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

namespace fge::net
{

///NetworkType

template<class T>
NetworkType<T>::NetworkType(fge::DataAccessor<T> source) :
        g_typeCopy(source._getter()),
        g_typeSource(std::move(source))
{}

template<class T>
void const* NetworkType<T>::getSource() const
{
    return &this->g_typeSource;
}
template<class T>
bool NetworkType<T>::applyData(fge::net::Packet const& pck)
{
    if (pck >> this->g_typeCopy)
    {
        this->g_typeSource._setter(this->g_typeCopy);
        this->setLastUpdateTime();
        this->clearWaitingUpdateFlag();
        this->_onApplied.call();
        return true;
    }
    return false;
}
template<class T>
void NetworkType<T>::packData(fge::net::Packet& pck, fge::net::Identity const& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << this->g_typeSource._getter();
        it->second._config &= ~fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
    }
}
template<class T>
void NetworkType<T>::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource._getter();
}
template<class T>
bool NetworkType<T>::check() const
{
    return (this->g_typeSource._getter() != this->g_typeCopy) || this->_g_force;
}
template<class T>
void NetworkType<T>::forceCheck()
{
    this->_g_force = true;
}
template<class T>
void NetworkType<T>::forceUncheck()
{
    this->_g_force = false;
    this->g_typeCopy = this->g_typeSource._getter();
}

///NetworkTypeProperty

template<class T>
NetworkTypeProperty<T>::NetworkTypeProperty(fge::Property* source) :
        g_typeSource(source)
{
    source->setType<T>();
}

template<class T>
void const* NetworkTypeProperty<T>::getSource() const
{
    return this->g_typeSource;
}

template<class T>
bool NetworkTypeProperty<T>::applyData(fge::net::Packet const& pck)
{
    pck >> this->g_typeSource->template setType<T>();

    this->setLastUpdateTime();
    this->clearWaitingUpdateFlag();
    this->_onApplied.call();
    return true;
}
template<class T>
void NetworkTypeProperty<T>::packData(fge::net::Packet& pck, fge::net::Identity const& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << this->g_typeSource->template setType<T>();

        it->second._config &= ~fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
    }
}
template<class T>
void NetworkTypeProperty<T>::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource->template setType<T>();
}

template<class T>
bool NetworkTypeProperty<T>::check() const
{
    return this->g_typeSource->isModified();
}
template<class T>
void NetworkTypeProperty<T>::forceCheck()
{
    this->g_typeSource->setModifiedFlag(true);
}
template<class T>
void NetworkTypeProperty<T>::forceUncheck()
{
    this->g_typeSource->setModifiedFlag(false);
}

///NetworkTypePropertyList
template<class T>
NetworkTypePropertyList<T>::NetworkTypePropertyList(fge::PropertyList* source, std::string const& vname) :
        g_typeSource(source),
        g_vname(vname)
{
    fge::Property& property = source->getProperty(vname);
    property.setType<T>();
}

template<class T>
void const* NetworkTypePropertyList<T>::getSource() const
{
    return this->g_typeSource;
}

template<class T>
bool NetworkTypePropertyList<T>::applyData(fge::net::Packet const& pck)
{
    fge::Property& property = this->g_typeSource->getProperty(this->g_vname);

    pck >> property.setType<T>();

    this->setLastUpdateTime();
    this->clearWaitingUpdateFlag();
    this->_onApplied.call();
    return false;
}
template<class T>
void NetworkTypePropertyList<T>::packData(fge::net::Packet& pck, fge::net::Identity const& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        fge::Property& property = this->g_typeSource->getProperty(this->g_vname);

        pck << property.setType<T>();

        it->second._config &= ~fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
    }
}
template<class T>
void NetworkTypePropertyList<T>::packData(fge::net::Packet& pck)
{
    fge::Property& property = this->g_typeSource->getProperty(this->g_vname);

    pck << property.setType<T>();
}

template<class T>
bool NetworkTypePropertyList<T>::check() const
{
    return this->g_typeSource->getProperty(this->g_vname).isModified();
}
template<class T>
void NetworkTypePropertyList<T>::forceCheck()
{
    this->g_typeSource->getProperty(this->g_vname).setModifiedFlag(true);
}
template<class T>
void NetworkTypePropertyList<T>::forceUncheck()
{
    this->g_typeSource->getProperty(this->g_vname).setModifiedFlag(false);
}

template<class T>
std::string const& NetworkTypePropertyList<T>::getValueName() const
{
    return this->g_vname;
}

///NetworkTypeManual
template<class T>
NetworkTypeManual<T>::NetworkTypeManual(T* source) :
        g_typeSource(source),
        g_trigger(false)
{}

template<class T>
void const* NetworkTypeManual<T>::getSource() const
{
    return this->g_typeSource;
}

template<class T>
bool NetworkTypeManual<T>::applyData(fge::net::Packet const& pck)
{
    if (pck >> *this->g_typeSource)
    {
        this->setLastUpdateTime();
        this->clearWaitingUpdateFlag();
        this->_onApplied.call();
        return true;
    }
    return false;
}
template<class T>
void NetworkTypeManual<T>::packData(fge::net::Packet& pck, fge::net::Identity const& id)
{
    fge::net::NetworkPerClientModificationTable::iterator it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << *this->g_typeSource;
        it->second._config &= ~fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
    }
}
template<class T>
void NetworkTypeManual<T>::packData(fge::net::Packet& pck)
{
    pck << *this->g_typeSource;
}

template<class T>
bool NetworkTypeManual<T>::check() const
{
    return this->g_trigger;
}
template<class T>
void NetworkTypeManual<T>::forceCheck()
{
    this->g_trigger = true;
}
template<class T>
void NetworkTypeManual<T>::forceUncheck()
{
    this->g_trigger = false;
}

template<class T>
void NetworkTypeManual<T>::trigger()
{
    this->g_trigger = true;
}

//RecordedVector

template<class T>
typename RecordedVector<T>::const_reference RecordedVector<T>::at(SizeType index) const
{
    return this->g_container.at(index);
}
template<class T>
typename RecordedVector<T>::const_reference RecordedVector<T>::operator[](SizeType index) const
{
    return this->g_container[index];
}
template<class T>
typename RecordedVector<T>::const_reference RecordedVector<T>::front() const
{
    return this->g_container.front();
}
template<class T>
typename RecordedVector<T>::const_reference RecordedVector<T>::back() const
{
    return this->g_container.back();
}
template<class T>
T const* RecordedVector<T>::data() const
{
    return this->g_container.data();
}

template<class T>
typename RecordedVector<T>::const_iterator RecordedVector<T>::begin() const
{
    return this->g_container.begin();
}
template<class T>
typename RecordedVector<T>::const_iterator RecordedVector<T>::end() const
{
    return this->g_container.end();
}
template<class T>
typename RecordedVector<T>::const_iterator RecordedVector<T>::cbegin() const
{
    return this->g_container.cbegin();
}
template<class T>
typename RecordedVector<T>::const_iterator RecordedVector<T>::cend() const
{
    return this->g_container.cend();
}
template<class T>
typename RecordedVector<T>::const_reverse_iterator RecordedVector<T>::rbegin() const
{
    return this->g_container.rbegin();
}
template<class T>
typename RecordedVector<T>::const_reverse_iterator RecordedVector<T>::rend() const
{
    return this->g_container.rend();
}
template<class T>
typename RecordedVector<T>::const_reverse_iterator RecordedVector<T>::crbegin() const
{
    return this->g_container.crbegin();
}
template<class T>
typename RecordedVector<T>::const_reverse_iterator RecordedVector<T>::crend() const
{
    return this->g_container.crend();
}

template<class T>
SizeType RecordedVector<T>::size() const
{
    return static_cast<SizeType>(this->g_container.size());
}
template<class T>
bool RecordedVector<T>::empty() const
{
    return this->g_container.empty();
}

template<class T>
void RecordedVector<T>::reserve(SizeType n)
{
    this->g_container.reserve(n);
}

template<class T>
void RecordedVector<T>::clear()
{
    this->g_container.clear();
    this->clearEvents();
    this->pushEvent({RecordedEventTypes::REMOVE_ALL, 0});
}
template<class T>
typename RecordedVector<T>::iterator RecordedVector<T>::insert(const_iterator pos, T const& value)
{
    SizeType index = pos - this->g_container.begin();
    this->pushEvent({RecordedEventTypes::ADD, index});
    return this->g_container.insert(pos, value);
}
template<class T>
typename RecordedVector<T>::iterator RecordedVector<T>::insert(const_iterator pos, T&& value)
{
    SizeType index = pos - this->g_container.begin();
    this->pushEvent({RecordedEventTypes::ADD, index});
    return this->g_container.insert(pos, std::move(value));
}
template<class T>
template<class... TArgs>
typename RecordedVector<T>::iterator RecordedVector<T>::emplace(const_iterator pos, TArgs&&... value)
{
    SizeType index = pos - this->g_container.begin();
    this->pushEvent({RecordedEventTypes::ADD, index});
    return this->g_container.emplace(pos, std::forward<TArgs>(value)...);
}
template<class T>
template<class TArg>
void RecordedVector<T>::push_back(TArg&& arg)
{
    this->pushEvent({RecordedEventTypes::ADD, this->g_container.size()});
    this->g_container.push_back(std::forward<TArg>(arg));
}
template<class T>
template<class... TArgs>
typename RecordedVector<T>::reference RecordedVector<T>::emplace_back(TArgs&&... arg)
{
    this->pushEvent({RecordedEventTypes::ADD, this->g_container.size()});
    return this->g_container.emplace_back(std::forward<TArgs>(arg)...);
}
template<class T>
typename RecordedVector<T>::const_iterator RecordedVector<T>::erase(const_iterator pos)
{
    SizeType index = pos - this->g_container.begin();
    this->pushEvent({RecordedEventTypes::REMOVE, index});
    return this->g_container.erase(pos);
}
template<class T>
void RecordedVector<T>::pop_back()
{
    this->pushEvent({RecordedEventTypes::REMOVE, this->g_container.size() - 1});
    this->g_container.pop_back();
}

template<class T>
typename RecordedVector<T>::reference RecordedVector<T>::modify(SizeType index)
{
    this->pushEvent({RecordedEventTypes::MODIFY, index});
    return this->g_container[index];
}
template<class T>
typename RecordedVector<T>::reference RecordedVector<T>::modify(const_iterator pos)
{
    SizeType index = pos - this->g_container.begin();
    this->pushEvent({RecordedEventTypes::MODIFY, index});
    return this->g_container[index];
}

template<class T>
void RecordedVector<T>::clearEvents()
{
    this->g_events.clear();
}
template<class T>
SizeType RecordedVector<T>::eventsSize() const
{
    return static_cast<SizeType>(this->g_events.size());
}
template<class T>
typename RecordedVector<T>::EventQueue const& RecordedVector<T>::getEventQueue() const
{
    return this->g_events;
}

template<class T>
bool RecordedVector<T>::isRegisteringEvents() const
{
    return this->g_registerEvents;
}
template<class T>
void RecordedVector<T>::registerEvents(bool enable)
{
    this->clearEvents();
    this->g_registerEvents = enable;
}

template<class T>
void RecordedVector<T>::pushEvent(RecordedEvent event)
{
    if (this->g_registerEvents)
    {
        this->g_events.emplace_back(event);
    }
}

template<class T>
Packet& operator<<(Packet& pck, RecordedVector<T> const& vec)
{
    return pck << vec.g_container;
}
template<class T>
Packet const& operator>>(Packet const& pck, RecordedVector<T>& vec)
{
    return pck >> vec.g_container;
}
inline Packet& operator<<(Packet& pck, RecordedEvent const& event)
{
    return pck << event._type << event._index;
}
inline Packet const& operator>>(Packet const& pck, RecordedEvent& event)
{
    return pck >> event._type >> event._index;
}

///NetworkTypeVector

template<class T>
NetworkTypeVector<T>::NetworkTypeVector(RecordedVector<T>* source) :
        g_typeSource(source)
{}
template<class T>
NetworkTypeVector<T>::~NetworkTypeVector()
{
    for (auto& it: this->_g_tableId)
    {
        this->destroyClientCustomData(it.second._customData);
    }
}

template<class T>
void const* NetworkTypeVector<T>::getSource() const
{
    return this->g_typeSource;
}

template<class T>
bool NetworkTypeVector<T>::applyData(fge::net::Packet const& pck)
{
    PackTypes packType;
    pck >> packType;

    if (packType == PackTypes::FULL)
    {
        pck >> *this->g_typeSource;
    }
    else
    {
        SizeType eventCount{0};
        pck >> eventCount;
        for (SizeType i = 0; i < eventCount; ++i)
        {
            RecordedEvent event{};
            pck >> event;
            switch (event._type)
            {
            case RecordedEventTypes::ADD:
                if (event._index >= this->g_typeSource->size())
                {
                    T value;
                    pck >> value;
                    this->g_typeSource->emplace_back(std::move(value));
                }
                else
                {
                    T value;
                    pck >> value;
                    this->g_typeSource->emplace(this->g_typeSource->begin() + event._index, std::move(value));
                }
                break;
            case RecordedEventTypes::REMOVE:
                if (event._index < this->g_typeSource->size())
                {
                    this->g_typeSource->erase(this->g_typeSource->begin() + event._index);
                }
                else
                {
                    return false; ///TODO: handle error
                }
                break;
            case RecordedEventTypes::REMOVE_ALL:
                this->g_typeSource->clear();
                break;
            case RecordedEventTypes::MODIFY:
                if (event._index < this->g_typeSource->size())
                {
                    pck >> this->g_typeSource->modify(event._index);
                }
                else
                {
                    return false; ///TODO: handle error
                }
                break;
            }
        }
    }

    this->setLastUpdateTime();
    this->clearWaitingUpdateFlag();
    this->_onApplied.call();
    return true;
}
template<class T>
void NetworkTypeVector<T>::packData(fge::net::Packet& pck, fge::net::Identity const& id)
{
    auto itId = this->_g_tableId.find(id);
    if (itId != this->_g_tableId.end())
    {
        auto* events = static_cast<typename RecordedVector<T>::EventQueue*>(itId->second._customData);

        if (events->empty())
        { //Events queue is empty but the modified flag is set, that means we have to send the full vector
            pck << PackTypes::FULL;
            pck << *this->g_typeSource;
        }
        else
        {
            pck << PackTypes::PARTIAL;
            pck << static_cast<SizeType>(events->size());
            for (auto itEvent = events->begin(); itEvent != events->end(); ++itEvent)
            {
                pck << *itEvent;
                if (itEvent->_type == RecordedEventTypes::ADD || itEvent->_type == RecordedEventTypes::MODIFY)
                {
                    //We have to pack the data but the index do not represent the current state of the vector.
                    //So we have to reverse read the events (history) to get the correct index.
                    auto finalIndex = itEvent->_index;
                    for (auto itReverse = events->rbegin(); &(*itReverse) != &(*itEvent); ++itReverse)
                    {
                        if (itReverse->_type == RecordedEventTypes::ADD && itReverse->_index <= finalIndex)
                        {
                            ++finalIndex;
                        }
                        else if (itReverse->_type == RecordedEventTypes::REMOVE && itReverse->_index < finalIndex)
                        {
                            --finalIndex;
                        }
                    }
                    pck << this->g_typeSource->at(finalIndex);
                }
            }
            events->clear();
        }

        itId->second._config &= ~fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
    }
}
template<class T>
void NetworkTypeVector<T>::packData(fge::net::Packet& pck)
{
    pck << PackTypes::FULL << *this->g_typeSource;
}

template<class T>
void NetworkTypeVector<T>::forceCheckClient(fge::net::Identity const& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second._config |= fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
        auto* events = static_cast<typename RecordedVector<T>::EventQueue*>(it->second._customData);
        events->clear();
    }
}
template<class T>
void NetworkTypeVector<T>::forceUncheckClient(fge::net::Identity const& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second._config &= ~fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
        auto* events = static_cast<typename RecordedVector<T>::EventQueue*>(it->second._customData);
        events->clear();
    }
}

template<class T>
bool NetworkTypeVector<T>::check() const
{
    if (this->_g_force)
    { //When events is cleared, we send the full vector
        this->g_typeSource->clearEvents();
    }
    return this->g_typeSource->eventsSize() != 0 || this->_g_force;
}
template<class T>
void NetworkTypeVector<T>::forceCheck()
{
    this->_g_force = true;
}
template<class T>
void NetworkTypeVector<T>::forceUncheck()
{
    this->_g_force = false;
    this->g_typeSource->clearEvents();
}

template<class T>
void NetworkTypeVector<T>::createClientCustomData(void*& ptr) const
{
    ptr = new typename RecordedVector<T>::EventQueue();
}
template<class T>
void NetworkTypeVector<T>::destroyClientCustomData(void*& ptr) const
{
    delete static_cast<typename RecordedVector<T>::EventQueue*>(ptr);
}
template<class T>
void NetworkTypeVector<T>::applyClientCustomData(void*& ptr) const
{
    auto* events = static_cast<typename RecordedVector<T>::EventQueue*>(ptr);
    bool clearFirst = this->g_typeSource->getEventQueue().front()._type == RecordedEventTypes::REMOVE_ALL;
    if (clearFirst)
    {
        events->clear();
    }
    for (auto const& event: this->g_typeSource->getEventQueue())
    {
        events->push_back(event);
    }
}


} // namespace fge::net
