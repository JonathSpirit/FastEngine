#ifndef _FGE_C_CALLBACKHANDLER_HPP_INCLUDED
#define _FGE_C_CALLBACKHANDLER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_subscription.hpp>
#include <forward_list>
#include <mutex>
#include <memory>

namespace fge
{

///CallbackFunctorBase

template <class ... Types>
class CallbackFunctorBase
{
public:
    virtual ~CallbackFunctorBase() = default;

    virtual void call(Types ... args) = 0;
    virtual bool check(void* ptr) = 0;
};

///CallbackFunctor

template <class ... Types>
class CallbackFunctor : public fge::CallbackFunctorBase<Types ...>
{
public:
    using CallbackFunction = void (*) (Types ... args);

    CallbackFunctor(fge::CallbackFunctor<Types ...>::CallbackFunction func);
    ~CallbackFunctor() override = default;

    void call(Types ... args) override;
    inline bool check(void* ptr) override;

protected:
    fge::CallbackFunctor<Types ...>::CallbackFunction g_function;
};

///CallbackFunctorObject

template <class TObject, class ... Types>
class CallbackFunctorObject : public fge::CallbackFunctorBase<Types ...>
{
public:
    using CallbackFunctionObject = void (TObject::*) (Types ... args);

    CallbackFunctorObject(fge::CallbackFunctorObject<TObject, Types ...>::CallbackFunctionObject func, TObject* object);
    ~CallbackFunctorObject() override = default;

    void call(Types ... args) override;
    inline bool check(void* ptr) override;

protected:
    fge::CallbackFunctorObject<TObject, Types ...>::CallbackFunctionObject g_functionObj;
    TObject* g_object;
};

///CallbackHandler

template <class ... Types>
class CallbackHandler : public fge::Subscription
{
public:
    CallbackHandler() = default;
    ~CallbackHandler() override = default;

    //Copy constructor that does nothing
    CallbackHandler(const fge::CallbackHandler<Types ...>& n){};
    //Move constructor prohibed
    CallbackHandler(fge::CallbackHandler<Types ...>&& n) = delete;

    //Copy operator that does nothing
    fge::CallbackHandler<Types ...>& operator =(const fge::CallbackHandler<Types ...>& n){return *this;};
    //Move operator prohibed
    fge::CallbackHandler<Types ...>& operator =(fge::CallbackHandler<Types ...>&& n) = delete;

    inline void clear();

    inline void add(fge::CallbackFunctorBase<Types ...>* callback, fge::Subscriber* subscriber = nullptr);
    void delPtr(void* ptr);
    void del(fge::Subscriber* subscriber);

    void call(Types ... args);

protected:
    void onDetach(fge::Subscriber* subscriber) override;

private:
    using CalleePtr = std::unique_ptr<fge::CallbackFunctorBase<Types ...> >;
    struct CalleeData
    {
        fge::CallbackHandler<Types ...>::CalleePtr _f;
        fge::Subscriber* _subscriber = nullptr;
    };
    using CalleeList = std::forward_list<fge::CallbackHandler<Types ...>::CalleeData>;

    fge::CallbackHandler<Types ...>::CalleeList g_callees;

    mutable std::recursive_mutex g_mutex;
};

}//end fge

#include <FastEngine/C_callback.inl>

#endif // _FGE_C_CALLBACKHANDLER_HPP_INCLUDED
