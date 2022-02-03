namespace fge
{

///ValueObj

template <class T>
ValueObj<T>::ValueObj(T& val) : _data(val)
{
}
template <class T>
ValueObj<T>::ValueObj(const T& val) : _data(val)
{
}
template <class T>
ValueObj<T>::ValueObj(T&& val) : _data(std::move(val))
{
}
template <class T>
ValueObj<T>::ValueObj(const T&& val) : _data(std::move(val))
{
}

template <class T>
ValueObj<T>::ValueObj(const fge::ValueObj<T>& val) : _data(val._data)
{
}
template <class T>
ValueObj<T>::ValueObj(fge::ValueObj<T>&& val) : _data(std::move(val._data))
{
}

template <class T>
fge::ValueObj<T>& ValueObj<T>::operator= (const fge::ValueObj<T>& val)
{
    this->_data = val._data;
}
template <class T>
fge::ValueObj<T>& ValueObj<T>::operator= (fge::ValueObj<T>&& val)
{
    this->_data = std::move(val._data);
}

template <class T>
std::string ValueObj<T>::toString() const
{
    if constexpr (std::is_arithmetic<T>::value || std::is_pointer<T>::value)
    {
        return fge::string::ToStr(this->_data);
    }
    else
    {
        return "";
    }
}
template <>
inline std::string ValueObj<const char*>::toString() const
{
    return std::string(this->_data);
}
template <>
inline std::string ValueObj<std::string>::toString() const
{
    return this->_data;
}
template <>
inline std::string ValueObj<fge::ValueArray>::toString() const
{
    return fge::string::ToStr(this->_data);
}
template <>
inline std::string ValueObj<sf::Vector2f>::toString() const
{
    return fge::string::ToStr(this->_data);
}
template <>
inline std::string ValueObj<sf::Vector2u>::toString() const
{
    return fge::string::ToStr(this->_data);
}
template <>
inline std::string ValueObj<sf::Vector2i>::toString() const
{
    return fge::string::ToStr(this->_data);
}
template <>
inline std::string ValueObj<sf::Vector3f>::toString() const
{
    return fge::string::ToStr(this->_data);
}
template <>
inline std::string ValueObj<sf::Vector3i>::toString() const
{
    return fge::string::ToStr(this->_data);
}

template <class T>
const std::type_info& ValueObj<T>::getType() const
{
    return typeid(T);
}
template <class T>
const std::type_info& ValueObj<T>::GetType()
{
    return typeid(T);
}

template <class T>
fge::ValueObjBase* ValueObj<T>::copy() const
{
    return new fge::ValueObj<T>(this->_data);
}

template <class T>
bool ValueObj<T>::tryToSet(const fge::ValueObjBase* val)
{
    if ( typeid(T) == val->getType() )
    {
        this->_data = static_cast<const fge::ValueObj<T>* >( val )->_data;
        return true;
    }
    return false;
}

template <class T>
bool ValueObj<T>::tryToSet(void* val, const std::type_info& type, bool isArithmetic, bool isFloatingPoint, bool isSigned)
{
    if (typeid(T) == type)
    {
        this->_data = *static_cast<T*>(val);
        return true;
    }

    if constexpr (std::is_arithmetic<T>::value)
    {
        if (isArithmetic)
        {
            if (isFloatingPoint)
            {
                if (type == typeid(float))
                {
                    this->_data = static_cast<T>( *reinterpret_cast<float*>(val) );
                    return true;
                }
                else if (type == typeid(double))
                {
                    this->_data = static_cast<T>( *reinterpret_cast<double*>(val) );
                    return true;
                }
                else if (type == typeid(long double))
                {
                    this->_data = static_cast<T>( *reinterpret_cast<long double*>(val) );
                    return true;
                }
            }
            else
            {
                if (isSigned)
                {
                    if (type == typeid(char))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<char*>(val) );
                        return true;
                    }
                    else if (type == typeid(signed char))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<signed char*>(val) );
                        return true;
                    }
                    else if (type == typeid(short int))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<short int*>(val) );
                        return true;
                    }
                    else if (type == typeid(int))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<int*>(val) );
                        return true;
                    }
                    else if (type == typeid(long int))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<long int*>(val) );
                        return true;
                    }
                    else if (type == typeid(long long int))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<long long int*>(val) );
                        return true;
                    }
                }
                else
                {
                    if (type == typeid(bool))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<bool*>(val) );
                        return true;
                    }
                    else if (type == typeid(char16_t))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<char16_t*>(val) );
                        return true;
                    }
                    else if (type == typeid(char32_t))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<char32_t*>(val) );
                        return true;
                    }
                    else if (type == typeid(wchar_t))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<wchar_t*>(val) );
                        return true;
                    }
                    else if (type == typeid(unsigned char))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<unsigned char*>(val) );
                        return true;
                    }
                    else if (type == typeid(unsigned short int))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<unsigned short int*>(val) );
                        return true;
                    }
                    else if (type == typeid(unsigned int))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<unsigned int*>(val) );
                        return true;
                    }
                    else if (type == typeid(unsigned long int))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<unsigned long int*>(val) );
                        return true;
                    }
                    else if (type == typeid(unsigned long long int))
                    {
                        this->_data = static_cast<T>( *reinterpret_cast<unsigned long long int*>(val) );
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

template <class T>
bool ValueObj<T>::operator== (const fge::ValueObjBase& value) const
{
    if ( typeid(T) == value.getType() )
    {
        return this->_data == static_cast<const fge::ValueObj<T>& >(value)._data;
    }
    return false;
}

template <class T>
fge::ValueObj<T>* ValueObj<T>::CastPtr(fge::ValueObjBase* n)
{
    return static_cast<fge::ValueObj<T>* >(n);
}

///Value

//Copy/Move some type constructor
template<typename T>
Value::Value(T& val) :
    g_valueObj( new fge::ValueObj<T>(val) ),
    g_isModified(true)
{
}
template<typename T>
Value::Value(const T& val) :
    g_valueObj( new fge::ValueObj<T>(val) ),
    g_isModified(true)
{
}
template<typename T>
Value::Value(T&& val) :
    g_valueObj( new fge::ValueObj<T>(std::move(val)) ),
    g_isModified(true)
{
}
template<typename T>
Value::Value(const T&& val) :
    g_valueObj( new fge::ValueObj<T>(std::move(val)) ),
    g_isModified(true)
{
}

//Copy/Move some type operator
template<typename T>
fge::Value& Value::operator= (T& val)
{
    if ( this->g_valueObj != nullptr )
    {
        this->g_isModified |= this->g_valueObj.get()->tryToSet(&val, typeid(val), std::is_arithmetic<T>::value,
                                                                                 std::is_floating_point<T>::value,
                                                                                 std::is_signed<T>::value);
    }
    else
    {
        this->g_valueObj.reset( new fge::ValueObj<T>(val) );
        this->g_isModified = true;
    }
    return *this;
}
template<typename T>
fge::Value& Value::operator= (const T& val)
{
    if ( this->g_valueObj != nullptr )
    {
        this->g_isModified |= this->g_valueObj.get()->tryToSet(&val, typeid(val), std::is_arithmetic<T>::value,
                                                                                    std::is_floating_point<T>::value,
                                                                                    std::is_signed<T>::value);
    }
    else
    {
        this->g_valueObj.reset( new fge::ValueObj<T>(val) );
        this->g_isModified = true;
    }
    return *this;
}
template<typename T>
fge::Value& Value::operator= (T&& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->getType() == typeid(T) )
        {
            fge::ValueObj<T>::CastPtr(this->g_valueObj.get())->_data = std::move(val);
            this->g_isModified = true;
        }
        else
        {
            this->g_isModified |= this->g_valueObj.get()->tryToSet(&val, typeid(val), std::is_arithmetic<T>::value,
                                                                                    std::is_floating_point<T>::value,
                                                                                    std::is_signed<T>::value);
        }
    }
    else
    {
        this->g_valueObj.reset( new fge::ValueObj<T>(std::move(val)) );
        this->g_isModified = true;
    }
    return *this;
}
template<typename T>
fge::Value& Value::operator= (const T&& val)
{
    if ( this->g_valueObj != nullptr )
    {
        this->g_isModified |= this->g_valueObj.get()->tryToSet(&val, typeid(val), std::is_arithmetic<T>::value,
                                                                                    std::is_floating_point<T>::value,
                                                                                    std::is_signed<T>::value);
    }
    else
    {
        this->g_valueObj.reset( new fge::ValueObj<T>(val) );
        this->g_isModified = true;
    }
    return *this;
}

