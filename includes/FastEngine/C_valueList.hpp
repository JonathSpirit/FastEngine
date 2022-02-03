#ifndef _FGE_C_VALUELIST_HPP_INCLUDED
#define _FGE_C_VALUELIST_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_value.hpp>
#include <string>
#include <unordered_map>

namespace fge
{

class FGE_API ValueList
{
public:
    using ValueListType = std::unordered_map<std::string, fge::Value>;

    ValueList() = default;
    ~ValueList() = default;

    ValueList(const fge::ValueList& vl);
    ValueList(fge::ValueList& vl);
    ValueList(const fge::ValueList&& vl);
    ValueList(fge::ValueList&& vl);

    fge::ValueList& operator=(const fge::ValueList& vl);
    fge::ValueList& operator=(fge::ValueList& vl);
    fge::ValueList& operator=(const fge::ValueList&& vl);
    fge::ValueList& operator=(fge::ValueList&& vl);

    void delAllValues();
    void delValue(const std::string& vname);

    bool checkValue(const std::string& vname) const;

    void setValue(const std::string& vname, fge::Value& value);
    void setValue(const std::string& vname, const fge::Value& value);
    void setValue(const std::string& vname, fge::Value&& value);
    void setValue(const std::string& vname, const fge::Value&& value);

    template <typename T>
    T* getValueType(const std::string& vname);
    template <typename T>
    const T* getValueType(const std::string& vname) const;

    fge::Value& getValue(const std::string& vname);
    const fge::Value& getValue(const std::string& vname) const;

    fge::Value& operator[] (const std::string& vname);
    const fge::Value& operator[] (const std::string& vname) const;

    std::size_t getValueSize() const;

    fge::ValueList::ValueListType::iterator begin();
    fge::ValueList::ValueListType::iterator end();
    fge::ValueList::ValueListType::const_iterator cbegin();
    fge::ValueList::ValueListType::const_iterator cend();

    fge::ValueList::ValueListType::const_iterator find(const std::string& vname) const;
    fge::ValueList::ValueListType::iterator find(const std::string& vname);

    void clearAllModificationFlags();
    std::size_t countAllModificationFlags() const;

private:
    fge::ValueList::ValueListType g_data;
};

}//end fge

#include <FastEngine/C_valueList.inl>

#endif // _FGE_C_VALUELIST_HPP_INCLUDED
