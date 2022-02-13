#include "FastEngine/C_value.hpp"

#include <memory>
#include <utility>
#include "FastEngine/extra_string.hpp"

namespace fge
{

///Value

Value::Value() :
    g_valueObj(nullptr),
    g_isModified(false)
{
}

//Copy/Move constructor
Value::Value(fge::Value& val) :
    g_valueObj(val.g_valueObj->copy()),
    g_isModified(true)
{
}
Value::Value(const fge::Value& val) :
    g_valueObj(val.g_valueObj->copy()),
    g_isModified(true)
{
}
Value::Value(fge::Value&& val) :
    g_valueObj(std::move(val.g_valueObj)),
    g_isModified(true)
{
    val.clear();
}
Value::Value(const fge::Value&& val) :
    g_valueObj(val.g_valueObj->copy()),
    g_isModified(true)
{
}

//Special string copy constructor
Value::Value(const char* val) :
    g_valueObj( new fge::ValueObj<std::string>( std::move(std::string(val)) ) ),
    g_isModified(true)
{
}
//Special string copy operator
fge::Value& Value::operator= (const char* val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->g_valueObj->getType() == typeid(std::string) )
        {
            fge::ValueObj<std::string>::CastPtr(this->g_valueObj.get())->_data = std::move(std::string(val));
            this->g_isModified = true;
        }
    }
    else
    {
        this->g_valueObj = std::make_unique<fge::ValueObj<std::string>>( std::move(std::string(val)) );
        this->g_isModified = true;
    }
    return *this;
}

//Copy/Move operator
fge::Value& Value::operator= (fge::Value& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( !this->g_valueObj.get()->tryToSet( val.g_valueObj.get() ) )
        {
            this->g_valueObj.reset( val.g_valueObj.get()->copy() );
        }
    }
    else
    {
        this->g_valueObj.reset( val.g_valueObj.get()->copy() );
    }
    this->g_isModified = true;
    return *this;
}
fge::Value& Value::operator= (const fge::Value& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( !this->g_valueObj.get()->tryToSet( val.g_valueObj.get() ) )
        {
            this->g_valueObj.reset( val.g_valueObj.get()->copy() );
        }
    }
    else
    {
        this->g_valueObj.reset( val.g_valueObj.get()->copy() );
    }
    this->g_isModified = true;
    return *this;
}
fge::Value& Value::operator= (fge::Value&& val)
{
    this->g_valueObj = std::move(val.g_valueObj);
    this->g_isModified = true;
    return *this;
}
fge::Value& Value::operator= (const fge::Value&& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( !this->g_valueObj->tryToSet( val.g_valueObj.get() ) )
        {
            this->g_valueObj.reset( val.g_valueObj->copy() );
        }
    }
    else
    {
        this->g_valueObj.reset( val.g_valueObj->copy() );
    }
    this->g_isModified = true;
    return *this;
}

void Value::clear()
{
    this->g_valueObj.reset();
}

bool Value::operator== (const fge::Value& value) const
{
    if ( (this->g_valueObj != nullptr) && (value.g_valueObj != nullptr) )
    {
        return (*this->g_valueObj) == (*value.g_valueObj);
    }
    return (this->g_valueObj==nullptr) && (this->g_valueObj==nullptr);
}

bool Value::set(const char* val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->g_valueObj->getType() == typeid(std::string) )
        {
            fge::ValueObj<std::string>::CastPtr(this->g_valueObj.get())->_data = std::move(std::string(val));
            this->g_isModified = true;
            return true;
        }
    }
    else
    {
        this->g_valueObj = std::make_unique<fge::ValueObj<std::string>>( std::move(std::string(val)) );
        this->g_isModified = true;
        return true;
    }
    return false;
}

