namespace fge
{

///Property
Property::Property(const fge::Property& val) :
        g_type(val.g_type),
        g_isSigned(val.g_isSigned),
        g_isModified(true)
{
    switch (val.g_type)
    {
    case fge::Property::PTYPE_NULL:
        break;
    case fge::Property::PTYPE_STRING:
        this->g_data._ptr = new std::string( *reinterpret_cast<std::string*>(val.g_data._ptr) );
        break;
    case fge::Property::PTYPE_CLASS:
        this->g_data._ptr = reinterpret_cast<fge::PropertyClassWrapper*>(val.g_data._ptr)->copy();
        break;

    default:
        this->g_data = val.g_data;
        break;
    }
}
Property::Property(fge::Property&& val) noexcept :
        g_type(val.g_type),
        g_data(val.g_data),
        g_isSigned(val.g_isSigned),
        g_isModified(true)
{
    val.g_type = fge::Property::PTYPE_NULL;
}

template<class T,
        typename>
Property::Property(const T& val) :
        g_isModified(true)
{
    if constexpr (std::is_integral<std::remove_reference_t<T> >::value || std::is_enum<std::remove_reference_t<T> >::value)
    {
        this->g_type = fge::Property::PTYPE_INTEGERS;

        if constexpr ( std::is_signed<std::remove_reference_t<T> >::value )
        {
            this->g_data._i = static_cast<fge::PintType>(val);
            this->g_isSigned = true;
        }
        else
        {
            this->g_data._u = static_cast<fge::PuintType>(val);
            this->g_isSigned = false;
        }
    }
    else if constexpr ( std::is_floating_point<std::remove_reference_t<T> >::value )
    {
        if constexpr ( std::is_same<std::remove_reference_t<T>, float>::value )
        {
            this->g_type = fge::Property::PTYPE_FLOAT;
            this->g_data._f = val;
        }
        else
        {
            this->g_type = fge::Property::PTYPE_DOUBLE;
            this->g_data._d = static_cast<double>(val);
        }
    }
    else if constexpr ( std::is_same<std::remove_reference_t<T>, std::string>::value )
    {
        this->g_type = fge::Property::PTYPE_STRING;
        this->g_data._ptr = new std::string(val);
    }
    else if constexpr ( std::is_pointer<std::remove_reference_t<T> >::value )
    {
        this->g_type = fge::Property::PTYPE_POINTER;
        this->g_data._ptr = val;
    }
    else
    {
        this->g_type = fge::Property::PTYPE_CLASS;
        this->g_data._ptr = new fge::PropertyClassWrapperType<std::remove_reference_t<T> >(val);
    }
}
template<class T,
        typename>
Property::Property(T&& val) :
        g_isModified(true)
{
    if constexpr (std::is_integral<std::remove_reference_t<T> >::value || std::is_enum<std::remove_reference_t<T> >::value)
    {
        this->g_type = fge::Property::PTYPE_INTEGERS;

        if constexpr ( std::is_signed<std::remove_reference_t<T> >::value )
        {
            this->g_data._i = static_cast<fge::PintType>(val);
            this->g_isSigned = true;
        }
        else
        {
            this->g_data._u = static_cast<fge::PuintType>(val);
            this->g_isSigned = false;
        }
    }
    else if constexpr ( std::is_floating_point<std::remove_reference_t<T> >::value )
    {
        if constexpr ( std::is_same<std::remove_reference_t<T>, float>::value )
        {
            this->g_type = fge::Property::PTYPE_FLOAT;
            this->g_data._f = val;
        }
        else
        {
            this->g_type = fge::Property::PTYPE_DOUBLE;
            this->g_data._d = static_cast<double>(val);
        }
    }
    else if constexpr ( std::is_same<std::remove_reference_t<T>, std::string>::value )
    {
        this->g_type = fge::Property::PTYPE_STRING;
        this->g_data._ptr = new std::string(std::forward<T>(val));
    }
    else if constexpr ( std::is_pointer<std::remove_reference_t<T> >::value )
    {
        this->g_type = fge::Property::PTYPE_POINTER;
        this->g_data._ptr = val;
    }
    else
    {
        this->g_type = fge::Property::PTYPE_CLASS;
        this->g_data._ptr = new fge::PropertyClassWrapperType<std::remove_reference_t<T> >(std::forward<T>(val));
    }
}

Property::Property(const char* val) :
        g_type{fge::Property::PTYPE_STRING},
        g_isModified(true)
{
    this->g_data._ptr = new std::string(val);
}

Property::~Property()
{
    this->clear();
}

void Property::clear()
{
    if (this->g_type == fge::Property::PTYPE_STRING)
    {
        delete reinterpret_cast<std::string*>(this->g_data._ptr);
    }
    else if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        delete reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr);
    }

    this->g_type = fge::Property::PTYPE_NULL;
}

