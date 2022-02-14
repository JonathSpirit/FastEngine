#ifndef _FGE_C_PROPERTYLIST_HPP_INCLUDED
#define _FGE_C_PROPERTYLIST_HPP_INCLUDED

#include <FastEngine/C_property.hpp>
#include <string>
#include <unordered_map>

namespace fge
{

class PropertyList
{
public:
    using PropertyListType = std::unordered_map<std::string, fge::Property>;

    PropertyList() = default;
    PropertyList(const PropertyList& r);
    PropertyList(PropertyList&& r) noexcept;
    ~PropertyList() = default;

    void delAllProperties();
    void delProperty(const std::string& key);

    bool checkProperty(const std::string& key) const;

    void setProperty(const std::string& key, const fge::Property& value);
    void setProperty(const std::string& key, fge::Property&& value);

    template <typename T>
    T* getPropertyType(const std::string& key);
    template <typename T>
    const T* getPropertyType(const std::string& key) const;

    fge::Property& getProperty(const std::string& key);
    const fge::Property& getProperty(const std::string& key) const;

    fge::Property& operator[] (const std::string& key);
    const fge::Property& operator[] (const std::string& key) const;

    std::size_t getPropertiesSize() const;

    fge::PropertyList::PropertyListType::iterator begin();
    fge::PropertyList::PropertyListType::iterator end();
    fge::PropertyList::PropertyListType::const_iterator cbegin();
    fge::PropertyList::PropertyListType::const_iterator cend();

    fge::PropertyList::PropertyListType::const_iterator find(const std::string& key) const;
    fge::PropertyList::PropertyListType::iterator find(const std::string& key);

    void clearAllModificationFlags();
    std::size_t countAllModificationFlags() const;

private:
    fge::PropertyList::PropertyListType g_data;
};

}//end fge

#include <FastEngine/C_propertyList.inl>

#endif // _FGE_C_PROPERTYLIST_HPP_INCLUDED
