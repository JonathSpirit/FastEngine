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

#ifndef _FGE_C_CALLBACKHANDLER_HPP_INCLUDED
#define _FGE_C_CALLBACKHANDLER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_subscription.hpp"
#include <forward_list>
#include <memory>
#include <mutex>

namespace fge
{

/**
 * \class CallbackBase
 * \ingroup callback
 * \brief Base class for callbacks
 *
 * \tparam Types The list of arguments types passed to the callback
 */
template<class... Types>
class CallbackBase
{
public:
    virtual ~CallbackBase() = default;

    virtual void call(Types... args) = 0;
    virtual bool check(void* ptr) = 0;
};

/**
 * \class CallbackFunctor
 * \ingroup callback
 * \brief Callback functor
 *
 * \tparam Types The list of arguments types passed to the functor
 */
template<class... Types>
class CallbackFunctor : public fge::CallbackBase<Types...>
{
public:
    using CallbackFunction = void (*)(Types... args);

    /**
     * \brief Constructor
     *
     * \param func The callback function
     */
    explicit CallbackFunctor(fge::CallbackFunctor<Types...>::CallbackFunction func);
    ~CallbackFunctor() override = default;

    /**
     * \brief Call the callback function with the given arguments
     *
     * \param args The list of arguments
     */
    void call(Types... args) override;
    /**
     * \brief Check if the given pointer is the same as the one used to construct the functor
     *
     * \param ptr The pointer to check
     * \return \b True if the given pointer is the same as the one used to construct the functor
     */
    inline bool check(void* ptr) override;

protected:
    fge::CallbackFunctor<Types...>::CallbackFunction g_function;
};

/**
 * \class CallbackLambda
 * \ingroup callback
 * \brief Callback lambda (with/without capture)
 *
 * \tparam Types The list of arguments types passed to the lambda
 */
template<class... Types>
class CallbackLambda : public fge::CallbackBase<Types...>
{
public:
    /**
     * \brief Constructor
     *
     * \tparam TLambda The lambda type
     * \param lambda The callback lambda
     */
    template<typename TLambda>
    explicit CallbackLambda(TLambda const& lambda);
    ~CallbackLambda() override;

    /**
     * \brief Call the callback function with the given arguments
     *
     * \param args The list of arguments
     */
    void call(Types... args) override;
    /**
     * \brief Always return false
     *
     * \param ptr The pointer to check (unused)
     * \return \b False
     */
    inline bool check(void* ptr) override;

protected:
    void* g_lambda;
    void (*g_executeLambda)(void*, Types...);
    void (*g_deleteLambda)(void*);
};

/**
 * \class CallbackObjectFunctor
 * \ingroup callback
 * \brief Callback functor of a method with an object
 *
 * \tparam Types The list of arguments types passed to the functor
 * \tparam TObject The object type
 */
template<class TObject, class... Types>
class CallbackObjectFunctor : public fge::CallbackBase<Types...>
{
public:
    using CallbackFunctionObject = void (TObject::*)(Types... args);

    /**
     * \brief Constructor
     *
     * \param func The callback method of the object
     * \param object The object pointer
     */
    CallbackObjectFunctor(fge::CallbackObjectFunctor<TObject, Types...>::CallbackFunctionObject func, TObject* object);
    ~CallbackObjectFunctor() override = default;

    /**
     * \brief Call the callback method with the given arguments
     *
     * \param args The list of arguments
     */
    void call(Types... args) override;
    /**
     * \brief Check if the given object pointer is the same as the one used to construct the functor
     *
     * \param ptr The object pointer to check
     * \return \b True if the given object pointer is the same as the one used to construct the functor
     */
    inline bool check(void* ptr) override;

protected:
    fge::CallbackObjectFunctor<TObject, Types...>::CallbackFunctionObject g_functionObj;
    TObject* g_object;
};

/**
 * \class CallbackHandler
 * \ingroup callback
 * \brief This class is used to handle callbacks in a safe way
 *
 * Every callback muse use the same template parameters Types than a handler.
 * This class is thread-safe.
 *
 * This class inherits from Subscription to be able to subscribe to it. When a subscriber is
 * added to a handler and is destroyed, all the callbacks related to this subscriber are automatically removed.
 *
 * \see Subscription
 *
 * \tparam Types The list of arguments types passed to the callbacks
 */
template<class... Types>
class CallbackHandler : public fge::Subscription
{
public:
    using CalleePtr = std::unique_ptr<fge::CallbackBase<Types...>>;

    CallbackHandler() = default;
    ~CallbackHandler() override = default;

    /**
     * \brief Copy constructor that does nothing
     */
    CallbackHandler([[maybe_unused]] fge::CallbackHandler<Types...> const& n) :
            fge::Subscription(){};
    /**
     * \brief Move constructor prohibited
     */
    CallbackHandler(fge::CallbackHandler<Types...>&& n) = delete;