bool Property::operator== (const fge::Property& val) const
{
    if (this->g_type == val.g_type)
    {
        switch (this->g_type)
        {
        case PTYPE_NULL:
            return true;
            break;
        case PTYPE_INTEGERS:
            if (this->g_isSigned)
            {
                if (val.g_isSigned)
                {
                    return this->g_data._i == val.g_data._i;
                }
                else
                {
                    return this->g_data._i == val.g_data._u;
                }
            }
            else
            {
                if (val.g_isSigned)
                {
                    return this->g_data._u == val.g_data._i;
                }
                else
                {
                    return this->g_data._u == val.g_data._u;
                }
            }
            break;
        case PTYPE_FLOAT:
            return this->g_data._f == val.g_data._f;
            break;
        case PTYPE_DOUBLE:
            return this->g_data._d == val.g_data._d;
            break;
        case PTYPE_STRING:
            return *reinterpret_cast<std::string*>(this->g_data._ptr) == *reinterpret_cast<std::string*>(val.g_data._ptr);
            break;
        case PTYPE_POINTER:
            return this->g_data._ptr == val.g_data._ptr;
            break;
        case PTYPE_CLASS:
            return reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->compare( reinterpret_cast<fge::PropertyClassWrapper*>(val.g_data._ptr) );
            break;
        }
    }
    return false;
}

fge::Property& Property::operator= (const fge::Property& val)
{
    this->set(val);
    return *this;
}
fge::Property& Property::operator= (fge::Property&& val) noexcept
{
    this->set(std::move(val));
    return *this;
}

template<class T,
        typename>
fge::Property& Property::operator= (const T& val)
{
    this->set(std::forward<T>(val));
    return *this;
}
template<class T,
        typename>
fge::Property& Property::operator= (T&& val)
{
    this->set(std::forward<T>(val));
    return *this;
}

fge::Property& Property::operator= (const char* val)
{
    this->set(val);
    return *this;
}

template<class T>
T& Property::setType()
{
    if constexpr (std::is_integral<T>::value || std::is_enum<T>::value)
    {
        if (this->g_type != fge::Property::PTYPE_INTEGERS)
        {
            this->clear();
            this->g_type = fge::Property::PTYPE_INTEGERS;
        }

        if constexpr ( std::is_signed<T>::value )
        {
            this->g_isSigned = true;
            return this->g_data._i;
        }
        else
        {
            this->g_isSigned = true;
            return this->g_data._u;
        }
    }
    else if constexpr ( std::is_floating_point<T>::value )
    {
        if constexpr ( std::is_same<T, float>::value )
        {
            if (this->g_type != fge::Property::PTYPE_FLOAT)
            {
                this->clear();
                this->g_type = fge::Property::PTYPE_FLOAT;
            }

            return this->g_data._f;
        }
        else
        {
            if (this->g_type != fge::Property::PTYPE_DOUBLE)
            {
                this->clear();
                this->g_type = fge::Property::PTYPE_DOUBLE;
            }

            return this->g_data._d;
        }
    }
    else if constexpr ( std::is_same<T, std::string>::value )
    {
        if (this->g_type != fge::Property::PTYPE_STRING)
        {
            this->clear();
            this->g_type = fge::Property::PTYPE_STRING;
            this->g_data._ptr = new std::string();
        }

        return *reinterpret_cast<std::string*>(this->g_data._ptr);
    }
    else if constexpr ( std::is_pointer<T>::value )
    {
        if (this->g_type != fge::Property::PTYPE_POINTER)
        {
            this->clear();
            this->g_type = fge::Property::PTYPE_POINTER;
        }

        return reinterpret_cast<typename std::add_lvalue_reference<T>::type>(this->g_data._ptr);
    }
    else
    {
        if (this->g_type != fge::Property::PTYPE_CLASS)
        {
            this->clear();
            this->g_type = fge::Property::PTYPE_CLASS;
            this->g_data._ptr = new fge::PropertyClassWrapperType<std::remove_reference_t<T> >();
            return reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>* >(this->g_data._ptr)->_data;
        }
        else
        {
            if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T) )
            {
                return reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>* >(this->g_data._ptr)->_data;
            }
            else
            {
                this->clear();
                this->g_type = fge::Property::PTYPE_CLASS;
                this->g_data._ptr = new fge::PropertyClassWrapperType<std::remove_reference_t<T>>();
                return reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)->_data;
            }
        }
    }
}
void Property::setType(fge::Property::Types type)
{
    if (type == fge::Property::PTYPE_CLASS)
    {
        throw std::logic_error("Can't set arbitrary class type !");
    }

    if (type != this->g_type)
    {
        this->clear();
        if (type == fge::Property::PTYPE_STRING)
        {
            this->g_data._ptr = new std::string();
        }
        this->g_type = type;
    }
}
template<class T>
bool Property::isType() const
{
    if constexpr (std::is_integral<T>::value || std::is_enum<T>::value)
    {
        return this->g_type == fge::Property::PTYPE_INTEGERS;
    }
    else if constexpr ( std::is_floating_point<T>::value )
    {
        if constexpr ( std::is_same<T, float>::value )
        {
            return this->g_type == fge::Property::PTYPE_FLOAT;
        }
        else
        {
            return this->g_type == fge::Property::PTYPE_DOUBLE;
        }
    }
    else if constexpr ( std::is_same<T, std::string>::value )
    {
        return this->g_type == fge::Property::PTYPE_STRING;
    }
    else if constexpr ( std::is_pointer<T>::value )
    {
        return this->g_type == fge::Property::PTYPE_POINTER;
    }
    else
    {
        if (this->g_type == fge::Property::PTYPE_CLASS)
        {
            return reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T);
        }
        else
        {
            return false;
        }
    }
}
bool Property::isType(fge::Property::Types type) const
{
    return this->g_type == type;
}

