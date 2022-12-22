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

namespace fge
{

//Property

template<class T, typename>
Property::Property(const T& val) :
        g_isModified(true)
{
    if constexpr (std::is_integral<std::remove_reference_t<T>>::value ||
                  std::is_enum<std::remove_reference_t<T>>::value)
    {
        this->g_type = fge::Property::Types::PTYPE_INTEGERS;

        if constexpr (std::is_signed<std::remove_reference_t<T>>::value)
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
    else if constexpr (std::is_floating_point<std::remove_reference_t<T>>::value)
    {
        if constexpr (std::is_same<std::remove_reference_t<T>, float>::value)
        {
            this->g_type = fge::Property::Types::PTYPE_FLOAT;
            this->g_data._f = val;
        }
        else
        {
            this->g_type = fge::Property::Types::PTYPE_DOUBLE;
            this->g_data._d = static_cast<double>(val);
        }
    }
    else if constexpr (std::is_same<std::remove_reference_t<T>, std::string>::value)
    {
        this->g_type = fge::Property::Types::PTYPE_STRING;
        this->g_data._ptr = new std::string(val);
    }
    else if constexpr (std::is_pointer<std::remove_reference_t<T>>::value)
    {
        this->g_type = fge::Property::Types::PTYPE_POINTER;
        this->g_data._ptr = val;
    }
    else
    {
        this->g_type = fge::Property::Types::PTYPE_CLASS;
        this->g_data._ptr = static_cast<fge::PropertyClassWrapper*>(
                new fge::PropertyClassWrapperType<std::remove_reference_t<T>>(val));
    }
}
template<class T, typename>
Property::Property(T&& val) :
        g_isModified(true)
{
    if constexpr (std::is_integral<std::remove_reference_t<T>>::value ||
                  std::is_enum<std::remove_reference_t<T>>::value)
    {
        this->g_type = fge::Property::Types::PTYPE_INTEGERS;

        if constexpr (std::is_signed<std::remove_reference_t<T>>::value)
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
    else if constexpr (std::is_floating_point<std::remove_reference_t<T>>::value)
    {
        if constexpr (std::is_same<std::remove_reference_t<T>, float>::value)
        {
            this->g_type = fge::Property::Types::PTYPE_FLOAT;
            this->g_data._f = val;
        }
        else
        {
            this->g_type = fge::Property::Types::PTYPE_DOUBLE;
            this->g_data._d = static_cast<double>(val);
        }
    }
    else if constexpr (std::is_same<std::remove_reference_t<T>, std::string>::value)
    {
        this->g_type = fge::Property::Types::PTYPE_STRING;
        this->g_data._ptr = new std::string(std::forward<T>(val));
    }
    else if constexpr (std::is_pointer<std::remove_reference_t<T>>::value)
    {
        this->g_type = fge::Property::Types::PTYPE_POINTER;
        this->g_data._ptr = val;
    }
    else
    {
        this->g_type = fge::Property::Types::PTYPE_CLASS;
        this->g_data._ptr = static_cast<fge::PropertyClassWrapper*>(
                new fge::PropertyClassWrapperType<std::remove_reference_t<T>>(std::forward<T>(val)));
    }
}

template<class T, typename>
fge::Property& Property::operator=(const T& val)
{
    this->set(std::forward<T>(val));
    return *this;
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
    if constexpr (std::is_integral<T>::value || std::is_enum<T>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_INTEGERS)
        {
            this->clear();
            this->g_type = fge::Property::Types::PTYPE_INTEGERS;
        }

        if constexpr (std::is_signed<T>::value)
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
    else if constexpr (std::is_floating_point<T>::value)
    {
        if constexpr (std::is_same<T, float>::value)
        {
            if (this->g_type != fge::Property::Types::PTYPE_FLOAT)
            {
                this->clear();
                this->g_type = fge::Property::Types::PTYPE_FLOAT;
            }

            return this->g_data._f;
        }
        else
        {
            if (this->g_type != fge::Property::Types::PTYPE_DOUBLE)
            {
                this->clear();
                this->g_type = fge::Property::Types::PTYPE_DOUBLE;
            }

            return this->g_data._d;
        }
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_STRING)
        {
            this->clear();
            this->g_type = fge::Property::Types::PTYPE_STRING;
            this->g_data._ptr = new std::string();
        }

        return *reinterpret_cast<std::string*>(this->g_data._ptr);
    }
    else if constexpr (std::is_pointer<T>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_POINTER)
        {
            this->clear();
            this->g_type = fge::Property::Types::PTYPE_POINTER;
        }

        return reinterpret_cast<typename std::add_lvalue_reference<T>::type>(this->g_data._ptr);
    }
    else
    {
        if (this->g_type != fge::Property::Types::PTYPE_CLASS)
        {
            this->clear();
            this->g_type = fge::Property::Types::PTYPE_CLASS;
            this->g_data._ptr = static_cast<fge::PropertyClassWrapper*>(
                    new fge::PropertyClassWrapperType<std::remove_reference_t<T>>());
            return reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)
                    ->_data;
        }
        else
        {
            if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T))
            {
                return reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)
                        ->_data;
            }
            else
            {
                this->clear();
                this->g_type = fge::Property::Types::PTYPE_CLASS;
                this->g_data._ptr = static_cast<fge::PropertyClassWrapper*>(
                        new fge::PropertyClassWrapperType<std::remove_reference_t<T>>());
                return reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)
                        ->_data;
            }
        }
    }
}

