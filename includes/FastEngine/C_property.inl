/*
 * Copyright 2026 Guillaume Guillet
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

namespace fge
{

template<class T, typename>
Property::Property(T&& val) :
        g_isModified(true)
{
    using TT = remove_cvref_t<T>;

    if constexpr (std::is_integral_v<TT> || std::is_enum_v<TT>)
    {
        this->g_type = Types::PTYPE_INTEGERS;

        if constexpr (std::is_signed_v<TT>)
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
    else if constexpr (std::is_floating_point_v<TT>)
    {
        if constexpr (std::is_same_v<TT, float>)
        {
            this->g_type = Types::PTYPE_FLOAT;
            this->g_data._f = val;
        }
        else
        {
            this->g_type = Types::PTYPE_DOUBLE;
            this->g_data._d = static_cast<double>(val);
        }
    }
    else if constexpr (std::is_same_v<TT, std::string>)
    {
        this->g_type = Types::PTYPE_STRING;
        this->g_data._ptr = new std::string(std::forward<T>(val));
    }
    else if constexpr (std::is_pointer_v<TT>)
    {
        this->g_type = Types::PTYPE_POINTER;
        this->g_data._ptr = val;
    }
    else
    {
        this->g_type = Types::PTYPE_CLASS;
        this->g_data.setClassWrapper(new fge::PropertyClassWrapperType<TT>(std::forward<T>(val)));
    }
}

template<class T, typename>
fge::Property& Property::operator=(T&& val)
{
    this->set(std::forward<T>(val));
    return *this;
}

template<class T>
T& Property::setType()
{
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
    {
        if (this->g_type != Types::PTYPE_INTEGERS)
        {
            this->clear();
            this->g_type = Types::PTYPE_INTEGERS;
        }

        if constexpr (std::is_signed_v<T>)
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
    else if constexpr (std::is_floating_point_v<T>)
    {
        if constexpr (std::is_same_v<T, float>)
        {
            if (this->g_type != Types::PTYPE_FLOAT)
            {
                this->clear();
                this->g_type = Types::PTYPE_FLOAT;
            }

            return this->g_data._f;
        }
        else
        {
            if (this->g_type != Types::PTYPE_DOUBLE)
            {
                this->clear();
                this->g_type = Types::PTYPE_DOUBLE;
            }

            return this->g_data._d;
        }
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        if (this->g_type != Types::PTYPE_STRING)
        {
            this->clear();
            this->g_type = Types::PTYPE_STRING;
            this->g_data._ptr = new std::string();
        }

        return *this->g_data.getString();
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        if (this->g_type != Types::PTYPE_POINTER)
        {
            this->clear();
            this->g_type = Types::PTYPE_POINTER;
        }

        return reinterpret_cast<std::add_lvalue_reference_t<T>>(this->g_data._ptr);
    }
    else
    {
        using TT = remove_cvref_t<T>;

        if (this->g_type != Types::PTYPE_CLASS)
        {
            this->clear();
            this->g_type = Types::PTYPE_CLASS;
            this->g_data.setClassWrapper(new fge::PropertyClassWrapperType<TT>());
            return this->g_data.getClassWrapper<TT>()->_data;
        }

        if (this->g_data.getClassWrapper()->getType() == typeid(T))
        {
            return this->g_data.getClassWrapper<TT>()->_data;
        }

        this->clear();
        this->g_type = Types::PTYPE_CLASS;
        this->g_data.setClassWrapper(new fge::PropertyClassWrapperType<TT>());
        return this->g_data.getClassWrapper<TT>()->_data;
    }
}

template<class T>
bool Property::isType() const
{
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
    {
        return this->g_type == Types::PTYPE_INTEGERS;
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return this->g_type == Types::PTYPE_FLOAT;
        }
        else
        {
            return this->g_type == Types::PTYPE_DOUBLE;
        }
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        return this->g_type == Types::PTYPE_STRING;
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        return this->g_type == Types::PTYPE_POINTER;
    }
    else
    {
        if (this->g_type == Types::PTYPE_CLASS)
        {
            return this->g_data.getClassWrapper()->getType() == typeid(T);
        }
        return false;
    }
}

template<class T, typename>
bool Property::set(T&& val)
{
    using TT = remove_cvref_t<T>;

    if constexpr (std::is_integral_v<TT> || std::is_enum_v<TT>)
    {
        if (this->g_type != Types::PTYPE_INTEGERS)
        {
            if (this->g_type == Types::PTYPE_NULL)
            {
                this->g_type = Types::PTYPE_INTEGERS;
            }
            else
            {
                return false;
            }
        }

        if constexpr (std::is_signed_v<TT>)
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
    else if constexpr (std::is_floating_point_v<TT>)
    {
        if constexpr (std::is_same_v<TT, float>)
        {
            if (this->g_type != Types::PTYPE_FLOAT)
            {
                if (this->g_type == Types::PTYPE_NULL)
                {
                    this->g_type = Types::PTYPE_FLOAT;
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
            if (this->g_type != Types::PTYPE_DOUBLE)
            {
                if (this->g_type == Types::PTYPE_NULL)
                {
                    this->g_type = Types::PTYPE_DOUBLE;
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
    else if constexpr (std::is_same_v<TT, std::string>)
    {
        if (this->g_type != Types::PTYPE_STRING)
        {
            if (this->g_type == Types::PTYPE_NULL)
            {
                this->g_type = Types::PTYPE_STRING;
                this->g_data._ptr = new std::string(std::forward<T>(val));
                return true;
            }
            return false;
        }

        *this->g_data.getString() = std::forward<T>(val);
        return true;
    }
    else if constexpr (std::is_pointer_v<TT>)
    {
        if (this->g_type != Types::PTYPE_POINTER)
        {
            if (this->g_type == Types::PTYPE_NULL)
            {
                this->g_type = Types::PTYPE_POINTER;
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
        if (this->g_type != Types::PTYPE_CLASS)
        {
            if (this->g_type == Types::PTYPE_NULL)
            {
                this->g_type = Types::PTYPE_CLASS;
                this->g_data.setClassWrapper(new fge::PropertyClassWrapperType<TT>(std::forward<T>(val)));
                return true;
            }
            return false;
        }

        if (this->g_data.getClassWrapper()->getType() == typeid(TT))
        {
            this->g_data.getClassWrapper<TT>()->_data = std::forward<T>(val);
            return true;
        }
        return false;
    }
}

template<class T>
bool Property::get(T& val) const
{
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
    {
        if (this->g_type != Types::PTYPE_INTEGERS)
        {
            if (this->g_type == Types::PTYPE_FLOAT)
            {
                val = static_cast<T>(this->g_data._f);
                return true;
            }
            if (this->g_type == Types::PTYPE_DOUBLE)
            {
                val = static_cast<T>(this->g_data._d);
                return true;
            }
            return false;
        }

        if constexpr (std::is_signed_v<T>)
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
    else if constexpr (std::is_floating_point_v<T>)
    {
        if constexpr (std::is_same_v<T, float>)
        {
            if (this->g_type != Types::PTYPE_FLOAT)
            {
                if (this->g_type == Types::PTYPE_INTEGERS)
                {
                    if (this->g_isSigned)
                    {
                        val = static_cast<T>(this->g_data._i);
                        return true;
                    }
                    val = static_cast<T>(this->g_data._u);
                    return true;
                }
                if (this->g_type == Types::PTYPE_DOUBLE)
                {
                    val = static_cast<T>(this->g_data._d);
                    return true;
                }
                return false;
            }

            val = static_cast<T>(this->g_data._f);
            return true;
        }
        else
        {
            if (this->g_type != Types::PTYPE_DOUBLE)
            {
                if (this->g_type == Types::PTYPE_INTEGERS)
                {
                    if (this->g_isSigned)
                    {
                        val = static_cast<T>(this->g_data._i);
                        return true;
                    }
                    val = static_cast<T>(this->g_data._u);
                    return true;
                }
                if (this->g_type == Types::PTYPE_FLOAT)
                {
                    val = static_cast<T>(this->g_data._f);
                    return true;
                }
                return false;
            }

            val = static_cast<T>(this->g_data._d);
            return true;
        }
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        if (this->g_type != Types::PTYPE_STRING)
        {
            return false;
        }

        val = *this->g_data.getString();
        return true;
    }
    else if constexpr (std::is_same_v<T, char const*>)
    {
        if (this->g_type != Types::PTYPE_STRING)
        {
            return false;
        }

        val = this->g_data.getString()->data();
        return true;
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        if (this->g_type != Types::PTYPE_POINTER)
        {
            return false;
        }

        val = reinterpret_cast<T>(this->g_data._ptr);
        return true;
    }
    else
    {
        if (this->g_type == Types::PTYPE_CLASS)
        {
            if (this->g_data.getClassWrapper()->getType() == typeid(T))
            {
                using TT = remove_cvref_t<T>;
                val = this->g_data.getClassWrapper<TT>()->_data;
                return true;
            }
        }

        return false;
    }
}
template<class T>
std::optional<T> Property::get() const
{
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
    {
        if (this->g_type != Types::PTYPE_INTEGERS)
        {
            if (this->g_type == Types::PTYPE_FLOAT)
            {
                return static_cast<T>(this->g_data._f);
            }
            if (this->g_type == Types::PTYPE_DOUBLE)
            {
                return static_cast<T>(this->g_data._d);
            }
            return std::nullopt;
        }

        if constexpr (std::is_signed_v<T>)
        {
            return static_cast<T>(this->g_data._i);
        }
        else
        {
            return static_cast<T>(this->g_data._u);
        }
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        if constexpr (std::is_same_v<T, float>)
        {
            if (this->g_type != Types::PTYPE_FLOAT)
            {
                if (this->g_type == Types::PTYPE_INTEGERS)
                {
                    if (this->g_isSigned)
                    {
                        return static_cast<T>(this->g_data._i);
                    }
                    return static_cast<T>(this->g_data._u);
                }
                if (this->g_type == Types::PTYPE_DOUBLE)
                {
                    return static_cast<T>(this->g_data._d);
                }
                return std::nullopt;
            }

            return static_cast<T>(this->g_data._f);
        }
        else
        {
            if (this->g_type != Types::PTYPE_DOUBLE)
            {
                if (this->g_type == Types::PTYPE_INTEGERS)
                {
                    if (this->g_isSigned)
                    {
                        return static_cast<T>(this->g_data._i);
                    }
                    return static_cast<T>(this->g_data._u);
                }
                if (this->g_type == Types::PTYPE_FLOAT)
                {
                    return static_cast<T>(this->g_data._f);
                }
                return std::nullopt;
            }

            return static_cast<T>(this->g_data._d);
        }
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        if (this->g_type != Types::PTYPE_STRING)
        {
            return std::nullopt;
        }

        return *this->g_data.getString();
    }
    else if constexpr (std::is_same_v<T, char const*>)
    {
        if (this->g_type != Types::PTYPE_STRING)
        {
            return std::nullopt;
        }

        return this->g_data.getString()->data();
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        if (this->g_type != Types::PTYPE_POINTER)
        {
            return std::nullopt;
        }

        return reinterpret_cast<T>(this->g_data._ptr);
    }
    else
    {
        if (this->g_type == Types::PTYPE_CLASS)
        {
            if (this->g_data.getClassWrapper()->getType() == typeid(T))
            {
                using TT = remove_cvref_t<T>;
                return this->g_data.getClassWrapper<TT>()->_data;
            }
        }

        return std::nullopt;
    }
}

template<class T>
T* Property::getPtr()
{
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
    {
        if constexpr (sizeof(T) != sizeof(fge::PintType))
        {
            return nullptr;
        }

        if (this->g_type != Types::PTYPE_INTEGERS)
        {
            return nullptr;
        }

        if constexpr (std::is_signed_v<T>)
        {
            return static_cast<T*>(&this->g_data._i);
        }
        else
        {
            return static_cast<T*>(&this->g_data._u);
        }
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        if constexpr (std::is_same_v<T, float>)
        {
            if (this->g_type != Types::PTYPE_FLOAT)
            {
                return nullptr;
            }

            return &this->g_data._f;
        }
        else
        {
            if (this->g_type != Types::PTYPE_DOUBLE)
            {
                return nullptr;
            }

            return &this->g_data._d;
        }
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        if (this->g_type != Types::PTYPE_STRING)
        {
            return nullptr;
        }

        return this->g_data.getString();
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        if (this->g_type != Types::PTYPE_POINTER)
        {
            return nullptr;
        }

        return &reinterpret_cast<T&>(this->g_data._ptr);
    }
    else
    {
        if (this->g_type == Types::PTYPE_CLASS)
        {
            if (this->g_data.getClassWrapper()->getType() == typeid(T))
            {
                using TT = remove_cvref_t<T>;
                return &this->g_data.getClassWrapper<TT>()->_data;
            }
        }

        return nullptr;
    }
}
template<class T>
T const* Property::getPtr() const
{
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
    {
        if constexpr (sizeof(T) != sizeof(fge::PintType))
        {
            return nullptr;
        }

        if (this->g_type != Types::PTYPE_INTEGERS)
        {
            return nullptr;
        }

        if constexpr (std::is_signed_v<T>)
        {
            return static_cast<T*>(&this->g_data._i);
        }
        else
        {
            return static_cast<T*>(&this->g_data._u);
        }
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        if constexpr (std::is_same_v<T, float>)
        {
            if (this->g_type != Types::PTYPE_FLOAT)
            {
                return nullptr;
            }

            return &this->g_data._f;
        }
        else
        {
            if (this->g_type != Types::PTYPE_DOUBLE)
            {
                return nullptr;
            }

            return &this->g_data._d;
        }
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        if (this->g_type != Types::PTYPE_STRING)
        {
            return nullptr;
        }

        return this->g_data.getString();
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        if (this->g_type != Types::PTYPE_POINTER)
        {
            return nullptr;
        }

        return &reinterpret_cast<T&>(this->g_data._ptr);
    }
    else
    {
        if (this->g_type == Types::PTYPE_CLASS)
        {
            if (this->g_data.getClassWrapper()->getType() == typeid(T))
            {
                using TT = remove_cvref_t<T>;
                return &this->g_data.getClassWrapper<TT>()->_data;
            }
        }

        return nullptr;
    }
}

template<class T>
bool Property::pushType()
{
    if (this->g_type == Types::PTYPE_CLASS)
    {
        if (this->g_data.getClassWrapper()->getType() == typeid(fge::ParrayType))
        {
            this->g_data.getArray()->_data.emplace_back().setType<T>();
            return true;
        }
    }
    return false;
}

template<class T>
bool Property::getData(std::size_t index, T& val) const
{
    if (this->g_type == Types::PTYPE_CLASS)
    {
        if (this->g_data.getClassWrapper()->getType() == typeid(fge::ParrayType))
        {
            if (this->g_data.getArray()->_data.size() > index)
            {
                return this->g_data.getArray()->_data[index].get<T>(val);
            }
        }
    }
    return false;
}
template<class T>
T* Property::getDataPtr(std::size_t index)
{
    if (this->g_type == Types::PTYPE_CLASS)
    {
        if (this->g_data.getClassWrapper()->getType() == typeid(fge::ParrayType))
        {
            if (this->g_data.getArray()->_data.size() > index)
            {
                return this->g_data.getArray()->_data[index].getPtr<T>();
            }
        }
    }
    return nullptr;
}
template<class T>
T const* Property::getDataPtr(std::size_t index) const
{
    if (this->g_type == Types::PTYPE_CLASS)
    {
        if (this->g_data.getClassWrapper()->getType() == typeid(fge::ParrayType))
        {
            if (this->g_data.getArray()->_data.size() > index)
            {
                return this->g_data.getArray()->_data[index].getPtr<T>();
            }
        }
    }
    return nullptr;
}

//PropertyClassWrapperType

template<class T>
template<typename>
PropertyClassWrapperType<T>::PropertyClassWrapperType(T const& val) :
        _data(val)
{}
template<class T>
template<typename>
PropertyClassWrapperType<T>::PropertyClassWrapperType(T&& val) noexcept :
        _data(val)
{}

template<class T>
std::type_info const& PropertyClassWrapperType<T>::getType() const
{
    return typeid(T);
}

template<class T>
std::string PropertyClassWrapperType<T>::toString() const
{
    if constexpr (std::is_same_v<T, fge::ParrayType>)
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
    return static_cast<fge::PropertyClassWrapper*>(new fge::PropertyClassWrapperType<T>(this->_data));
}

template<class T>
bool PropertyClassWrapperType<T>::tryToCopy(fge::PropertyClassWrapper const* val)
{
    if (val->getType() == typeid(T))
    {
        this->_data = reinterpret_cast<fge::PropertyClassWrapperType<T> const*>(val)->_data;
        return true;
    }
    return false;
}

template<class T>
bool PropertyClassWrapperType<T>::compare(fge::PropertyClassWrapper const* val)
{
    if (val->getType() == typeid(T))
    {
        if constexpr (comparisonCheck::EqualExists<T>::value)
        {
            return this->_data == reinterpret_cast<fge::PropertyClassWrapperType<T> const*>(val)->_data;
        }
        else
        {
            return false;
        }
    }
    return false;
}

} // namespace fge