const std::type_info& Property::getClassType() const
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        return reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType();
    }
    throw std::logic_error("Not a class type !");
}
Property::Types Property::getType() const
{
    return this->g_type;
}
bool Property::isSigned() const
{
    return this->g_isSigned;
}

std::string Property::toString() const
{
    switch (this->g_type)
    {
    case fge::Property::PTYPE_INTEGERS:
        if (this->g_isSigned)
        {
            return fge::string::ToStr(this->g_data._i);
        }
        else
        {
            return fge::string::ToStr(this->g_data._u);
        }
        break;
    case fge::Property::PTYPE_FLOAT:
        return fge::string::ToStr(this->g_data._f);
        break;
    case fge::Property::PTYPE_DOUBLE:
        return fge::string::ToStr(this->g_data._d);
        break;
    case fge::Property::PTYPE_STRING:
        return *reinterpret_cast<std::string*>(this->g_data._ptr);
        break;

    case fge::Property::PTYPE_POINTER:
        return fge::string::ToStr(this->g_data._ptr);
        break;
    case fge::Property::PTYPE_CLASS:
        return reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->toString();
        break;

    default:
        return {};
        break;
    }
}

bool Property::set(const fge::Property& val)
{
    if (this->g_type == val.g_type)
    {
        switch (val.g_type)
        {
        case fge::Property::PTYPE_NULL:
            break;
        case fge::Property::PTYPE_STRING:
            *reinterpret_cast<std::string*>(this->g_data._ptr) = *reinterpret_cast<std::string*>(val.g_data._ptr);
            this->g_isModified = true;
            break;
        case fge::Property::PTYPE_CLASS:
            if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->tryToCopy(reinterpret_cast<fge::PropertyClassWrapper*>(val.g_data._ptr)) )
            {
                this->g_isModified = true;
            }
            break;

        default:
            this->g_isSigned = val.g_isSigned;
            this->g_data = val.g_data;
            this->g_isModified = true;
            break;
        }

        return true;
    }
    else if (this->g_type == fge::Property::PTYPE_NULL)
    {
        this->g_isModified = true;
        this->g_type = val.g_type;
        switch (val.g_type)
        {
        case fge::Property::PTYPE_NULL:
            break;
        case fge::Property::PTYPE_STRING:
            this->g_data._ptr = new std::string(*reinterpret_cast<std::string*>(val.g_data._ptr));
            break;
        case fge::Property::PTYPE_CLASS:
            this->g_data._ptr = reinterpret_cast<fge::PropertyClassWrapper*>(val.g_data._ptr)->copy();
            break;

        default:
            this->g_isSigned = val.g_isSigned;
            this->g_data = val.g_data;
            break;
        }

        return true;
    }

    return false;
}
bool Property::set(fge::Property&& val)
{
    if (this->g_type == val.g_type)
    {
        switch (val.g_type)
        {
        case fge::Property::PTYPE_NULL:
            break;
        case fge::Property::PTYPE_STRING:
        case fge::Property::PTYPE_CLASS:
            this->g_data._ptr = val.g_data._ptr;
            this->g_isModified = true;
            break;

        default:
            this->g_isSigned = val.g_isSigned;
            this->g_data = val.g_data;
            this->g_isModified = true;
            break;
        }

        val.g_type = fge::Property::PTYPE_NULL;
        return true;
    }
    else if (this->g_type == fge::Property::PTYPE_NULL)
    {
        this->g_isModified = true;
        this->g_type = val.g_type;
        switch (val.g_type)
        {
        case fge::Property::PTYPE_NULL:
            break;
        case fge::Property::PTYPE_STRING:
        case fge::Property::PTYPE_CLASS:
            this->g_data._ptr = val.g_data._ptr;
            break;

        default:
            this->g_isSigned = val.g_isSigned;
            this->g_data = val.g_data;
            break;
        }

        val.g_type = fge::Property::PTYPE_NULL;
        return true;
    }

    return false;
}