template<class T>
bool Property::isType() const
{
    if constexpr (std::is_integral<T>::value || std::is_enum<T>::value)
    {
        return this->g_type == fge::Property::Types::PTYPE_INTEGERS;
    }
    else if constexpr (std::is_floating_point<T>::value)
    {
        if constexpr (std::is_same<T, float>::value)
        {
            return this->g_type == fge::Property::Types::PTYPE_FLOAT;
        }
        else
        {
            return this->g_type == fge::Property::Types::PTYPE_DOUBLE;
        }
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
        return this->g_type == fge::Property::Types::PTYPE_STRING;
    }
    else if constexpr (std::is_pointer<T>::value)
    {
        return this->g_type == fge::Property::Types::PTYPE_POINTER;
    }
    else
    {
        if (this->g_type == fge::Property::Types::PTYPE_CLASS)
        {
            return reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T);
        }
        else
        {
            return false;
        }
    }
}

template<class T, typename>
bool Property::set(const T& val)
{
    if constexpr (std::is_integral<std::remove_reference_t<T>>::value ||
                  std::is_enum<std::remove_reference_t<T>>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_INTEGERS)
        {
            if (this->g_type == fge::Property::Types::PTYPE_NULL)
            {
                this->g_type = fge::Property::Types::PTYPE_INTEGERS;
            }
            else
            {
                return false;
            }
        }

        if constexpr (std::is_signed<std::remove_reference_t<T>>::value)
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
    else if constexpr (std::is_floating_point<std::remove_reference_t<T>>::value)
    {
        if constexpr (std::is_same<std::remove_reference_t<T>, float>::value)
        {
            if (this->g_type != fge::Property::Types::PTYPE_FLOAT)
            {
                if (this->g_type == fge::Property::Types::PTYPE_NULL)
                {
                    this->g_type = fge::Property::Types::PTYPE_FLOAT;
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
            if (this->g_type != fge::Property::Types::PTYPE_DOUBLE)
            {
                if (this->g_type == fge::Property::Types::PTYPE_NULL)
                {
                    this->g_type = fge::Property::Types::PTYPE_DOUBLE;
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
    else if constexpr (std::is_same<std::remove_reference_t<T>, std::string>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_STRING)
        {
            if (this->g_type == fge::Property::Types::PTYPE_NULL)
            {
                this->g_type = fge::Property::Types::PTYPE_STRING;
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
    else if constexpr (std::is_pointer<std::remove_reference_t<T>>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_POINTER)
        {
            if (this->g_type == fge::Property::Types::PTYPE_NULL)
            {
                this->g_type = fge::Property::Types::PTYPE_POINTER;
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
        if (this->g_type != fge::Property::Types::PTYPE_CLASS)
        {
            if (this->g_type == fge::Property::Types::PTYPE_NULL)
            {
                this->g_type = fge::Property::Types::PTYPE_CLASS;
                this->g_data._ptr = static_cast<fge::PropertyClassWrapper*>(
                        new fge::PropertyClassWrapperType<std::remove_reference_t<T>>(val));
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() ==
                typeid(std::remove_reference_t<T>))
            {
                reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)->_data =
                        val;
                return true;
            }
            return false;
        }
    }
}
template<class T, typename>
bool Property::set(T&& val)
{
    if constexpr (std::is_integral<std::remove_reference_t<T>>::value ||
                  std::is_enum<std::remove_reference_t<T>>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_INTEGERS)
        {
            if (this->g_type == fge::Property::Types::PTYPE_NULL)
            {
                this->g_type = fge::Property::Types::PTYPE_INTEGERS;
            }
            else
            {
                return false;
            }
        }

        if constexpr (std::is_signed<std::remove_reference_t<T>>::value)
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
    else if constexpr (std::is_floating_point<std::remove_reference_t<T>>::value)
    {
        if constexpr (std::is_same<std::remove_reference_t<T>, float>::value)
        {
            if (this->g_type != fge::Property::Types::PTYPE_FLOAT)
            {
                if (this->g_type == fge::Property::Types::PTYPE_NULL)
                {
                    this->g_type = fge::Property::Types::PTYPE_FLOAT;
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
            if (this->g_type != fge::Property::Types::PTYPE_DOUBLE)
            {
                if (this->g_type == fge::Property::Types::PTYPE_NULL)
                {
                    this->g_type = fge::Property::Types::PTYPE_DOUBLE;
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
    else if constexpr (std::is_same<std::remove_reference_t<T>, std::string>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_STRING)
        {
            if (this->g_type == fge::Property::Types::PTYPE_NULL)
            {
                this->g_type = fge::Property::Types::PTYPE_STRING;
                this->g_data._ptr = new std::string(std::forward<T>(val));
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
    else if constexpr (std::is_pointer<std::remove_reference_t<T>>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_POINTER)
        {
            if (this->g_type == fge::Property::Types::PTYPE_NULL)
            {
                this->g_type = fge::Property::Types::PTYPE_POINTER;
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
        if (this->g_type != fge::Property::Types::PTYPE_CLASS)
        {
            if (this->g_type == fge::Property::Types::PTYPE_NULL)
            {
                this->g_type = fge::Property::Types::PTYPE_CLASS;
                this->g_data._ptr = static_cast<fge::PropertyClassWrapper*>(
                        new fge::PropertyClassWrapperType<std::remove_reference_t<T>>(std::forward<T>(val)));
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() ==
                typeid(std::remove_reference_t<T>))
            {
                reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)->_data =
                        std::forward<T>(val);
                return true;
            }
            return false;
        }
    }
}

template<class T>
bool Property::get(T& val) const
{
    if constexpr (std::is_integral<T>::value || std::is_enum<T>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_INTEGERS)
        {
            if (this->g_type == fge::Property::Types::PTYPE_FLOAT)
            {
                val = static_cast<T>(this->g_data._f);
                return true;
            }
            else if (this->g_type == fge::Property::Types::PTYPE_DOUBLE)
            {
                val = static_cast<T>(this->g_data._d);
                return true;
            }
            else
            {
                return false;
            }
        }

        if constexpr (std::is_signed<T>::value)
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
    else if constexpr (std::is_floating_point<T>::value)
    {
        if constexpr (std::is_same<T, float>::value)
        {
            if (this->g_type != fge::Property::Types::PTYPE_FLOAT)
            {
                if (this->g_type == fge::Property::Types::PTYPE_INTEGERS)
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
                else if (this->g_type == fge::Property::Types::PTYPE_DOUBLE)
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
            if (this->g_type != fge::Property::Types::PTYPE_DOUBLE)
            {
                if (this->g_type == fge::Property::Types::PTYPE_INTEGERS)
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
                else if (this->g_type == fge::Property::Types::PTYPE_FLOAT)
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
    else if constexpr (std::is_same<T, std::string>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_STRING)
        {
            return false;
        }

        val = *reinterpret_cast<std::string*>(this->g_data._ptr);
        return true;
    }
    else if constexpr (std::is_same<T, const char*>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_STRING)
        {
            return false;
        }

        val = reinterpret_cast<std::string*>(this->g_data._ptr)->data();
        return true;
    }
    else if constexpr (std::is_pointer<T>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_POINTER)
        {
            return false;
        }

        val = reinterpret_cast<T>(this->g_data._ptr);
        return true;
    }
    else
    {
        if (this->g_type == fge::Property::Types::PTYPE_CLASS)
        {
            if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T))
            {
                val = reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)
                              ->_data;
                return true;
            }
        }

        return false;
    }
}
template<class T>
std::optional<T> Property::get() const
{
    if constexpr (std::is_integral<T>::value || std::is_enum<T>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_INTEGERS)
        {
            if (this->g_type == fge::Property::Types::PTYPE_FLOAT)
            {
                return static_cast<T>(this->g_data._f);
            }
            if (this->g_type == fge::Property::Types::PTYPE_DOUBLE)
            {
                return static_cast<T>(this->g_data._d);
            }
            return std::nullopt;
        }

        if constexpr (std::is_signed<T>::value)
        {
            return static_cast<T>(this->g_data._i);
        }
        else
        {
            return static_cast<T>(this->g_data._u);
        }
    }
    else if constexpr (std::is_floating_point<T>::value)
    {
        if constexpr (std::is_same<T, float>::value)
        {
            if (this->g_type != fge::Property::Types::PTYPE_FLOAT)
            {
                if (this->g_type == fge::Property::Types::PTYPE_INTEGERS)
                {
                    if (this->g_isSigned)
                    {
                        return static_cast<T>(this->g_data._i);
                    }
                    return static_cast<T>(this->g_data._u);
                }
                if (this->g_type == fge::Property::Types::PTYPE_DOUBLE)
                {
                    return static_cast<T>(this->g_data._d);
                }
                return std::nullopt;
            }

            return static_cast<T>(this->g_data._f);
        }
        else
        {
            if (this->g_type != fge::Property::Types::PTYPE_DOUBLE)
            {
                if (this->g_type == fge::Property::Types::PTYPE_INTEGERS)
                {
                    if (this->g_isSigned)
                    {
                        return static_cast<T>(this->g_data._i);
                    }
                    return static_cast<T>(this->g_data._u);
                }
                if (this->g_type == fge::Property::Types::PTYPE_FLOAT)
                {
                    return static_cast<T>(this->g_data._f);
                }
                return std::nullopt;
            }

            return static_cast<T>(this->g_data._d);
        }
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_STRING)
        {
            return std::nullopt;
        }

        return *reinterpret_cast<std::string*>(this->g_data._ptr);
    }
    else if constexpr (std::is_same<T, const char*>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_STRING)
        {
            return std::nullopt;
        }

        return reinterpret_cast<std::string*>(this->g_data._ptr)->data();
    }
    else if constexpr (std::is_pointer<T>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_POINTER)
        {
            return std::nullopt;
        }

        return reinterpret_cast<T>(this->g_data._ptr);
    }
    else
    {
        if (this->g_type == fge::Property::Types::PTYPE_CLASS)
        {
            if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T))
            {
                return reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)
                        ->_data;
            }
        }

        return std::nullopt;
    }
}

template<class T>
T* Property::getPtr()
{
    if constexpr (std::is_integral<T>::value || std::is_enum<T>::value)
    {
        if constexpr (sizeof(T) != sizeof(fge::PintType))
        {
            return nullptr;
        }

        if (this->g_type != fge::Property::Types::PTYPE_INTEGERS)
        {
            return nullptr;
        }

        if constexpr (std::is_signed<T>::value)
        {
            return static_cast<T*>(&this->g_data._i);
        }
        else
        {
            return static_cast<T*>(&this->g_data._u);
        }
    }
    else if constexpr (std::is_floating_point<T>::value)
    {
        if constexpr (std::is_same<T, float>::value)
        {
            if (this->g_type != fge::Property::Types::PTYPE_FLOAT)
            {
                return nullptr;
            }

            return &this->g_data._f;
        }
        else
        {
            if (this->g_type != fge::Property::Types::PTYPE_DOUBLE)
            {
                return nullptr;
            }

            return &this->g_data._d;
        }
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_STRING)
        {
            return nullptr;
        }

        return reinterpret_cast<std::string*>(this->g_data._ptr);
    }
    else if constexpr (std::is_pointer<T>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_POINTER)
        {
            return nullptr;
        }

        return &reinterpret_cast<T>(this->g_data._ptr);
    }
    else
    {
        if (this->g_type == fge::Property::Types::PTYPE_CLASS)
        {
            if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T))
            {
                return &reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)
                                ->_data;
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
        if constexpr (sizeof(T) != sizeof(fge::PintType))
        {
            return nullptr;
        }

        if (this->g_type != fge::Property::Types::PTYPE_INTEGERS)
        {
            return nullptr;
        }

        if constexpr (std::is_signed<T>::value)
        {
            return static_cast<T*>(&this->g_data._i);
        }
        else
        {
            return static_cast<T*>(&this->g_data._u);
        }
    }
    else if constexpr (std::is_floating_point<T>::value)
    {
        if constexpr (std::is_same<T, float>::value)
        {
            if (this->g_type != fge::Property::Types::PTYPE_FLOAT)
            {
                return nullptr;
            }

            return &this->g_data._f;
        }
        else
        {
            if (this->g_type != fge::Property::Types::PTYPE_DOUBLE)
            {
                return nullptr;
            }

            return &this->g_data._d;
        }
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_STRING)
        {
            return nullptr;
        }

        return reinterpret_cast<std::string*>(this->g_data._ptr);
    }
    else if constexpr (std::is_pointer<T>::value)
    {
        if (this->g_type != fge::Property::Types::PTYPE_POINTER)
        {
            return nullptr;
        }

        return &reinterpret_cast<T>(this->g_data._ptr);
    }
    else
    {
        if (this->g_type == fge::Property::Types::PTYPE_CLASS)
        {
            if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(T))
            {
                return &reinterpret_cast<fge::PropertyClassWrapperType<std::remove_reference_t<T>>*>(this->g_data._ptr)
                                ->_data;
            }
        }

        return nullptr;
    }
}

template<class T>
bool Property::pushType()
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)
                    ->_data.emplace_back()
                    .setType<T>();
            return true;
        }
    }
    return false;
}