bool Value::set(fge::Value& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->g_valueObj->tryToSet( val.g_valueObj.get() ) )
        {
            this->g_isModified = true;
            return true;
        }
    }
    else
    {
        this->g_valueObj.reset( val.g_valueObj->copy() );
        this->g_isModified = true;
        return true;
    }
    return false;
}
bool Value::set(const fge::Value& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->g_valueObj->tryToSet( val.g_valueObj.get() ) )
        {
            this->g_isModified = true;
            return true;
        }
    }
    else
    {
        this->g_valueObj.reset( val.g_valueObj.get()->copy() );
        this->g_isModified = true;
        return true;
    }
    return false;
}
bool Value::set(fge::Value&& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->getType() == val.getType() )
        {
            this->g_valueObj = std::move(val.g_valueObj);
            this->g_isModified = true;
            return true;
        }
    }
    else
    {
        this->g_valueObj = std::move(val.g_valueObj);
        this->g_isModified = true;
        return true;
    }
    return false;
}
bool Value::set(const fge::Value&& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->g_valueObj->tryToSet( val.g_valueObj.get() ) )
        {
            this->g_isModified = true;
            return true;
        }
    }
    else
    {
        this->g_valueObj.reset( val.g_valueObj->copy() );
        this->g_isModified = true;
        return true;
    }
    return false;
}

//Value array control
fge::ValueArray& Value::setArrayType()
{
    if ( this->getType() != typeid(fge::ValueArray) )
    {
        this->g_valueObj = std::make_unique<fge::ValueObj<fge::ValueArray>>( );
        this->g_isModified = true;
    }
    return fge::ValueObj<fge::ValueArray>::CastPtr( this->g_valueObj.get() )->_data;
}

bool Value::resize(std::size_t n)
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data.resize(n);
        return true;
    }
    return false;
}
bool Value::reserve(std::size_t n)
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data.reserve(n);
        return true;
    }
    return false;
}

bool Value::addData(const fge::Value& value)
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is already an array
        fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data.push_back(value);
        return true;
    }
    else
    {//Is not an array
        if (this->g_valueObj == nullptr)
        {//Is null
            auto* buffObj = new fge::ValueObj<fge::ValueArray>();
            this->g_valueObj.reset( buffObj );
            buffObj->_data.push_back(value);
            return true;
        }
    }
    return false;
}
bool Value::addData(fge::Value&& value)
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is already an array
        fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data.push_back(std::move(value));
        return true;
    }
    else
    {//Is not an array
        if (this->g_valueObj == nullptr)
        {//Is null
            auto* buffObj = new fge::ValueObj<fge::ValueArray>();
            this->g_valueObj.reset( buffObj );
            buffObj->_data.push_back( std::move(value) );
            return true;
        }
    }
    return false;
}

bool Value::setData(std::size_t index, const fge::Value& value)
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueArray& buff = fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data;
        if ( index < buff.size() )
        {//In bound
            return buff[index].set(value);
        }
    }
    return false;
}
bool Value::setData(std::size_t index, fge::Value&& value)
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueArray& buff = fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data;
        if ( index < buff.size() )
        {//In bound
            return buff[index].set(std::move(value));
        }
    }
    return false;
}

const fge::Value* Value::getData(std::size_t index) const
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueArray& buff = fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data;
        if ( index < buff.size() )
        {
            return &buff[index];
        }
    }
    return nullptr;
}
fge::Value* Value::getData(std::size_t index)
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueArray& buff = fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data;
        if ( index < buff.size() )
        {
            return &buff[index];
        }
    }
    return nullptr;
}

std::size_t Value::getDataSize() const
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        return fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data.size();
    }
    return 0;
}

const fge::Value* Value::operator[] (std::size_t index) const
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueArray& buff = fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data;
        if ( index < buff.size() )
        {
            return &buff[index];
        }
    }
    return nullptr;
}
fge::Value* Value::operator[] (std::size_t index)
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueArray& buff = fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data;
        if ( index < buff.size() )
        {
            return &buff[index];
        }
    }
    return nullptr;
}

std::string Value::toString() const
{
    return (this->g_valueObj!=nullptr) ? this->g_valueObj->toString() : "";
}

//extra

void Value::setObj(std::unique_ptr<fge::ValueObjBase>&& valueObj)
{
    this->g_valueObj = std::move(valueObj);
    this->g_isModified = true;
}
const std::unique_ptr<fge::ValueObjBase>& Value::getObj() const
{
    return this->g_valueObj;
}

}//end fge