template<class T,
        typename>
bool Property::set(const T& val)
{
    if constexpr (std::is_integral<std::remove_reference_t<T> >::value || std::is_enum<std::remove_reference_t<T> >::value)
    {
        if (this->g_type != fge::Property::PTYPE_INTEGERS)
        {
            if (this->g_type == fge::Property::PTYPE_NULL)
            {
                this->g_type = fge::Property::PTYPE_INTEGERS;
            }
            else
            {
                return false;
            }
        }

        if constexpr ( std::is_signed<std::remove_reference_t<T> >::value )
        {
            this->g_data._i = static_cast<fge::PintType>(val);
            this->g_isSigned = true;
            return true;
        }
        else
        {
            this->g_data._u = static_cast<fge::PuintType>(val);
            this->g_isSigned = false;
            return true;
        }
    }
    else if constexpr ( std::is_floating_point<std::remove_reference_t<T> >::value )
    {
        if constexpr ( std::is_same<std::remove_reference_t<T>, float>::value )
        {
            if (this->g_type != fge::Property::PTYPE_FLOAT)
            {
                if (this->g_type == fge::Property::PTYPE_NULL)
                {
                    this->g_type = fge::Property::PTYPE_FLOAT;
                }
                else
                {
                    return false;
                }
            }

            this->g_data._f = val;
            return true;
        }
        else
        {
            if (this->g_type != fge::Property::PTYPE_DOUBLE)
            {
                if (this->g_type == fge::Property::PTYPE_NULL)
                {
                    this->g_type = fge::Property::PTYPE_DOUBLE;
                }
                else
                {
                    return false;
                }
            }

            this->g_data._d = static_cast<double>(val);
            return true;
        }
    }
    else if constexpr ( std::is_same<std::remove_reference_t<T>, std::string>::value )
    {
        if (this->g_type != fge::Property::PTYPE_STRING)
        {
            if (this->g_type == fge::Property::PTYPE_NULL)
            {
                this->g_type = fge::Property::PTYPE_STRING;
                this->g_data._ptr = new std::string(val);
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            *reinterpret_cast<std::string*>(this->g_data._ptr) = val;
            return true;
        }
    }
    else if constexpr ( std::is_pointer<std::remove_reference_t<T> >::value )
    {
        if (this->g_type != fge::Property::PTYPE_POINTER)
        {
            if (this->g_type == fge::Property::PTYPE_NULL)
            {
                this->g_type = fge::Property::PTYPE_POINTER;
            }
            else
            {
                return false;
            }
        }

        this->g_data._ptr = val;
        return true;
    }
    else
    {
        if (this->g_type != fge::Property::PTYPE_CLASS)
        {
            if (this->g_type == fge::Property::PTYPE_NULL)
            {
                this->g_type = fge::Property::PTYPE_CLASS;
                this->g_data._ptr = new fge::PropertyClassWrapperType<std::remove_reference_t<T> >(val);
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(std::remove_reference_t<T>) )
            {
                reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T> >* >(this->g_data._ptr)->_data = val;
                return true;
            }
            return false;
        }
    }
}
template<class T,
        typename>
bool Property::set(T&& val)
{
    if constexpr (std::is_integral<std::remove_reference_t<T> >::value || std::is_enum<std::remove_reference_t<T> >::value)
    {
        if (this->g_type != fge::Property::PTYPE_INTEGERS)
        {
            if (this->g_type == fge::Property::PTYPE_NULL)
            {
                this->g_type = fge::Property::PTYPE_INTEGERS;
            }
            else
            {
                return false;
            }
        }

        if constexpr ( std::is_signed<std::remove_reference_t<T> >::value )
        {
            this->g_data._i = static_cast<fge::PintType>(val);
            this->g_isSigned = true;
            return true;
        }
        else
        {
            this->g_data._u = static_cast<fge::PuintType>(val);
            this->g_isSigned = false;
            return true;
        }
    }
    else if constexpr ( std::is_floating_point<std::remove_reference_t<T> >::value )
    {
        if constexpr ( std::is_same<std::remove_reference_t<T>, float>::value )
        {
            if (this->g_type != fge::Property::PTYPE_FLOAT)
            {
                if (this->g_type == fge::Property::PTYPE_NULL)
                {
                    this->g_type = fge::Property::PTYPE_FLOAT;
                }
                else
                {
                    return false;
                }
            }

            this->g_data._f = val;
            return true;
        }
        else
        {
            if (this->g_type != fge::Property::PTYPE_DOUBLE)
            {
                if (this->g_type == fge::Property::PTYPE_NULL)
                {
                    this->g_type = fge::Property::PTYPE_DOUBLE;
                }
                else
                {
                    return false;
                }
            }

            this->g_data._d = static_cast<double>(val);
            return true;
        }
    }
    else if constexpr ( std::is_same<std::remove_reference_t<T>, std::string>::value )
    {
        if (this->g_type != fge::Property::PTYPE_STRING)
        {
            if (this->g_type == fge::Property::PTYPE_NULL)
            {
                this->g_type = fge::Property::PTYPE_STRING;
                this->g_data._ptr = new std::string( std::forward<T>(val) );
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            *reinterpret_cast<std::string*>(this->g_data._ptr) = std::forward<T>(val);
            return true;
        }
    }
    else if constexpr ( std::is_pointer<std::remove_reference_t<T> >::value )
    {
        if (this->g_type != fge::Property::PTYPE_POINTER)
        {
            if (this->g_type == fge::Property::PTYPE_NULL)
            {
                this->g_type = fge::Property::PTYPE_POINTER;
            }
            else
            {
                return false;
            }
        }

        this->g_data._ptr = val;
        return true;
    }
    else
    {
        if (this->g_type != fge::Property::PTYPE_CLASS)
        {
            if (this->g_type == fge::Property::PTYPE_NULL)
            {
                this->g_type = fge::Property::PTYPE_CLASS;
                this->g_data._ptr = new fge::PropertyClassWrapperType<std::remove_reference_t<T>>(std::forward<T>(val));
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(std::remove_reference_t<T>) )
            {
                reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T> >* >(this->g_data._ptr)->_data = std::forward<T>(val);
                return true;
            }
            return false;
        }
    }
}

bool Property::set(const char* val)
{
    if (this->g_type != fge::Property::PTYPE_STRING)
    {
        if (this->g_type == fge::Property::PTYPE_NULL)
        {
            this->g_type = fge::Property::PTYPE_STRING;
            this->g_data._ptr = new std::string(val);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        *reinterpret_cast<std::string*>(this->g_data._ptr) = val;
        return true;
    }
}

template<class T>
bool Property::get(T& val) const
{
    if constexpr (std::is_integral<T>::value || std::is_enum<T>::value)
    {
        if (this->g_type != Property::PTYPE_INTEGERS)
        {
            if (this->g_type == Property::PTYPE_FLOAT)
            {
                val = static_cast<T>(this->g_data._f);
                return true;
            }
            else if (this->g_type == Property::PTYPE_DOUBLE)
            {
                val = static_cast<T>(this->g_data._d);
                return true;
            }
            else
            {
                return false;
            }
        }

        if constexpr ( std::is_signed<T>::value )
        {
            val = static_cast<T>(this->g_data._i);
            return true;
        }
        else
        {
            val = static_cast<T>(this->g_data._u);
            return true;
        }
    }
    else if constexpr ( std::is_floating_point<T>::value )
    {
        if constexpr ( std::is_same<T, float>::value )
        {
            if (this->g_type != Property::PTYPE_FLOAT)
            {
                if (this->g_type == Property::PTYPE_INTEGERS)
                {
                    if (this->g_isSigned)
                    {
                        val = static_cast<T>(this->g_data._i);
                        return true;
                    }
                    else
                    {
                        val = static_cast<T>(this->g_data._u);
                        return true;
                    }
                }
                else if (this->g_type == Property::PTYPE_DOUBLE)
                {
                    val = static_cast<T>(this->g_data._d);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            val = static_cast<T>(this->g_data._f);
            return true;
        }
        else
        {
            if (this->g_type != Property::PTYPE_DOUBLE)
            {
                if (this->g_type == Property::PTYPE_INTEGERS)
                {
                    if (this->g_isSigned)
                    {
                        val = static_cast<T>(this->g_data._i);
                        return true;
                    }
                    else
                    {
                        val = static_cast<T>(this->g_data._u);
                        return true;
                    }
                }
                else if (this->g_type == Property::PTYPE_FLOAT)
                {
                    val = static_cast<T>(this->g_data._f);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            val = static_cast<T>(this->g_data._d);
            return true;
        }
    }
    else if constexpr ( std::is_same<T, std::string>::value )
    {
        if (this->g_type != fge::Property::PTYPE_STRING)
        {
            return false;
        }

        val = *reinterpret_cast<std::string*>(this->g_data._ptr);
        return true;
    }
    else if constexpr ( std::is_same<T, const char*>::value )
    {
        if (this->g_type != fge::Property::PTYPE_STRING)
        {
            return false;
        }

        val = reinterpret_cast<std::string*>(this->g_data._ptr)->data();
        return true;
    }
    else if constexpr ( std::is_pointer<T>::value )
    {
        if (this->g_type != fge::Property::PTYPE_POINTER)
        {
            return false;
        }

        val = reinterpret_cast<T>(this->g_data._ptr);
        return true;
    }
    else
    {
        if (this->g_type == fge::Property::PTYPE_CLASS)
        {
            if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T) )
            {
                val = reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)->_data;
                return true;
            }
        }

        return false;
    }
}
template<class T>
T Property::get() const
{
    if constexpr (std::is_integral<T>::value || std::is_enum<T>::value)
    {
        if (this->g_type != Property::PTYPE_INTEGERS)
        {
            if (this->g_type == Property::PTYPE_FLOAT)
            {
                return static_cast<T>(this->g_data._f);
            }
            else if (this->g_type == Property::PTYPE_DOUBLE)
            {
                return static_cast<T>(this->g_data._d);
            }
            else
            {
                return static_cast<T>(0);
            }
        }

        if constexpr ( std::is_signed<T>::value )
        {
            return static_cast<T>(this->g_data._i);
        }
        else
        {
            return static_cast<T>(this->g_data._u);
        }
    }
    else if constexpr ( std::is_floating_point<T>::value )
    {
        if constexpr ( std::is_same<T, float>::value )
        {
            if (this->g_type != Property::PTYPE_FLOAT)
            {
                if (this->g_type == Property::PTYPE_INTEGERS)
                {
                    if (this->g_isSigned)
                    {
                        return static_cast<T>(this->g_data._i);
                    }
                    else
                    {
                        return static_cast<T>(this->g_data._u);
                    }
                }
                else if (this->g_type == Property::PTYPE_DOUBLE)
                {
                    return static_cast<T>(this->g_data._d);
                }
                else
                {
                    return 0.0f;
                }
            }

            return static_cast<T>(this->g_data._f);
        }
        else
        {
            if (this->g_type != Property::PTYPE_DOUBLE)
            {
                if (this->g_type == Property::PTYPE_INTEGERS)
                {
                    if (this->g_isSigned)
                    {
                        return static_cast<T>(this->g_data._i);
                    }
                    else
                    {
                        return static_cast<T>(this->g_data._u);
                    }
                }
                else if (this->g_type == Property::PTYPE_FLOAT)
                {
                    return static_cast<T>(this->g_data._f);
                }
                else
                {
                    return 0.0;
                }
            }

            return static_cast<T>(this->g_data._d);
        }
    }
    else if constexpr ( std::is_same<T, std::string>::value )
    {
        if (this->g_type != fge::Property::PTYPE_STRING)
        {
            return {};
        }

        return *reinterpret_cast<std::string*>(this->g_data._ptr);
    }
    else if constexpr ( std::is_same<T, const char*>::value )
    {
        if (this->g_type != fge::Property::PTYPE_STRING)
        {
            return nullptr;
        }

        return reinterpret_cast<std::string*>(this->g_data._ptr)->data();
    }
    else if constexpr ( std::is_pointer<T>::value )
    {
        if (this->g_type != fge::Property::PTYPE_POINTER)
        {
            return nullptr;
        }

        return reinterpret_cast<T>(this->g_data._ptr);
    }
    else
    {
        if (this->g_type == fge::Property::PTYPE_CLASS)
        {
            if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T) )
            {
                return reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)->_data;
            }
        }

        throw std::logic_error("not implemented !");
    }
}

template<class T>
T* Property::getPtr()
{
    if constexpr (std::is_integral<T>::value || std::is_enum<T>::value)
    {
        if constexpr ( sizeof(T) != sizeof(fge::PintType) )
        {
            return nullptr;
        }

        if (this->g_type != Property::PTYPE_INTEGERS)
        {
            return nullptr;
        }

        if constexpr ( std::is_signed<T>::value )
        {
            return static_cast<T*>( &this->g_data._i );
        }
        else
        {
            return static_cast<T*>( &this->g_data._u );
        }
    }
    else if constexpr ( std::is_floating_point<T>::value )
    {
        if constexpr ( std::is_same<T, float>::value )
        {
            if (this->g_type != Property::PTYPE_FLOAT)
            {
                return nullptr;
            }

            return &this->g_data._f;
        }
        else
        {
            if (this->g_type != Property::PTYPE_DOUBLE)
            {
                return nullptr;
            }

            return &this->g_data._d;
        }
    }
    else if constexpr ( std::is_same<T, std::string>::value )
    {
        if (this->g_type != fge::Property::PTYPE_STRING)
        {
            return nullptr;
        }

        return reinterpret_cast<std::string*>(this->g_data._ptr);
    }
    else if constexpr ( std::is_pointer<T>::value )
    {
        if (this->g_type != fge::Property::PTYPE_POINTER)
        {
            return nullptr;
        }

        return &reinterpret_cast<T>(this->g_data._ptr);
    }
    else
    {
        if (this->g_type == fge::Property::PTYPE_CLASS)
        {
            if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T) )
            {
                return &reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T> >* >(this->g_data._ptr)->_data;
            }
        }

        return nullptr;
    }
}
template<class T>
const T* Property::getPtr() const
{
    if constexpr (std::is_integral<T>::value || std::is_enum<T>::value)
    {
        if constexpr ( sizeof(T) != sizeof(fge::PintType) )
        {
            return nullptr;
        }

        if (this->g_type != Property::PTYPE_INTEGERS)
        {
            return nullptr;
        }

        if constexpr ( std::is_signed<T>::value )
        {
            return static_cast<T*>( &this->g_data._i );
        }
        else
        {
            return static_cast<T*>( &this->g_data._u );
        }
    }
    else if constexpr ( std::is_floating_point<T>::value )
    {
        if constexpr ( std::is_same<T, float>::value )
        {
            if (this->g_type != Property::PTYPE_FLOAT)
            {
                return nullptr;
            }

            return &this->g_data._f;
        }
        else
        {
            if (this->g_type != Property::PTYPE_DOUBLE)
            {
                return nullptr;
            }

            return &this->g_data._d;
        }
    }
    else if constexpr ( std::is_same<T, std::string>::value )
    {
        if (this->g_type != fge::Property::PTYPE_STRING)
        {
            return nullptr;
        }

        return reinterpret_cast<std::string*>(this->g_data._ptr);
    }
    else if constexpr ( std::is_pointer<T>::value )
    {
        if (this->g_type != fge::Property::PTYPE_POINTER)
        {
            return nullptr;
        }

        return &reinterpret_cast<T>(this->g_data._ptr);
    }
    else
    {
        if (this->g_type == fge::Property::PTYPE_CLASS)
        {
            if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T) )
            {
                return &reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T> >* >(this->g_data._ptr)->_data;
            }
        }

        return nullptr;
    }
}