    /**
     * \brief Copy operator that does nothing
     */
    fge::CallbackHandler<Types...>& operator=([[maybe_unused]] fge::CallbackHandler<Types...> const& n)
    {
        return *this;
    };
    /**
     * \brief Move operator prohibited
     */
    fge::CallbackHandler<Types...>& operator=(fge::CallbackHandler<Types...>&& n) = delete;

    /**
     * \brief Clear the list of callbacks
     */
    inline void clear();

    /**
     * \brief Add a new callback to the list
     *
     * A subscriber can be passed to this function to "categorize" the callback. The
     * subscriber will be used to remove the group of callbacks related to him later.
     *
     * You can pass a nullptr to the subscriber parameter if you don't want to use it, if so
     * the callback will be added to the default group.
     *
     * To facilitate your code, you can use the helper methods addFunctor(), addLambda() and addObjectFunctor().
     *
     * \param callback The new callback to add
     * \param subscriber The subscriber to use to categorize the callback
     * \return The callback pointer
     */
    inline fge::CallbackBase<Types...>* add(CalleePtr&& callback, fge::Subscriber* subscriber = nullptr);

    /**
     * \brief Helper method to add a callback functor
     *
     * \see add()
     *
     * \param func The callback function
     * \param subscriber The subscriber to use to categorize the callback
     * \return The callback pointer
     */
    inline fge::CallbackFunctor<Types...>* addFunctor(typename fge::CallbackFunctor<Types...>::CallbackFunction func,
                                                      fge::Subscriber* subscriber = nullptr);
    /**
     * \brief Helper method to add a callback lambda
     *
     * \see add()
     *
     * \tparam TLambda The lambda type
     * \param lambda The callback lambda
     * \param subscriber The subscriber to use to categorize the callback
     * \return The callback pointer
     */
    template<typename TLambda>
    inline fge::CallbackLambda<Types...>* addLambda(TLambda const& lambda, fge::Subscriber* subscriber = nullptr);
    /**
     * \brief Helper method to add a callback object functor
     *
     * \see add()
     *
     * \tparam TObject The object type
     * \param func The callback method of the object
     * \param object The object pointer
     * \param subscriber The subscriber to use to categorize the callback
     * \return The callback pointer
     */
    template<class TObject>
    inline fge::CallbackObjectFunctor<TObject, Types...>*
    addObjectFunctor(typename fge::CallbackObjectFunctor<TObject, Types...>::CallbackFunctionObject func,
                     TObject* object,
                     Subscriber* subscriber = nullptr);

    /**
     * \brief Remove a callback from the list
     *
     * You can remove a callback with the specified function/object pointer.
     *
     * \param ptr The function/object pointer to remove
     */
    void delPtr(void* ptr);
    /**
     * \brief Remove a callback from the list
     *
     * Remove a group of callbacks with the specified subscriber or nullptr for the default group.
     *
     * \param subscriber The subscriber associated to the group of callbacks
     */
    void delSub(fge::Subscriber* subscriber);
    /**
     * \brief Remove a callback from the list
     *
     * Remove a callback with the specified pointer.
     *
     * \param callback The callback to remove
     */
    void del(fge::CallbackBase<Types...>* callback);

    /**
     * \brief Call all the callbacks with the given arguments
     *
     * \param args The list of arguments
     */
    void call(Types... args);

    /**
     * \brief Hook a callback handler to this handler
     *
     * This will propagate the calls from this handler to the given handler of the same type.
     *
     * \warning The other handler must remain valid during the lifetime of this handler as it is not
     * cleared automatically.
     *
     * \param handler Another handler of the same type
     * \param subscriber The subscriber associated with this handler
     */
    void hook(fge::CallbackHandler<Types...>& handler, fge::Subscriber* subscriber = nullptr);

protected:
    /**
     * \brief This method is called when a subscriber is destroyed (destructor called)
     *
     * This avoid calling the callbacks when the subscriber is destroyed.
     *
     * \param subscriber The subscriber that is destroyed (or going to be destroyed)
     */
    void onDetach(fge::Subscriber* subscriber) override;

private:
    struct CalleeData
    {
        fge::CallbackHandler<Types...>::CalleePtr _f;
        fge::Subscriber* _subscriber = nullptr;
    };
    using CalleeList = std::forward_list<fge::CallbackHandler<Types...>::CalleeData>;

    fge::CallbackHandler<Types...>::CalleeList g_callees;

    mutable std::recursive_mutex g_mutex;
};

} // namespace fge

#include "FastEngine/C_callback.inl"

#endif // _FGE_C_CALLBACKHANDLER_HPP_INCLUDED
