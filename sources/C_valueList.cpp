#include "FastEngine/C_valueList.hpp"

namespace fge
{

FGE_API ValueList::ValueList(const fge::ValueList& vl) :
    g_data(vl.g_data)
{
}
FGE_API ValueList::ValueList(fge::ValueList& vl) :
    g_data(vl.g_data)
{
}
FGE_API ValueList::ValueList(const fge::ValueList&& vl) :
    g_data(std::move(vl.g_data))
{
}
FGE_API ValueList::ValueList(fge::ValueList&& vl) :
    g_data(std::move(vl.g_data))
{
}

fge::ValueList& FGE_API ValueList::operator=(const fge::ValueList& vl)
{
    this->g_data = vl.g_data;
    return *this;
}
fge::ValueList& FGE_API ValueList::operator=(fge::ValueList& vl)
{
    this->g_data = vl.g_data;
    return *this;
}
fge::ValueList& FGE_API ValueList::operator=(const fge::ValueList&& vl)
{
    this->g_data = std::move(vl.g_data);
    return *this;
}
fge::ValueList& FGE_API ValueList::operator=(fge::ValueList&& vl)
{
    this->g_data = std::move(vl.g_data);
    return *this;
}

void FGE_API ValueList::delAllValues()
{
    this->g_data.clear();
}
void FGE_API ValueList::delValue(const std::string& vname)
{
    this->g_data.erase(vname);
}

bool FGE_API ValueList::checkValue(const std::string& vname) const
{
    return this->g_data.count(vname) > 0;
}

void FGE_API ValueList::setValue(const std::string& vname, fge::Value& value)
{
    this->g_data[vname] = value;
}
void FGE_API ValueList::setValue(const std::string& vname, const fge::Value& value)
{
    this->g_data[vname] = value;
}
void FGE_API ValueList::setValue(const std::string& vname, fge::Value&& value)
{
    this->g_data[vname] = std::move(value);
}
void FGE_API ValueList::setValue(const std::string& vname, const fge::Value&& value)
{
    this->g_data[vname] = std::move(value);
}

fge::Value& FGE_API ValueList::getValue(const std::string& vname)
{
    return this->g_data[vname];
}
const fge::Value& FGE_API ValueList::getValue(const std::string& vname) const
{
    return this->g_data.at(vname);
}

fge::Value& FGE_API ValueList::operator[] (const std::string& vname)
{
    return this->g_data[vname];
}
const fge::Value& FGE_API ValueList::operator[] (const std::string& vname) const
{
    return this->g_data.at(vname);
}

std::size_t FGE_API ValueList::getValueSize() const
{
    return this->g_data.size();
}

fge::ValueList::ValueListType::iterator FGE_API ValueList::begin()
{
    return this->g_data.begin();
}
fge::ValueList::ValueListType::iterator FGE_API ValueList::end()
{
    return this->g_data.end();
}
fge::ValueList::ValueListType::const_iterator FGE_API ValueList::cbegin()
{
    return this->g_data.cbegin();
}
fge::ValueList::ValueListType::const_iterator FGE_API ValueList::cend()
{
    return this->g_data.cend();
}

fge::ValueList::ValueListType::const_iterator FGE_API ValueList::find(const std::string& vname) const
{
    return this->g_data.find(vname);
}
fge::ValueList::ValueListType::iterator FGE_API ValueList::find(const std::string& vname)
{
    return this->g_data.find(vname);
}

void FGE_API ValueList::clearAllModificationFlags()
{
    for (auto& data : this->g_data)
    {
        data.second.setModifiedFlag(false);
    }
}
std::size_t FGE_API ValueList::countAllModificationFlags() const
{
    std::size_t counter = 0;

    for (const auto& data : this->g_data)
    {
        if ( data.second.isModified() )
        {
            ++counter;
        }
    }

    return counter;
}

}//end fge
