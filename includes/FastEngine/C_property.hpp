#ifndef _FGE_C_PROPERTY_HPP_INCLUDED
#define _FGE_C_PROPERTY_HPP_INCLUDED

#include <FastEngine/extra_string.hpp>
#include <string>
#include <vector>
#include <typeinfo>
#include <stdexcept>

namespace fge
{

using PintType = int64_t;
using PuintType = uint64_t;
using PfloatType = float;
using PdoubleType = double;

class Property;
using ParrayType = std::vector<fge::Property>;

class Property
{
public:
    enum Types
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
    inline Property(const fge::Property& val);
    inline Property(fge::Property&& val) noexcept;

    //Copy/Move some type constructor
    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    inline Property(const T& val);
    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    inline Property(T&& val);

    //Special string copy constructor
    inline Property(const char* val);

    inline ~Property();

    inline void clear();

    inline bool operator== (const fge::Property& val) const;

    //Copy/Move operator
    inline fge::Property& operator= (const fge::Property& val);
    inline fge::Property& operator= (fge::Property&& val) noexcept;

    //Copy/Move some type operator
    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    inline fge::Property& operator= (const T& val);
    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    inline fge::Property& operator= (T&& val);

    //Special string copy operator
    inline fge::Property& operator= (const char* val);

    template<class T>
    inline T& setType();
    inline void setType(fge::Property::Types type);
    template<class T>
    [[nodiscard]] inline bool isType() const;
    [[nodiscard]] inline bool isType(fge::Property::Types type) const;

    [[nodiscard]] inline const std::type_info& getClassType() const;
    [[nodiscard]] inline Property::Types getType() const;
    [[nodiscard]] inline bool isSigned() const;

    [[nodiscard]] inline std::string toString() const;

    inline bool set(const fge::Property& val);
    inline bool set(fge::Property&& val);

    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    inline bool set(const T& val);
    template<class T,
            typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, fge::Property>::value> >
    inline bool set(T&& val);

    inline bool set(const char* val);

    template<class T>
    inline bool get(T& val) const;
    template<class T>
    inline T get() const;

    template<class T>
    inline T* getPtr();
    template<class T>
    inline const T* getPtr() const;

    //Value array control
    inline fge::ParrayType& setArrayType();

    inline bool resize(std::size_t n);
    inline bool reserve(std::size_t n);

    inline bool pushData(const fge::Property& value);
    inline bool pushData(fge::Property&& value);

    template<class T>
    inline bool pushType();

    inline bool setData(std::size_t index, const fge::Property& value);
    inline bool setData(std::size_t index, fge::Property&& value);

    [[nodiscard]] inline fge::Property* getData(std::size_t index);
    [[nodiscard]] inline const fge::Property* getData(std::size_t index) const;

    template<class T>
    inline bool getData(std::size_t index, T& val) const;
    template<class T>
    inline T* getDataPtr(std::size_t index);
    template<class T>
    inline const T* getDataPtr(std::size_t index) const;

    [[nodiscard]] inline std::size_t getDataSize() const;

    inline fge::Property* operator[] (std::size_t index);
    inline const fge::Property* operator[] (std::size_t index) const;

    [[nodiscard]] inline bool isModified() const;
    inline void setModifiedFlag(bool flag);

private:
    Property::Types g_type{Property::PTYPE_NULL};
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

template<class T>
class PropertyClassWrapperType : public PropertyClassWrapper
{
    static_assert(std::negation<std::is_base_of<fge::Property, T>>::value, "fge::PropertyClassWrapperType<T>, T must not be based on fge::Property class type !");
    static_assert(std::negation<std::is_pointer<T>>::value, "fge::PropertyClassWrapperType<T>, T must not be a pointer !");
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
