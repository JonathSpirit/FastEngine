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

#include "FastEngine/C_property.hpp"

namespace fge
{

Property::Property(const fge::Property& val) :
        g_type(val.g_type),
        g_isSigned(val.g_isSigned),
        g_isModified(true)
{
    switch (val.g_type)
    {
    case fge::Property::Types::PTYPE_NULL:
        break;
    case fge::Property::Types::PTYPE_STRING:
        this->g_data._ptr = new std::string{*reinterpret_cast<std::string*>(val.g_data._ptr)};
        break;
    case fge::Property::Types::PTYPE_CLASS:
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
    val.g_type = fge::Property::Types::PTYPE_NULL;
}

Property::Property(const char* val) :
        g_type{fge::Property::Types::PTYPE_STRING},
        g_isModified(true)
{
    this->g_data._ptr = new std::string{val};
}

Property::~Property()
{
    this->clear();
}

void Property::clear()
{
    if (this->g_type == fge::Property::Types::PTYPE_STRING)
    {
        delete reinterpret_cast<std::string*>(this->g_data._ptr);
    }
    else if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        delete reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr);
    }

    this->g_type = fge::Property::Types::PTYPE_NULL;
}

bool Property::operator==(const fge::Property& val) const
{
    if (this->g_type == val.g_type)
    {
        switch (this->g_type)
        {
        case fge::Property::Types::PTYPE_NULL:
            return true;
            break;
        case fge::Property::Types::PTYPE_INTEGERS:
            if (this->g_isSigned)
            {
                if (val.g_isSigned)
                {
                    return this->g_data._i == val.g_data._i;
                }
                else
                {
                    return this->g_data._i == static_cast<fge::PintType>(val.g_data._u);
                }
            }
            else
            {
                if (val.g_isSigned)
                {
                    return this->g_data._u == static_cast<fge::PuintType>(val.g_data._i);
                }
                else
                {
                    return this->g_data._u == val.g_data._u;
                }
            }
            break;
        case fge::Property::Types::PTYPE_FLOAT:
            return this->g_data._f == val.g_data._f;
            break;
        case fge::Property::Types::PTYPE_DOUBLE:
            return this->g_data._d == val.g_data._d;
            break;
        case fge::Property::Types::PTYPE_STRING:
            return *reinterpret_cast<std::string*>(this->g_data._ptr) ==
                   *reinterpret_cast<std::string*>(val.g_data._ptr);
            break;
        case fge::Property::Types::PTYPE_POINTER:
            return this->g_data._ptr == val.g_data._ptr;
            break;
        case fge::Property::Types::PTYPE_CLASS:
            return reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)
                    ->compare(reinterpret_cast<fge::PropertyClassWrapper*>(val.g_data._ptr));
            break;
        }
    }
    return false;
}

fge::Property& Property::operator=(const fge::Property& val)
{
    this->set(val);
    return *this;
}
fge::Property& Property::operator=(fge::Property&& val) noexcept
{
    this->set(std::move(val));
    return *this;
}

fge::Property& Property::operator=(const char* val)
{
    this->set(val);
    return *this;
}

void Property::setType(fge::Property::Types type)
{
    if (type == fge::Property::Types::PTYPE_CLASS)
    {
        return;
    }

    if (type != this->g_type)
    {
        this->clear();
        if (type == fge::Property::Types::PTYPE_STRING)
        {
            this->g_data._ptr = new std::string();
        }
        this->g_type = type;
    }
}

bool Property::isType(fge::Property::Types type) const
{
    return this->g_type == type;
}

