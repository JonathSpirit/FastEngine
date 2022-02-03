namespace fge
{

template <typename T>
T* ValueList::getValueType(const std::string& vname)
{
    return this->g_data[vname].get<T>();
}
template <typename T>
const T* ValueList::getValueType(const std::string& vname) const
{
    auto it = this->g_data.find(vname);
    if (it != this->g_data.cend())
    {
        return it->second.get<T>();
    }
    return nullptr;
}

}//end fge
