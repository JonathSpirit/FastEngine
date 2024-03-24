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

#ifndef _FGE_C_PROPERTY_HPP_INCLUDED
#define _FGE_C_PROPERTY_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/extra/extra_string.hpp"
#include <optional>
#include <string>
#include <typeinfo>
#include <vector>

namespace fge
{

using PintType = int64_t;
using PuintType = uint64_t;
using PfloatType = float;
using PdoubleType = double;

class Property;
using ParrayType = std::vector<fge::Property>;

class PropertyClassWrapper;
template<class T>
class PropertyClassWrapperType;

/**
 * \class Property
 * \ingroup utility
 * \brief A class that can store any type of data
 *
 * This class can store any type of data.
 * Integer is converted to PintType or PuintType so in order to get the value you need to use the defined types.
 * class oder than the basic type are stored as a pointer to a PropertyClassWrapper.
 *
 * This class also easly enable to store an array of Property.
 */
class FGE_API Property
{
    template<class T>
    struct remove_cvref
    {
        typedef std::remove_cv_t<std::remove_reference_t<T>> type;
    };
    template<class T>
    using remove_cvref_t = typename remove_cvref<T>::type;

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

        constexpr void setClassWrapper(fge::PropertyClassWrapper* val) { this->_ptr = val; }
        [[nodiscard]] constexpr fge::PropertyClassWrapper* getClassWrapper() const
        {
            return static_cast<fge::PropertyClassWrapper*>(this->_ptr);
        }
        template<class T>
        [[nodiscard]] constexpr fge::PropertyClassWrapperType<T>* getClassWrapper() const
        {
            return static_cast<fge::PropertyClassWrapperType<T>*>(this->_ptr);
        }
        [[nodiscard]] constexpr fge::PropertyClassWrapperType<fge::ParrayType>* getArray() const
        {
            return static_cast<fge::PropertyClassWrapperType<fge::ParrayType>*>(this->_ptr);
        }
        [[nodiscard]] constexpr std::string* getString() const { return static_cast<std::string*>(this->_ptr); }
    };

    Property() = default;

    //Copy/Move constructor
    Property(fge::Property const& val);
    Property(fge::Property&& val) noexcept;

    //Copy/Move some type constructor
    template<class T, typename = std::enable_if_t<!std::is_same_v<remove_cvref_t<T>, fge::Property>>>
    Property(T&& val);

    //Special string copy constructor
    Property(char const* val);

    ~Property();

    void clear();

    [[nodiscard]] bool operator==(fge::Property const& val) const;

    //Copy/Move operator
    fge::Property& operator=(fge::Property const& val);
    fge::Property& operator=(fge::Property&& val) noexcept;

    //Copy/Move some type operator
    template<class T, typename = std::enable_if_t<!std::is_same_v<remove_cvref_t<T>, fge::Property>>>
    fge::Property& operator=(T&& val);

    //Special string copy operator
    fge::Property& operator=(char const* val);

    template<class T>
    T& setType();
    void setType(Types type);
    template<class T>
    [[nodiscard]] bool isType() const;
    [[nodiscard]] bool isType(Types type) const;

    [[nodiscard]] std::type_info const& getClassType() const;
    [[nodiscard]] Types getType() const;
    [[nodiscard]] bool isSigned() const;

    [[nodiscard]] std::string toString() const;

    bool set(fge::Property const& val);
    bool set(fge::Property&& val);

    template<class T, typename = std::enable_if_t<!std::is_same_v<remove_cvref_t<T>, fge::Property>>>
    bool set(T&& val);

    bool set(char const* val);

    template<class T>
    bool get(T& val) const;
    template<class T>
    std::optional<T> get() const;

    template<class T>
    T* getPtr();
    template<class T>
    T const* getPtr() const;

    //Value array control
    fge::ParrayType& setArrayType();

    bool resize(std::size_t n);
    bool reserve(std::size_t n);

    bool pushData(fge::Property const& value);
    bool pushData(fge::Property&& value);

    template<class T>
    bool pushType();

    bool setData(std::size_t index, fge::Property const& value);
    bool setData(std::size_t index, fge::Property&& value);

    [[nodiscard]] fge::Property* getData(std::size_t index);
    [[nodiscard]] fge::Property const* getData(std::size_t index) const;

    template<class T>
    bool getData(std::size_t index, T& val) const;
    template<class T>
    T* getDataPtr(std::size_t index);
    template<class T>
    T const* getDataPtr(std::size_t index) const;

    [[nodiscard]] std::size_t getDataSize() const;

    fge::Property* operator[](std::size_t index);
    fge::Property const* operator[](std::size_t index) const;

    [[nodiscard]] bool isModified() const;
    void setModifiedFlag(bool flag);

private:
    Types g_type{Types::PTYPE_NULL};
    Data g_data{};
    bool g_isSigned{};
    bool g_isModified{false};
};

class PropertyClassWrapper
{
public:
    PropertyClassWrapper() = default;
    virtual ~PropertyClassWrapper() = default;

    [[nodiscard]] virtual std::type_info const& getType() const = 0;

    [[nodiscard]] virtual std::string toString() const = 0;

    [[nodiscard]] virtual fge::PropertyClassWrapper* copy() const = 0;

    virtual bool tryToCopy(fge::PropertyClassWrapper const* val) = 0;

    [[nodiscard]] virtual bool compare(fge::PropertyClassWrapper const* val) = 0;
};

namespace comparisonCheck
{
struct No
{};
template<typename T, typename Arg>
No operator==(T const&, Arg const&);

template<typename T, typename Arg = T>
struct EqualExists
{
    enum
    {
        value = !std::is_same<decltype(std::declval<T>() == std::declval<Arg>()), No>::value
    };
};
} // namespace comparisonCheck

template<class T>
class PropertyClassWrapperType : public PropertyClassWrapper
{
    static_assert(std::negation_v<std::is_base_of<fge::PropertyClassWrapper, T>>,
                  "fge::PropertyClassWrapperType<T>, T must not be based on fge::PropertyClassWrapper class type !");
    static_assert(std::negation_v<std::is_base_of<fge::Property, T>>,
                  "fge::PropertyClassWrapperType<T>, T must not be based on fge::Property class type !");
    static_assert(std::negation_v<std::is_pointer<T>>, "fge::PropertyClassWrapperType<T>, T must not be a pointer !");
    static_assert(std::negation_v<std::is_reference<T>>,
                  "fge::PropertyClassWrapperType<T>, T must not be a reference !");

public:
    PropertyClassWrapperType() = default;
    template<typename = std::enable_if_t<std::is_copy_constructible_v<T>>>
    explicit PropertyClassWrapperType(T const& val);
    template<typename = std::enable_if_t<std::is_move_constructible_v<T>>>
    explicit PropertyClassWrapperType(T&& val) noexcept;
    ~PropertyClassWrapperType() override = default;

    [[nodiscard]] std::type_info const& getType() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] fge::PropertyClassWrapper* copy() const override;

    bool tryToCopy(fge::PropertyClassWrapper const* val) override;

    [[nodiscard]] bool compare(fge::PropertyClassWrapper const* val) override;

    T _data;
};

} // namespace fge

#include "FastEngine/C_property.inl"

#endif // _FGE_C_PROPERTY_HPP_INCLUDED