template<typename T>
bool Value::getData(std::size_t index, T& valBuff) const
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueArray& buff = fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data;
        if ( index < buff.size() )
        {
            return buff[index].get<T>(valBuff);
        }
    }
    return false;
}
template<typename T>
const T* Value::getData(std::size_t index) const
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueArray& buff = fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data;
        if ( index < buff.size() )
        {
            return buff[index].get<T>();
        }
    }
    return nullptr;
}
template<typename T>
T* Value::getData(std::size_t index)
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueArray& buff = fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data;
        if ( index < buff.size() )
        {
            return buff[index].get<T>();
        }
    }
    return nullptr;
}

const std::type_info& Value::getType() const
{
    return (this->g_valueObj==nullptr) ? typeid(nullptr) : this->g_valueObj.get()->getType();
}

template<typename T>
bool Value::addType()
{
    fge::Value value;
    value.setType<T>();
    return this->addData(std::move(value));
}

template<typename T>
T& Value::setType()
{
    if ( this->getType() != typeid(T) )
    {
        this->g_valueObj.reset( new fge::ValueObj<T>() );
        this->g_isModified = true;
    }
    return fge::ValueObj<T>::CastPtr( this->g_valueObj.get() )->_data;
}

template<typename T>
bool Value::set(T& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->g_valueObj.get()->tryToSet(&val, typeid(val), std::is_arithmetic<T>::value,
                                                                    std::is_floating_point<T>::value,
                                                                    std::is_signed<T>::value) )
        {
            this->g_isModified = true;
            return true;
        }
    }
    else
    {
        this->g_valueObj.reset( new fge::ValueObj<T>(val) );
        this->g_isModified = true;
        return true;
    }
    return false;
}
template<typename T>
bool Value::set(const T& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->g_valueObj.get()->tryToSet(&val, typeid(val), std::is_arithmetic<T>::value,
                                                                    std::is_floating_point<T>::value,
                                                                    std::is_signed<T>::value) )
        {
            this->g_isModified = true;
            return true;
        }
    }
    else
    {
        this->g_valueObj.reset( new fge::ValueObj<T>(val) );
        this->g_isModified = true;
        return true;
    }
    return false;
}
template<typename T>
bool Value::set(T&& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->getType() == typeid(T) )
        {
            fge::ValueObj<T>::CastPtr(this->g_valueObj.get())->_data = std::move(val);
            this->g_isModified = true;
            return true;
        }
        else if ( this->g_valueObj.get()->tryToSet(&val, typeid(val), std::is_arithmetic<T>::value,
                                                                     std::is_floating_point<T>::value,
                                                                     std::is_signed<T>::value) )
        {
            this->g_isModified = true;
            return true;
        }
    }
    else
    {
        this->g_valueObj.reset( new fge::ValueObj<T>(std::move(val)) );
        this->g_isModified = true;
        return true;
    }
    return false;
}
template<typename T>
bool Value::set(const T&& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->g_valueObj.get()->tryToSet(&val, typeid(val), std::is_arithmetic<T>::value,
                                                                 std::is_floating_point<T>::value,
                                                                 std::is_signed<T>::value) )
        {
            this->g_isModified = true;
            return true;
        }
    }
    else
    {
        this->g_valueObj.reset( new fge::ValueObj<T>(val) );
        this->g_isModified = true;
        return true;
    }
    return false;
}

template<typename T>
bool Value::get(T& valBuff) const
{
    if ( this->getType() == typeid(T) )
    {
        valBuff = fge::ValueObj<T>::CastPtr(this->g_valueObj.get())->_data;
        return true;
    }
    return false;
}
template<typename T>
T* Value::get()
{
    if ( this->getType() == typeid(T) )
    {
        return &fge::ValueObj<T>::CastPtr(this->g_valueObj.get())->_data;
    }
    return nullptr;
}
template<typename T>
const T* Value::get() const
{
    if ( this->getType() == typeid(T) )
    {
        return &fge::ValueObj<T>::CastPtr(this->g_valueObj.get())->_data;
    }
    return nullptr;
}

bool Value::isModified() const
{
    return this->g_isModified;
}
void Value::setModifiedFlag(bool flag)
{
    this->g_isModified = flag;
}

}//end fge
