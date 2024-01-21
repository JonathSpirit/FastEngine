/*
 * Copyright 2024 Guillaume Guillet
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

namespace fge
{

//CallbackFunctor

template<class... Types>
CallbackFunctor<Types...>::CallbackFunctor(fge::CallbackFunctor<Types...>::CallbackFunction func) :
        g_function(func)
{}

template<class... Types>
void CallbackFunctor<Types...>::call(Types... args)
{
    this->g_function(args...);
}
template<class... Types>
bool CallbackFunctor<Types...>::check(void* ptr)
{
    return this->g_function == reinterpret_cast<fge::CallbackFunctor<Types...>::CallbackFunction>(ptr);
}

//CallbackObjectFunctor

template<class TObject, class... Types>
CallbackObjectFunctor<TObject, Types...>::CallbackObjectFunctor(
        fge::CallbackObjectFunctor<TObject, Types...>::CallbackFunctionObject func,
        TObject* object) :
        g_functionObj(func),
        g_object(object)
{}

template<class TObject, class... Types>
void CallbackObjectFunctor<TObject, Types...>::call(Types... args)
{
    ((this->g_object)->*(this->g_functionObj))(args...);
}

template<class TObject, class... Types>
bool CallbackObjectFunctor<TObject, Types...>::check(void* ptr)
{
    return this->g_object == reinterpret_cast<TObject*>(ptr);
}

//CallbackLambda

template<class... Types>
template<typename TLambda>
CallbackLambda<Types...>::CallbackLambda(TLambda const& lambda) :
        g_lambda(new TLambda(lambda))
{
    this->g_executeLambda = [](void* lambdaPtr, Types... arguments) {
        return (*reinterpret_cast<TLambda*>(lambdaPtr))(arguments...);
    };
    this->g_deleteLambda = [](void* lambdaPtr) { delete reinterpret_cast<TLambda*>(lambdaPtr); };
}
template<class... Types>
CallbackLambda<Types...>::~CallbackLambda()
{
    (*this->g_deleteLambda)(this->g_lambda);
}

template<class... Types>
void CallbackLambda<Types...>::call(Types... args)
{
    (*this->g_executeLambda)(this->g_lambda, args...);
}
template<class... Types>
bool CallbackLambda<Types...>::check([[maybe_unused]] void* ptr)
{
    return false;
}

//CallbackHandler

template<class... Types>
void CallbackHandler<Types...>::clear()
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    this->detachAll();
    this->g_callees.clear();
}

template<class... Types>
fge::CallbackBase<Types...>* CallbackHandler<Types...>::add(CalleePtr&& callback, fge::Subscriber* subscriber)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    this->attach(subscriber);
    this->g_callees.push_front({std::move(callback), subscriber});
    return this->g_callees.front()._f.get();
}
template<class... Types>
inline fge::CallbackFunctor<Types...>*
CallbackHandler<Types...>::addFunctor(typename fge::CallbackFunctor<Types...>::CallbackFunction func,
                                      fge::Subscriber* subscriber)
{
    return reinterpret_cast<fge::CallbackFunctor<Types...>*>(
            this->add(std::make_unique<fge::CallbackFunctor<Types...>>(func), subscriber));
}
template<class... Types>
template<typename TLambda>
inline fge::CallbackLambda<Types...>* CallbackHandler<Types...>::addLambda(TLambda const& lambda,
                                                                           fge::Subscriber* subscriber)
{
    return reinterpret_cast<fge::CallbackLambda<Types...>*>(
            this->add(std::make_unique<fge::CallbackLambda<Types...>>(lambda), subscriber));
}
template<class... Types>
template<class TObject>
inline fge::CallbackObjectFunctor<TObject, Types...>* CallbackHandler<Types...>::addObjectFunctor(
        typename fge::CallbackObjectFunctor<TObject, Types...>::CallbackFunctionObject func,
        TObject* object,
        Subscriber* subscriber)
{
    return reinterpret_cast<fge::CallbackObjectFunctor<TObject, Types...>*>(
            this->add(std::make_unique<fge::CallbackObjectFunctor<TObject, Types...>>(func, object), subscriber));
}

template<class... Types>
void CallbackHandler<Types...>::delPtr(void* ptr)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    typename fge::CallbackHandler<Types...>::CalleeList::iterator prev = this->g_callees.before_begin();
    for (typename fge::CallbackHandler<Types...>::CalleeList::iterator it = this->g_callees.begin();
         it != this->g_callees.end(); ++it)
    {
        if ((*it)._f->check(ptr))
        {
            if (this->detachOnce((*it)._subscriber) == 0)
            {
                this->g_callees.erase_after(prev);
                break;
            }
            this->g_callees.erase_after(prev);
            it = prev;
        }
        else
        {
            prev = it;
        }
    }
}
template<class... Types>
void CallbackHandler<Types...>::delSub(fge::Subscriber* subscriber)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    typename fge::CallbackHandler<Types...>::CalleeList::iterator prev = this->g_callees.before_begin();
    for (typename fge::CallbackHandler<Types...>::CalleeList::iterator it = this->g_callees.begin();
         it != this->g_callees.end(); ++it)
    {
        if ((*it)._subscriber == subscriber)
        {
            if (this->detachOnce((*it)._subscriber) == 0)
            {
                this->g_callees.erase_after(prev);
                break;
            }
            this->g_callees.erase_after(prev);
            it = prev;
        }
        else
        {
            prev = it;
        }
    }
}
template<class... Types>
void CallbackHandler<Types...>::del(fge::CallbackBase<Types...>* callback)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    typename fge::CallbackHandler<Types...>::CalleeList::iterator prev = this->g_callees.before_begin();
    for (typename fge::CallbackHandler<Types...>::CalleeList::iterator it = this->g_callees.begin();
         it != this->g_callees.end(); ++it)
    {
        if ((*it)._f.get() == callback)
        {
            this->detachOnce((*it)._subscriber);
            this->g_callees.erase_after(prev);
            it = prev;
        }
        else
        {
            prev = it;
        }
    }
}

template<class... Types>
void CallbackHandler<Types...>::call(Types... args)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    auto itCalleeNext = this->g_callees.begin();
    for (auto itCallee = this->g_callees.begin(); itCallee != this->g_callees.end(); itCallee = itCalleeNext)
    {
        ++itCalleeNext;
        itCallee->_f->call(std::forward<Types>(args)...);
    }
}

template<class... Types>
void CallbackHandler<Types...>::hook(fge::CallbackHandler<Types...>& handler, fge::Subscriber* subscriber)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    this->add(new fge::CallbackLambda<Types...>(
                      [&handler](Types... args) { handler.call(std::forward<Types>(args)...); }),
              subscriber);
}

template<class... Types>
void CallbackHandler<Types...>::onDetach(fge::Subscriber* subscriber)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    typename fge::CallbackHandler<Types...>::CalleeList::iterator prev = this->g_callees.before_begin();
    for (typename fge::CallbackHandler<Types...>::CalleeList::iterator it = this->g_callees.begin();
         it != this->g_callees.end(); ++it)
    {
        if ((*it)._subscriber == subscriber)
        {
            this->g_callees.erase_after(prev);
            it = prev;
        }
        else
        {
            prev = it;
        }
    }
}

} // namespace fge
