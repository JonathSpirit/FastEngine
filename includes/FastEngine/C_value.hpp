#ifndef _FGE_C_VALUE_HPP_INCLUDED
#define _FGE_C_VALUE_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/extra_string.hpp>
#include <memory>
#include <string>
#include <vector>

namespace fge
{

class Value;

using ValueArray = std::vector<fge::Value>;

class ValueObjBase
{
public:
    inline ValueObjBase() = default;

    virtual std::string toString() const = 0;

    virtual const std::type_info& getType() const = 0;

    virtual fge::ValueObjBase* copy() const = 0;

    virtual bool tryToSet(const fge::ValueObjBase* val) = 0;
    virtual bool tryToSet(void* val, const std::type_info& type, bool isArithmetic, bool isFloatingPoint, bool isSigned) = 0;

    virtual bool operator== (const fge::ValueObjBase& value) const = 0;
};

template <class T>
class ValueObj : public fge::ValueObjBase
{
public:
    static_assert(std::negation<std::is_base_of<fge::Value, T>>::value, "fge::ValueObj<T>, T must not be based on fge::Value class type !");

    ValueObj() = default;

    ValueObj(T& val);
    ValueObj(const T& val);
    ValueObj(T&& val);
    ValueObj(const T&& val);

    ValueObj(const fge::ValueObj<T>& val);
    ValueObj(fge::ValueObj<T>&& val);

    ~ValueObj() = default;

    fge::ValueObj<T>& operator= (const fge::ValueObj<T>& val);
    fge::ValueObj<T>& operator= (fge::ValueObj<T>&& val);

    std::string toString() const override;

    const std::type_info& getType() const override;
    static const std::type_info& GetType();

    fge::ValueObjBase* copy() const override;

    bool tryToSet(const fge::ValueObjBase* val) override;
    bool tryToSet(void* val, const std::type_info& type, bool isArithmetic, bool isFloatingPoint, bool isSigned) override;

    bool operator== (const fge::ValueObjBase& value) const override;

    static inline fge::ValueObj<T>* CastPtr(fge::ValueObjBase* n);

    T _data;
};

class FGE_API Value
{
public:
    Value();

    //Copy/Move constructor
    Value(fge::Value& val);
    Value(const fge::Value& val);
    Value(fge::Value&& val);
    Value(const fge::Value&& val);

    //Copy/Move some type constructor
    template<typename T>
    Value(T& val);
    template<typename T>
    Value(const T& val);
    template<typename T>
    Value(T&& val);
    template<typename T>
    Value(const T&& val);

    //Special string copy constructor
    Value(const char* val);
    //Special string copy operator
    fge::Value& operator= (const char* val);

    //Copy/Move operator
    fge::Value& operator= (fge::Value& val);
    fge::Value& operator= (const fge::Value& val);
    fge::Value& operator= (fge::Value&& val);
    fge::Value& operator= (const fge::Value&& val);

    void clear();

    bool operator== (const fge::Value& value) const;

    template<typename T>
    T& setType();

    //Setter
    template<typename T>
    bool set(T& val);
    template<typename T>
    bool set(const T& val);
    template<typename T>
    bool set(T&& val);
    template<typename T>
    bool set(const T&& val);

    bool set(const char* val);

    bool set(fge::Value& val);
    bool set(const fge::Value& val);
    bool set(fge::Value&& val);
    bool set(const fge::Value&& val);

    template<typename T>
    fge::Value& operator= (T& val);
    template<typename T>
    fge::Value& operator= (const T& val);
    template<typename T>
    fge::Value& operator= (T&& val);
    template<typename T>
    fge::Value& operator= (const T&& val);

    //Getter
    template<typename T>
    bool get(T& valBuff) const;
    template<typename T>
    T* get();
    template<typename T>
    const T* get() const;

    inline const std::type_info& getType() const;

    //Value array control
    fge::ValueArray& setArrayType();

    bool resize(std::size_t n);
    bool reserve(std::size_t n);

    bool addData(const fge::Value& value);
    bool addData(fge::Value&& value);

    template<typename T>
    bool addType();

    bool setData(std::size_t index, const fge::Value& value);
    bool setData(std::size_t index, fge::Value&& value);

    const fge::Value* getData(std::size_t index) const;
    fge::Value* getData(std::size_t index);

    template<typename T>
    bool getData(std::size_t index, T& valBuff) const;
    template<typename T>
    const T* getData(std::size_t index) const;
    template<typename T>
    T* getData(std::size_t index);

    std::size_t getDataSize() const;

    const fge::Value* operator[] (std::size_t index) const;
    fge::Value* operator[] (std::size_t index);

    std::string toString() const;

    //Extra
    void setObj(std::unique_ptr<fge::ValueObjBase>&& valueObj);
    const std::unique_ptr<fge::ValueObjBase>& getObj() const;

    inline bool isModified() const;
    inline void setModifiedFlag(bool flag);

private:
    std::unique_ptr<fge::ValueObjBase> g_valueObj;
    bool g_isModified;
};

}//end fge

#include <FastEngine/C_value.inl>

#endif // _FGE_C_VALUE_HPP_INCLUDED
