namespace fge
{
namespace net
{

///NetworkType

template<class T>
NetworkType<T>::NetworkType(T* source) :
    g_typeCopy(*source),
    g_typeSource(source),
    g_force(false)
{
}

template<class T>
void* NetworkType<T>::getSource() const
{
    return this->g_typeSource;
}
template<class T>
bool NetworkType<T>::applyData(fge::net::Packet& pck)
{
    if ( pck >> this->g_typeCopy )
    {
        *this->g_typeSource = this->g_typeCopy;
        this->_onApplied.call();
        return true;
    }
    return false;
}
template<class T>
void NetworkType<T>::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    fge::net::NetworkPerClientModificationTable::iterator it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << *this->g_typeSource;
        it->second = false;
    }
}
template<class T>
void NetworkType<T>::packData(fge::net::Packet& pck)
{
    pck << *this->g_typeSource;
}
template<class T>
bool NetworkType<T>::check() const
{
    return ((*this->g_typeSource) != this->g_typeCopy) || this->g_force;
}
template<class T>
void NetworkType<T>::forceCheck()
{
    this->g_force = true;
}
template<class T>
void NetworkType<T>::forceUncheck()
{
    this->g_force = false;
    this->g_typeCopy = *this->g_typeSource;
}

///NetworkTypeValue

template<class T>
NetworkTypeValue<T>::NetworkTypeValue(fge::Value* source)
{
    this->g_typeSource = source;

    if ( source->getType() != typeid(T) )
    {//not the same type
        source->setType<T>();
    }
}

template<class T>
void* NetworkTypeValue<T>::getSource() const
{
    return this->g_typeSource;
}

template <class T>
bool NetworkTypeValue<T>::applyData(fge::net::Packet& pck)
{
    if ( this->g_typeSource->getType() != typeid(T) )
    {//not the same type
        pck >> this->g_typeSource->setType<T>();
    }
    else
    {
        pck >> *this->g_typeSource->get<T>();
    }

    this->_onApplied.call();
    return true;
}
template <class T>
void NetworkTypeValue<T>::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    fge::net::NetworkPerClientModificationTable::iterator it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        if ( this->g_typeSource->getType() != typeid(T) )
        {//not the same type
            pck << this->g_typeSource->setType<T>();
        }
        else
        {
            pck << *this->g_typeSource->get<T>();
        }

        it->second = false;
    }
}
template <class T>
void NetworkTypeValue<T>::packData(fge::net::Packet& pck)
{
    if ( this->g_typeSource->getType() != typeid(T) )
    {//not the same type
        pck << this->g_typeSource->setType<T>();
    }
    else
    {
        pck << *this->g_typeSource->get<T>();
    }
}

template <class T>
bool NetworkTypeValue<T>::check() const
{
    return this->g_typeSource->isModified();
}
template <class T>
void NetworkTypeValue<T>::forceCheck()
{
    this->g_typeSource->setModifiedFlag(true);
}
template <class T>
void NetworkTypeValue<T>::forceUncheck()
{
    this->g_typeSource->setModifiedFlag(false);
}

///NetworkTypeDataList
template <class T>
NetworkTypeDataList<T>::NetworkTypeDataList(fge::ValueList* source, const std::string& vname)
{
    this->g_typeSource = source;
    this->g_vname = vname;
    fge::Value* buffer = &source->getValue(vname);

    if ( buffer->getType() != typeid(T) )
    {//not the same type
        buffer->setType<T>();
    }
}

template <class T>
void* NetworkTypeDataList<T>::getSource() const
{
    return this->g_typeSource;
}

template <class T>
bool NetworkTypeDataList<T>::applyData(fge::net::Packet& pck)
{
    fge::Value& value = this->g_typeSource->getValue(this->g_vname);

    if ( value.getType() != typeid(T) )
    {//not the same type
        pck >> value.setType<T>();
    }
    else
    {
        pck >> *value.get<T>();
    }

    this->_onApplied.call();
    return false;
}
template <class T>
void NetworkTypeDataList<T>::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    fge::net::NetworkPerClientModificationTable::iterator it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        fge::Value& value = this->g_typeSource->getValue(this->g_vname);

        if ( value.getType() != typeid(T) )
        {//not the same type
            pck << value.setType<T>();
        }
        else
        {
            pck << *value.get<T>();
        }

        it->second = false;
    }
}
template <class T>
void NetworkTypeDataList<T>::packData(fge::net::Packet& pck)
{
    fge::Value& value = this->g_typeSource->getValue(this->g_vname);

    if ( value.getType() != typeid(T) )
    {//not the same type
        pck << value.setType<T>();
    }
    else
    {
        pck << *value.get<T>();
    }
}

template <class T>
bool NetworkTypeDataList<T>::check() const
{
    return this->g_typeSource->getValue(this->g_vname).isModified();
}
template <class T>
void NetworkTypeDataList<T>::forceCheck()
{
    this->g_typeSource->getValue(this->g_vname).setModifiedFlag(true);
}
template <class T>
void NetworkTypeDataList<T>::forceUncheck()
{
    this->g_typeSource->getValue(this->g_vname).setModifiedFlag(false);
}

template <class T>
const std::string& NetworkTypeDataList<T>::getValueName() const
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
void* NetworkTypeManual<T>::getSource() const
{
    return this->g_typeSource;
}

template<class T>
bool NetworkTypeManual<T>::applyData(fge::net::Packet& pck)
{
    if ( pck >> *this->g_typeSource )
    {
        this->_onApplied.call();
        return true;
    }
    return false;
}
template<class T>
void NetworkTypeManual<T>::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    fge::net::NetworkPerClientModificationTable::iterator it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << *this->g_typeSource;
        it->second = false;
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

}//end net
}//end fge
