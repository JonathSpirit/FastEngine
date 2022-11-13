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

#ifndef _FGE_C_PROPERTY_HPP_INCLUDED
#define _FGE_C_PROPERTY_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/extra/extra_string.hpp"
#include <string>
#include <vector>
#include <typeinfo>
#include <optional>

namespace fge
{

using PintType = int64_t;
using PuintType = uint64_t;
using PfloatType = float;
using PdoubleType = double;

class Property;
using ParrayType = std::vector<fge::Property>;

class FGE_API Property
{
public:
    enum class Types : uint8_t
    {
        PTYPE_NULL,

        PTYPE_INTEGERS,
        PTYPE_FLOAT,
        PTYPE_DOUBLE,

        PTYPE_STRING,

        PTYPE_POINTER,
        PTYPE_CLASS
    };
    union Data
    {
        fge::PuintType _u;
        fge::PintType _i;

        fge::PfloatType _f;
        fge::PdoubleType _d;

        void* _ptr;
    };

    Property() = default;

    //Copy/Move constructor
    Property(const fge::Property& val);
    Property(fge::Property&& val) noexcept;

    //Copy/Move some type constructor
    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    Property(const T& val);
    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    Property(T&& val);

    //Special string copy constructor
    Property(const char* val);

    ~Property();

    void clear();

    bool operator== (const fge::Property& val) const;

    //Copy/Move operator
    fge::Property& operator= (const fge::Property& val);
    fge::Property& operator= (fge::Property&& val) noexcept;

    //Copy/Move some type operator
    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    fge::Property& operator= (const T& val);
    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    fge::Property& operator= (T&& val);

    //Special string copy operator
    fge::Property& operator= (const char* val);

    template<class T>
    T& setType();
    void setType(fge::Property::Types type);
    template<class T>
    [[nodiscard]] bool isType() const;
    [[nodiscard]] bool isType(fge::Property::Types type) const;

    [[nodiscard]] const std::type_info& getClassType() const;
    [[nodiscard]] Property::Types getType() const;
    [[nodiscard]] bool isSigned() const;

    [[nodiscard]] std::string toString() const;

    bool set(const fge::Property& val);
    bool set(fge::Property&& val);

    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    bool set(const T& val);
    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    bool set(T&& val);

    bool set(const char* val);

    template<class T>
    bool get(T& val) const;
    template<class T>
    std::optional<T> get() const;

    template<class T>
    T* getPtr();
    template<class T>
    const T* getPtr() const;

    //Value array control
    fge::ParrayType& setArrayType();

    bool resize(std::size_t n);
    bool reserve(std::size_t n);

    bool pushData(const fge::Property& value);
    bool pushData(fge::Property&& value);

    template<class T>
    bool pushType();

    bool setData(std::size_t index, const fge::Property& value);
    bool setData(std::size_t index, fge::Property&& value);

    [[nodiscard]] fge::Property* getData(std::size_t index);
    [[nodiscard]] const fge::Property* getData(std::size_t index) const;

    template<class T>
    bool getData(std::size_t index, T& val) const;
    template<class T>
    T* getDataPtr(std::size_t index);
    template<class T>
    const T* getDataPtr(std::size_t index) const;

    [[nodiscard]] std::size_t getDataSize() const;

    fge::Property* operator[] (std::size_t index);
    const fge::Property* operator[] (std::size_t index) const;

    [[nodiscard]] bool isModified() const;
    void setModifiedFlag(bool flag);

private:
    Property::Types g_type{Property::Types::PTYPE_NULL};
    Property::Data g_data{};
    bool g_isSigned{};
    bool g_isModified{false};
};

class PropertyClassWrapper
{
public:
    PropertyClassWrapper() = default;
    virtual ~PropertyClassWrapper() = default;

    [[nodiscard]] virtual const std::type_info& getType() const = 0;

    [[nodiscard]] virtual std::string toString() const = 0;

    [[nodiscard]] virtual fge::PropertyClassWrapper* copy() const = 0;

    virtual bool tryToCopy(const fge::PropertyClassWrapper* val) = 0;

    [[nodiscard]] virtual bool compare(const fge::PropertyClassWrapper* val) = 0;
};

namespace comparisonCheck
{
    struct No {};
    template<typename T, typename Arg> No operator== (const T&, const Arg&);

    template<typename T, typename Arg = T>
    struct EqualExists
    {
        enum { value = !std::is_same<decltype(std::declval<T>() == std::declval<Arg>()), No>::value };
    };
}

template<class T>
class PropertyClassWrapperType : public PropertyClassWrapper
{
    static_assert(std::negation<std::is_base_of<fge::PropertyClassWrapper, T>>::value, "fge::PropertyClassWrapperType<T>, T must not be based on fge::PropertyClassWrapper class type !");
    static_assert(std::negation<std::is_base_of<fge::Property, T>>::value, "fge::PropertyClassWrapperType<T>, T must not be based on fge::Property class type !");
    static_assert(std::negation<std::is_pointer<T>>::value, "fge::PropertyClassWrapperType<T>, T must not be a pointer !");
    static_assert(std::negation<std::is_reference<T>>::value, "fge::PropertyClassWrapperType<T>, T must not be a reference !");
public:
    PropertyClassWrapperType() = default;
    explicit PropertyClassWrapperType(T val);
    ~PropertyClassWrapperType() override = default;

    [[nodiscard]] const std::type_info& getType() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] fge::PropertyClassWrapper* copy() const override;

    bool tryToCopy(const fge::PropertyClassWrapper* val) override;

    [[nodiscard]] bool compare(const fge::PropertyClassWrapper* val) override;

    T _data;
};

}//end fge

#include <FastEngine/C_property.inl>

#endif // _FGE_C_PROPERTY_HPP_INCLUDED