fge::ParrayType& Property::setArrayType()
{
    if (this->g_type != fge::Property::PTYPE_CLASS)
    {
        this->clear();
        this->g_type = fge::Property::PTYPE_CLASS;
        this->g_data._ptr = new fge::PropertyClassWrapperType<fge::ParrayType>();
    }
    else
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() != typeid(fge::ParrayType) )
        {
            this->clear();
            this->g_type = fge::Property::PTYPE_CLASS;
            this->g_data._ptr = new fge::PropertyClassWrapperType<fge::ParrayType>();
        }
    }

    return reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data;
}

bool Property::resize(std::size_t n)
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data.resize(n);
            return true;
        }
    }
    return false;
}
bool Property::reserve(std::size_t n)
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data.reserve(n);
            return true;
        }
    }
    return false;
}

bool Property::pushData(const fge::Property& value)
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data.emplace_back(value);
            return true;
        }
    }
    return false;
}
bool Property::pushData(fge::Property&& value)
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data.emplace_back(std::move(value));
            return true;
        }
    }
    return false;
}

template<class T>
bool Property::pushType()
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data.emplace_back().setType<T>();
            return true;
        }
    }
    return false;
}

bool Property::setData(std::size_t index, const fge::Property& value)
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data[index] = value;
            return true;
        }
    }
    return false;
}
bool Property::setData(std::size_t index, fge::Property&& value)
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data[index] = std::move(value);
            return true;
        }
    }
    return false;
}

