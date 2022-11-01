/*
 * Copyright 2022 Guillaume Guillet
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

#ifndef _FGE_C_DATAACCESSOR_HPP_INCLUDED
#define _FGE_C_DATAACCESSOR_HPP_INCLUDED

#include <functional>
#include <memory>

namespace fge
{

template<class T>
struct DataAccessor
{
    DataAccessor() = default;

    explicit DataAccessor(T* directAccessPtr) :
            _setter([directAccessPtr](T&& r){*directAccessPtr = std::forward<T>(r);}),
            _getter([directAccessPtr](){return *directAccessPtr;})
    {}

    DataAccessor(std::function<T()> getter, std::function<void(T)> setter) :
            _setter(std::move(setter)),
            _getter(std::move(getter))
    {}

    DataAccessor(T* directAccessPtrGetter, std::function<void(T)> setter) :
            _setter(std::move(setter)),
            _getter([directAccessPtrGetter](){return *directAccessPtrGetter;})
    {}

    DataAccessor(std::function<T()> getter, T* directAccessPtrSetter) :
            _setter([directAccessPtrSetter](T&& r){*directAccessPtrSetter = std::forward<T>(r);}),
            _getter(std::move(getter))
    {}

    std::function<void(T)> _setter;
    std::function<T()> _getter;
};

}//end fge

#endif //_FGE_C_DATAACCESSOR_HPP_INCLUDED
