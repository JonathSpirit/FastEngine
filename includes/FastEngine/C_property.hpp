#ifndef _FGE_C_PROPERTY_HPP_INCLUDED
#define _FGE_C_PROPERTY_HPP_INCLUDED

#include <FastEngine/extra_string.hpp>
#include <string>
#include <vector>
#include <typeinfo>

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
    Property(fge::Property& val);
    Property(const fge::Property& val);
    Property(fge::Property&& val) noexcept;
    Property(const fge::Property&& val);

    //Copy/Move some type constructor
    template<typename T>
    Property(const T& val);
    template<typename T>
    Property(T&& val);

    //Special string copy constructor
    Property(const char* val);

    ~Property();

    void clear();

    bool operator== (const fge::Property& val) const;

    //Copy/Move operator
    inline fge::Property& operator= (fge::Property& val);
    inline fge::Property& operator= (const fge::Property& val);
    inline fge::Property& operator= (fge::Property&& val) noexcept;
    inline fge::Property& operator= (const fge::Property&& val);

    //Copy/Move some type operator
    template<typename T>
    inline fge::Property& operator= (const T& val);
    template<typename T>
    inline fge::Property& operator= (T&& val);

    //Special string copy operator
    inline fge::Property& operator= (const char* val);

    template<typename T>
    T& setType();
    void setType(fge::Property::Types type);
    template<typename T>
    [[nodiscard]] bool isType() const;
    [[nodiscard]] bool isType(fge::Property::Types type) const;

    [[nodiscard]] const std::type_info& getClassType() const;
    [[nodiscard]] inline Property::Types getType() const;
    [[nodiscard]] inline bool isSigned() const;

    [[nodiscard]] std::string toString() const;

    bool set(fge::Property& val);
    bool set(const fge::Property& val);
    bool set(fge::Property&& val);
    bool set(const fge::Property&& val);

    template<class T>
    bool set(const T& val);
    template<class T>
    bool set(T&& val);

    bool set(const char* val);

    template<class T>
    bool get(T& val) const;
    template<class T>
    T get() const;

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

    template<typename T>
    bool pushType();

    bool setData(std::size_t index, const fge::Property& value);
    bool setData(std::size_t index, fge::Property&& value);

    [[nodiscard]] fge::Property* getData(std::size_t index);
    [[nodiscard]] const fge::Property* getData(std::size_t index) const;

    template<typename T>
    bool getData(std::size_t index, T& val) const;
    template<typename T>
    T* getDataPtr(std::size_t index);
    template<typename T>
    const T* getDataPtr(std::size_t index) const;

    [[nodiscard]] std::size_t getDataSize() const;

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