fge::Property* Property::getData(std::size_t index)
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            if ( reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data.size() > index )
            {
                return &reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data[index];
            }
        }
    }
    return nullptr;
}
const fge::Property* Property::getData(std::size_t index) const
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            if ( reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data.size() > index )
            {
                return &reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data[index];
            }
        }
    }
    return nullptr;
}

template<class T>
bool Property::getData(std::size_t index, T& val) const
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            if ( reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data.size() > index )
            {
                return reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data[index].get<T>(val);
            }
        }
    }
    return false;
}
template<class T>
T* Property::getDataPtr(std::size_t index)
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            if ( reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data.size() > index )
            {
                return reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data[index].getPtr<T>();
            }
        }
    }
    return nullptr;
}
template<class T>
const T* Property::getDataPtr(std::size_t index) const
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            if ( reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data.size() > index )
            {
                return reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data[index].getPtr<T>();
            }
        }
    }
    return nullptr;
}

std::size_t Property::getDataSize() const
{
    if (this->g_type == fge::Property::PTYPE_CLASS)
    {
        if ( reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType) )
        {
            return reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>* >(this->g_data._ptr)->_data.size();
        }
    }
    return 0;
}

fge::Property* Property::operator[] (std::size_t index)
{
    return this->getData(index);
}
const fge::Property* Property::operator[] (std::size_t index) const
{
    return this->getData(index);
}

