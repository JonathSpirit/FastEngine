/*
 * Copyright 2026 Guillaume Guillet
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

template<class TReturn, class... Types>
CallbackFunctor<TReturn, Types...>::CallbackFunctor(CallbackFunction func) :
        g_function(func)
{}

template<class TReturn, class... Types>
TReturn CallbackFunctor<TReturn, Types...>::call(Types... args)
{
    return this->g_function(args...);
}
template<class TReturn, class... Types>
bool CallbackFunctor<TReturn, Types...>::check(void* ptr)
{
    return this->g_function == reinterpret_cast<CallbackFunction>(ptr);
}

//CallbackObjectFunctor

template<class TReturn, class TObject, class... Types>
CallbackObjectFunctor<TReturn, TObject, Types...>::CallbackObjectFunctor(CallbackFunctionObject func, TObject* object) :
        g_functionObj(func),
        g_object(object)
{}

template<class TReturn, class TObject, class... Types>
TReturn CallbackObjectFunctor<TReturn, TObject, Types...>::call(Types... args)
{
    return ((this->g_object)->*(this->g_functionObj))(args...);
}

template<class TReturn, class TObject, class... Types>
bool CallbackObjectFunctor<TReturn, TObject, Types...>::check(void* ptr)
{
    return this->g_object == reinterpret_cast<TObject*>(ptr);
}

//CallbackLambda

template<class TReturn, class... Types>
template<typename TLambda>
CallbackLambda<TReturn, Types...>::CallbackLambda(TLambda const& lambda) :
        g_lambda(new TLambda(lambda))
{
    this->g_executeLambda = [](void* lambdaPtr, [[maybe_unused]] Types... arguments) {
        if constexpr (std::is_invocable_v<TLambda, Types...>)
        {
            return (*reinterpret_cast<TLambda*>(lambdaPtr))(arguments...);
        }
        else
        {
            return (*reinterpret_cast<TLambda*>(lambdaPtr))();
        }
    };
    this->g_deleteLambda = [](void* lambdaPtr) { delete reinterpret_cast<TLambda*>(lambdaPtr); };
}
template<class TReturn, class... Types>
CallbackLambda<TReturn, Types...>::~CallbackLambda()
{
    (*this->g_deleteLambda)(this->g_lambda);
}

template<class TReturn, class... Types>
TReturn CallbackLambda<TReturn, Types...>::call(Types... args)
{
    return (*this->g_executeLambda)(this->g_lambda, args...);
}
template<class TReturn, class... Types>
bool CallbackLambda<TReturn, Types...>::check([[maybe_unused]] void* ptr)
{
    return false;
}

//CallbackHandler

template<class... Types>
void CallbackHandler<Types...>::clear()
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    this->detachAll();
    for (auto& callee: this->g_callees)
    {
        callee._markedForDeletion = true;
    }
}

template<class... Types>
fge::CallbackBase<void, Types...>* CallbackHandler<Types...>::add(CalleePtr&& callback, fge::Subscriber* subscriber)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);

    this->attach(subscriber);

    //Search for a "mark for deletion" callee and replace it with the new one
    for (auto& callee: this->g_callees)
    {
        if (callee._markedForDeletion)
        {
            //TODO: _markedForDeletion has been added in order to avoid undefined behavior when a callback remove itself
            // while being called. But here we do the same mistake, we replace a callee that could be called at the same time.
            // We need a better solution.
            callee._f = std::move(callback);
            callee._subscriber = subscriber;
            callee._markedForDeletion = false;
            return callee._f.get();
        }
    }

    //Emplace back the new callee if there is no "mark for deletion"
    this->g_callees.emplace_back(std::move(callback), subscriber);
    return this->g_callees.back()._f.get();
}
template<class... Types>
inline fge::CallbackFunctor<void, Types...>*
CallbackHandler<Types...>::addFunctor(typename fge::CallbackFunctor<void, Types...>::CallbackFunction func,
                                      fge::Subscriber* subscriber)
{
    return reinterpret_cast<fge::CallbackFunctor<void, Types...>*>(
            this->add(StaticHelpers::newFunctor(func), subscriber));
}
template<class... Types>
template<typename TLambda>
inline fge::CallbackLambda<void, Types...>* CallbackHandler<Types...>::addLambda(TLambda const& lambda,
                                                                                 fge::Subscriber* subscriber)
{
    return reinterpret_cast<fge::CallbackLambda<void, Types...>*>(
            this->add(StaticHelpers::newLambda(lambda), subscriber));
}
template<class... Types>
template<class TObject>
inline fge::CallbackObjectFunctor<void, TObject, Types...>* CallbackHandler<Types...>::addObjectFunctor(
        typename fge::CallbackObjectFunctor<void, TObject, Types...>::CallbackFunctionObject func,
        TObject* object,
        Subscriber* subscriber)
{
    return reinterpret_cast<fge::CallbackObjectFunctor<void, TObject, Types...>*>(
            this->add(StaticHelpers::newObjectFunctor(func, object), subscriber));
}

template<class... Types>
void CallbackHandler<Types...>::delPtr(void* ptr)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    for (auto& callee: this->g_callees)
    {
        if (callee._markedForDeletion)
        {
            continue;
        }

        if (callee._f->check(ptr))
        {
            this->detachOnce(callee._subscriber);
            callee._markedForDeletion = true;
        }
    }
}
template<class... Types>
void CallbackHandler<Types...>::delSub(fge::Subscriber* subscriber)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    for (auto& callee: this->g_callees)
    {
        if (callee._markedForDeletion)
        {
            continue;
        }

        if (callee._subscriber == subscriber)
        {
            if (this->detachOnce(callee._subscriber) == 0)
            { //At this point, all subscribers are detached so we can end the loop
                callee._markedForDeletion = true;
                break;
            }
            callee._markedForDeletion = true;
        }
    }
}
template<class... Types>
void CallbackHandler<Types...>::del(fge::CallbackBase<void, Types...>* callback)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    for (auto& callee: this->g_callees)
    {
        if (callee._markedForDeletion)
        {
            continue;
        }

        if (callee._f.get() == callback)
        {
            this->detachOnce(callee._subscriber);
            this->g_callees._markedForDeletion = true;
        }
    }
}

template<class... Types>
void CallbackHandler<Types...>::call(Types... args)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);

    std::size_t eraseCount = 0;
    for (std::size_t i = 0; i < this->g_callees.size(); ++i)
    {
        if (this->g_callees[i]._markedForDeletion)
        {
            ++eraseCount;
            continue;
        }

        if (eraseCount > 0)
        { //We found a valid callee, we can erase the previous ones
            this->g_callees.erase(this->g_callees.begin() + i - eraseCount, this->g_callees.begin() + i);
            i -= eraseCount;
            eraseCount = 0;
        }

        this->g_callees[i]._f->call(std::forward<Types>(args)...);

        //While calling, the callee can remove itself or others
        //Every erased callee is flagged (mark for deletion)
        //In order to avoid calling something that is not valid anymore
    }
}

template<class... Types>
void CallbackHandler<Types...>::hook(fge::CallbackHandler<Types...>& handler, fge::Subscriber* subscriber)
{
    if (this == &handler)
    { //Can't hook itself
        return;
    }

    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    this->add(new fge::CallbackLambda<Types...>(
                      [&handler](Types... args) { handler.call(std::forward<Types>(args)...); }),
              subscriber);
}

template<class... Types>
void CallbackHandler<Types...>::onDetach(fge::Subscriber* subscriber)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    for (auto& callee: this->g_callees)
    {
        if (callee._markedForDeletion)
        {
            continue;
        }

        if (callee._subscriber == subscriber)
        {
            callee._markedForDeletion = true;
        }
    }
}

//UniqueCallbackHandler

template<class... Types>
void UniqueCallbackHandler<Types...>::clear()
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    this->g_callee._f = nullptr;
    this->g_callee._subscriber = nullptr;
}

template<class... Types>
fge::CallbackBase<void, Types...>* UniqueCallbackHandler<Types...>::set(CalleePtr&& callback,
                                                                        fge::Subscriber* subscriber)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);

    if (subscriber != this->g_callee._subscriber)
    {
        this->detachOnce(this->g_callee._subscriber);
        this->attach(subscriber);
    }

    this->g_callee._f = std::move(callback);
    this->g_callee._subscriber = subscriber;
    return this->g_callee._f.get();
}
template<class... Types>
inline fge::CallbackFunctor<void, Types...>*
UniqueCallbackHandler<Types...>::setFunctor(typename fge::CallbackFunctor<void, Types...>::CallbackFunction func,
                                            fge::Subscriber* subscriber)
{
    return reinterpret_cast<fge::CallbackFunctor<void, Types...>*>(
            this->set(StaticHelpers::newFunctor(func), subscriber));
}
template<class... Types>
template<typename TLambda>
inline fge::CallbackLambda<void, Types...>* UniqueCallbackHandler<Types...>::setLambda(TLambda const& lambda,
                                                                                       fge::Subscriber* subscriber)
{
    return reinterpret_cast<fge::CallbackLambda<void, Types...>*>(
            this->set(StaticHelpers::newLambda(lambda), subscriber));
}
template<class... Types>
template<class TObject>
inline fge::CallbackObjectFunctor<void, TObject, Types...>* UniqueCallbackHandler<Types...>::setObjectFunctor(
        typename fge::CallbackObjectFunctor<void, TObject, Types...>::CallbackFunctionObject func,
        TObject* object,
        Subscriber* subscriber)
{
    return reinterpret_cast<fge::CallbackObjectFunctor<void, TObject, Types...>*>(
            this->set(StaticHelpers::newObjectFunctor(func, object), subscriber));
}

template<class... Types>
void UniqueCallbackHandler<Types...>::delPtr(void* ptr)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);

    if (this->g_callee._f->check(ptr))
    {
        this->detachOnce(this->g_callee._subscriber);
        this->g_callee._f = nullptr;
        this->g_callee._subscriber = nullptr;
    }
}
template<class... Types>
void UniqueCallbackHandler<Types...>::delSub(fge::Subscriber* subscriber)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);

    if (this->g_callee._subscriber == subscriber)
    {
        this->detachOnce(this->g_callee._subscriber);
        this->g_callee._f = nullptr;
        this->g_callee._subscriber = nullptr;
    }
}
template<class... Types>
void UniqueCallbackHandler<Types...>::del(fge::CallbackBase<Types...>* callback)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);

    if (this->g_callee._f.get() == callback)
    {
        this->detachOnce(this->g_callee._subscriber);
        this->g_callee._f = nullptr;
        this->g_callee._subscriber = nullptr;
    }
}

template<class... Types>
void UniqueCallbackHandler<Types...>::call(Types... args)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);

    if (this->g_callee._f == nullptr)
    {
        return;
    }

    auto unique = std::move(this->g_callee._f);
    unique->call(std::forward<Types>(args)...);
    this->g_callee._f = std::move(unique);

    //TODO :
    // While calling, the callee can remove itself
    // So we move the unique pointer in order to keep the callee alive during the call
    // They should be a better solution
}

template<class... Types>
void UniqueCallbackHandler<Types...>::onDetach(fge::Subscriber* subscriber)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    if (this->g_callee._subscriber == subscriber)
    {
        this->g_callee._f = nullptr;
        this->g_callee._subscriber = nullptr;
    }
}

} // namespace fge
