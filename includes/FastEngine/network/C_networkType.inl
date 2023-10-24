/*
 * Copyright 2023 Guillaume Guillet
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
        it->second = false;
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
NetworkTypeProperty<T>::NetworkTypeProperty(fge::Property* source)
{
    this->g_typeSource = source;
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

        it->second &= ~fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK;
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
NetworkTypePropertyList<T>::NetworkTypePropertyList(fge::PropertyList* source, std::string const& vname)
{
    this->g_typeSource = source;
    this->g_vname = vname;
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

        it->second &= ~fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK;
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
NetworkTypeManual<T>::NetworkTypeManual(T* source)
{
    this->g_typeSource = source;
    this->g_trigger = false;
}

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
        it->second &= ~fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK;
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

} // namespace fge::net