template<class T>
bool Property::getData(std::size_t index, T& val) const
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            if (reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)->_data.size() >
                index)
            {
                return reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)
                        ->_data[index]
                        .get<T>(val);
            }
        }
    }
    return false;
}
template<class T>
T* Property::getDataPtr(std::size_t index)
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            if (reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)->_data.size() >
                index)
            {
                return reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)
                        ->_data[index]
                        .getPtr<T>();
            }
        }
    }
    return nullptr;
}
template<class T>
const T* Property::getDataPtr(std::size_t index) const
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            if (reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)->_data.size() >
                index)
            {
                return reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)
                        ->_data[index]
                        .getPtr<T>();
            }
        }
    }
    return nullptr;
}

//PropertyClassWrapperType

template<class T>
PropertyClassWrapperType<T>::PropertyClassWrapperType(T val) :
        _data(std::move(val))
{}

template<class T>
const std::type_info& PropertyClassWrapperType<T>::getType() const
{
    return typeid(T);
}

template<class T>
std::string PropertyClassWrapperType<T>::toString() const
{
    if constexpr (std::is_same<T, fge::ParrayType>::value)
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
bool PropertyClassWrapperType<T>::tryToCopy(const fge::PropertyClassWrapper* val)
{
    if (val->getType() == typeid(T))
    {
        this->_data = reinterpret_cast<const fge::PropertyClassWrapperType<T>*>(val)->_data;
        return true;
    }
    return false;
}

template<class T>
bool PropertyClassWrapperType<T>::compare(const fge::PropertyClassWrapper* val)
{
    if (val->getType() == typeid(T))
    {
        if constexpr (comparisonCheck::EqualExists<T>::value)
        {
            return this->_data == reinterpret_cast<const fge::PropertyClassWrapperType<T>*>(val)->_data;
        }
        else
        {
            return false;
        }
    }
    return false;
}

} // namespace fge
