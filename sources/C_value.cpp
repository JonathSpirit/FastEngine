#include "FastEngine/C_value.hpp"
#include "FastEngine/extra_string.hpp"

namespace fge
{

///Value

FGE_API Value::Value() :
    g_valueObj(nullptr),
    g_isModified(false)
{
}

//Copy/Move constructor
FGE_API Value::Value(fge::Value& val) :
    g_valueObj(val.g_valueObj.get()->copy()),
    g_isModified(true)
{
}
FGE_API Value::Value(const fge::Value& val) :
    g_valueObj(val.g_valueObj.get()->copy()),
    g_isModified(true)
{
}
FGE_API Value::Value(fge::Value&& val) :
    g_valueObj(std::move(val.g_valueObj)),
    g_isModified(true)
{
    val.clear();
}
FGE_API Value::Value(const fge::Value&& val) :
    g_valueObj(val.g_valueObj.get()->copy()),
    g_isModified(true)
{
}

//Special string copy constructor
FGE_API Value::Value(const char* val) :
    g_valueObj( new fge::ValueObj<std::string>( std::move(std::string(val)) ) ),
    g_isModified(true)
{
}
//Special string copy operator
fge::Value& FGE_API Value::operator= (const char* val)
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
        this->g_valueObj.reset( new fge::ValueObj<std::string>( std::move(std::string(val)) ) );
        this->g_isModified = true;
    }
    return *this;
}

//Copy/Move operator
fge::Value& FGE_API Value::operator= (fge::Value& val)
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
fge::Value& FGE_API Value::operator= (const fge::Value& val)
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
fge::Value& FGE_API Value::operator= (fge::Value&& val)
{
    this->g_valueObj.reset( val.g_valueObj.release() );
    this->g_isModified = true;
    return *this;
}
fge::Value& FGE_API Value::operator= (const fge::Value&& val)
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

void FGE_API Value::clear()
{
    this->g_valueObj.reset();
}

bool FGE_API Value::operator== (const fge::Value& value) const
{
    if ( (this->g_valueObj != nullptr) && (value.g_valueObj != nullptr) )
    {
        return (*this->g_valueObj.get()) == (*value.g_valueObj.get());
    }
    return (this->g_valueObj==nullptr) && (this->g_valueObj==nullptr);
}

bool FGE_API Value::set(const char* val)
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
        this->g_valueObj.reset( new fge::ValueObj<std::string>( std::move(std::string(val)) ) );
        this->g_isModified = true;
        return true;
    }
    return false;
}

bool FGE_API Value::set(fge::Value& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->g_valueObj.get()->tryToSet( val.g_valueObj.get() ) )
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
bool FGE_API Value::set(const fge::Value& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->g_valueObj.get()->tryToSet( val.g_valueObj.get() ) )
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
bool FGE_API Value::set(fge::Value&& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->getType() == val.getType() )
        {
            this->g_valueObj.reset( val.g_valueObj.release() );
            this->g_isModified = true;
            return true;
        }
    }
    else
    {
        this->g_valueObj.reset( val.g_valueObj.release() );
        this->g_isModified = true;
        return true;
    }
    return false;
}
bool FGE_API Value::set(const fge::Value&& val)
{
    if ( this->g_valueObj != nullptr )
    {
        if ( this->g_valueObj.get()->tryToSet( val.g_valueObj.get() ) )
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

//Value array control
fge::ValueArray& FGE_API Value::setArrayType()
{
    if ( this->getType() != typeid(fge::ValueArray) )
    {
        this->g_valueObj.reset( new fge::ValueObj<fge::ValueArray>() );
        this->g_isModified = true;
    }
    return fge::ValueObj<fge::ValueArray>::CastPtr( this->g_valueObj.get() )->_data;
}

bool FGE_API Value::resize(std::size_t n)
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data.resize(n);
        return true;
    }
    return false;
}
bool FGE_API Value::reserve(std::size_t n)
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data.reserve(n);
        return true;
    }
    return false;
}

bool FGE_API Value::addData(const fge::Value& value)
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
            fge::ValueObj<fge::ValueArray>* buffObj = new fge::ValueObj<fge::ValueArray>();
            this->g_valueObj.reset( buffObj );
            buffObj->_data.push_back(value);
            return true;
        }
    }
    return false;
}
bool FGE_API Value::addData(fge::Value&& value)
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
            fge::ValueObj<fge::ValueArray>* buffObj = new fge::ValueObj<fge::ValueArray>();
            this->g_valueObj.reset( buffObj );
            buffObj->_data.push_back( std::move(value) );
            return true;
        }
    }
    return false;
}

bool FGE_API Value::setData(std::size_t index, const fge::Value& value)
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
bool FGE_API Value::setData(std::size_t index, fge::Value&& value)
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

const fge::Value* FGE_API Value::getData(std::size_t index) const
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
fge::Value* FGE_API Value::getData(std::size_t index)
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

std::size_t FGE_API Value::getDataSize() const
{
    if ( this->getType() == typeid(fge::ValueArray) )
    {//Is an array
        return fge::ValueObj<fge::ValueArray>::CastPtr(this->g_valueObj.get())->_data.size();
    }
    return 0;
}

const fge::Value* FGE_API Value::operator[] (std::size_t index) const
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
fge::Value* FGE_API Value::operator[] (std::size_t index)
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

std::string FGE_API Value::toString() const
{
    return (this->g_valueObj!=nullptr) ? this->g_valueObj->toString() : "";
}

//extra

void FGE_API Value::setObj(std::unique_ptr<fge::ValueObjBase>&& valueObj)
{
    this->g_valueObj = std::move(valueObj);
    this->g_isModified = true;
}
const std::unique_ptr<fge::ValueObjBase>& FGE_API Value::getObj() const
{
    return this->g_valueObj;
}

}//end fge