const std::type_info& Property::getClassType() const
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        return reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType();
    }
    return typeid(nullptr);
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
    case fge::Property::Types::PTYPE_INTEGERS:
        if (this->g_isSigned)
        {
            return fge::string::ToStr(this->g_data._i);
        }
        else
        {
            return fge::string::ToStr(this->g_data._u);
        }
        break;
    case fge::Property::Types::PTYPE_FLOAT:
        return fge::string::ToStr(this->g_data._f);
        break;
    case fge::Property::Types::PTYPE_DOUBLE:
        return fge::string::ToStr(this->g_data._d);
        break;
    case fge::Property::Types::PTYPE_STRING:
        return *reinterpret_cast<std::string*>(this->g_data._ptr);
        break;

    case fge::Property::Types::PTYPE_POINTER:
        return fge::string::ToStr(this->g_data._ptr);
        break;
    case fge::Property::Types::PTYPE_CLASS:
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
        case fge::Property::Types::PTYPE_NULL:
            break;
        case fge::Property::Types::PTYPE_STRING:
            *reinterpret_cast<std::string*>(this->g_data._ptr) = *reinterpret_cast<std::string*>(val.g_data._ptr);
            this->g_isModified = true;
            break;
        case fge::Property::Types::PTYPE_CLASS:
            if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)
                        ->tryToCopy(reinterpret_cast<fge::PropertyClassWrapper*>(val.g_data._ptr)))
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
    else if (this->g_type == fge::Property::Types::PTYPE_NULL)
    {
        this->g_isModified = true;
        this->g_type = val.g_type;
        switch (val.g_type)
        {
        case fge::Property::Types::PTYPE_NULL:
            break;
        case fge::Property::Types::PTYPE_STRING:
            this->g_data._ptr = new std::string(*reinterpret_cast<std::string*>(val.g_data._ptr));
            break;
        case fge::Property::Types::PTYPE_CLASS:
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
        case fge::Property::Types::PTYPE_NULL:
            break;
        case fge::Property::Types::PTYPE_STRING:
        case fge::Property::Types::PTYPE_CLASS:
            this->g_data._ptr = val.g_data._ptr;
            this->g_isModified = true;
            break;

        default:
            this->g_isSigned = val.g_isSigned;
            this->g_data = val.g_data;
            this->g_isModified = true;
            break;
        }

        val.g_type = fge::Property::Types::PTYPE_NULL;
        return true;
    }
    else if (this->g_type == fge::Property::Types::PTYPE_NULL)
    {
        this->g_isModified = true;
        this->g_type = val.g_type;
        switch (val.g_type)
        {
        case fge::Property::Types::PTYPE_NULL:
            break;
        case fge::Property::Types::PTYPE_STRING:
        case fge::Property::Types::PTYPE_CLASS:
            this->g_data._ptr = val.g_data._ptr;
            break;

        default:
            this->g_isSigned = val.g_isSigned;
            this->g_data = val.g_data;
            break;
        }

        val.g_type = fge::Property::Types::PTYPE_NULL;
        return true;
    }

    return false;
}

bool Property::set(const char* val)
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

fge::ParrayType& Property::setArrayType()
{
    if (this->g_type != fge::Property::Types::PTYPE_CLASS)
    {
        this->clear();
        this->g_type = fge::Property::Types::PTYPE_CLASS;
        this->g_data._ptr =
                static_cast<fge::PropertyClassWrapper*>(new fge::PropertyClassWrapperType<fge::ParrayType>());
    }
    else
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() != typeid(fge::ParrayType))
        {
            this->clear();
            this->g_type = fge::Property::Types::PTYPE_CLASS;
            this->g_data._ptr =
                    static_cast<fge::PropertyClassWrapper*>(new fge::PropertyClassWrapperType<fge::ParrayType>());
        }
    }

    return reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)->_data;
}

bool Property::resize(std::size_t n)
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)->_data.resize(n);
            return true;
        }
    }
    return false;
}
bool Property::reserve(std::size_t n)
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)->_data.reserve(n);
            return true;
        }
    }
    return false;
}

bool Property::pushData(const fge::Property& value)
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)
                    ->_data.emplace_back(value);
            return true;
        }
    }
    return false;
}
bool Property::pushData(fge::Property&& value)
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)
                    ->_data.emplace_back(std::move(value));
            return true;
        }
    }
    return false;
}

bool Property::setData(std::size_t index, const fge::Property& value)
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)->_data[index] = value;
            return true;
        }
    }
    return false;
}
bool Property::setData(std::size_t index, fge::Property&& value)
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)->_data[index] =
                    std::move(value);
            return true;
        }
    }
    return false;
}

fge::Property* Property::getData(std::size_t index)
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            if (reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)->_data.size() >
                index)
            {
                return &reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)
                                ->_data[index];
            }
        }
    }
    return nullptr;
}
const fge::Property* Property::getData(std::size_t index) const
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            if (reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)->_data.size() >
                index)
            {
                return &reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)
                                ->_data[index];
            }
        }
    }
    return nullptr;
}

std::size_t Property::getDataSize() const
{
    if (this->g_type == fge::Property::Types::PTYPE_CLASS)
    {
        if (reinterpret_cast<fge::PropertyClassWrapper*>(this->g_data._ptr)->getType() == typeid(fge::ParrayType))
        {
            return reinterpret_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->g_data._ptr)->_data.size();
        }
    }
    return 0;
}

fge::Property* Property::operator[](std::size_t index)
{
    return this->getData(index);
}
const fge::Property* Property::operator[](std::size_t index) const
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

} // namespace fge