bool Property::isModified() const
{
    return this->g_isModified;
}
void Property::setModifiedFlag(bool flag)
{
    this->g_isModified = flag;
}

///PropertyClassWrapperType

template<class T>
PropertyClassWrapperType<T>::PropertyClassWrapperType(T val) :
    _data(std::move(val))
{
}

template<class T>
const std::type_info& PropertyClassWrapperType<T>::getType() const
{
    return typeid(T);
}

template<class T>
std::string PropertyClassWrapperType<T>::toString() const
{
    if constexpr ( std::is_same<T, fge::ParrayType>::value )
    {
        return fge::string::ToStr(this->_data);
    }
    else
    {
        return typeid(T).name();
    }
}

template<class T>
fge::PropertyClassWrapper* PropertyClassWrapperType<T>::copy() const
{
    return new fge::PropertyClassWrapperType<T>(this->_data);
}

template<class T>
bool PropertyClassWrapperType<T>::tryToCopy(const fge::PropertyClassWrapper* val)
{
    if (val->getType() == typeid(T))
    {
        this->_data = reinterpret_cast<const PropertyClassWrapperType<T>*>(val)->_data;
        return true;
    }
    return false;
}

template<class T>
bool PropertyClassWrapperType<T>::compare(const fge::PropertyClassWrapper* val)
{
    if (val->getType() == typeid(T))
    {
        return this->_data == reinterpret_cast<const PropertyClassWrapperType<T>*>(val)->_data;
    }
    return false;
}

}//end fge
