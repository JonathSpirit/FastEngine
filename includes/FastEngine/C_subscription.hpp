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

#ifndef _FGE_C_SUBSCRIPTION_HPP_INCLUDED
#define _FGE_C_SUBSCRIPTION_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

namespace fge
{

class FGE_API Subscriber;

///Subscription

class FGE_API Subscription
{
public:
    using SubscriberCount = unsigned int;

    Subscription() = default;

    //Copy constructor prohibited (does nothing)
    Subscription(const fge::Subscription& r){};
    Subscription(fge::Subscription& r){};
    //Move constructor prohibited
    Subscription(const fge::Subscription&& r) = delete;
    Subscription(fge::Subscription&& r) = delete;

    virtual inline ~Subscription(){this->detachAll();}

    //Copy operator prohibited (does nothing)
    fge::Subscription& operator =(const fge::Subscription& r){return *this;};
    fge::Subscription& operator =(fge::Subscription& r){return *this;};
    //Move operator prohibited
    fge::Subscription& operator =(const fge::Subscription&& r) = delete;
    fge::Subscription& operator =(fge::Subscription&& r) = delete;

protected:
    virtual void onDetach(fge::Subscriber* subscriber) = 0;

    void detachAll();

    bool detach(fge::Subscriber* subscriber);
    fge::Subscription::SubscriberCount detachOnce(fge::Subscriber* subscriber);

    fge::Subscription::SubscriberCount attach(fge::Subscriber* subscriber);

    fge::Subscription::SubscriberCount getCount(fge::Subscriber* subscriber) const;

private:
    using SubscriptionDataType = std::unordered_map<fge::Subscriber*, fge::Subscription::SubscriberCount>;

    void detachSilent(fge::Subscriber* subscriber);

    fge::Subscription::SubscriptionDataType g_subData;
    friend fge::Subscriber;
};

///Subscriber

class FGE_API Subscriber
{
public:
    Subscriber() = default;

    //Copy constructor prohibited (does nothing)
    Subscriber(const fge::Subscriber& n){};
    Subscriber(fge::Subscriber& n){};
    //Move constructor prohibited
    Subscriber(const fge::Subscriber&& n) = delete;
    Subscriber(fge::Subscriber&& n) = delete;

    virtual inline ~Subscriber(){this->detachAll();};

    //Copy operator prohibited (does nothing)
    fge::Subscriber& operator =(const fge::Subscriber& n){return *this;};
    fge::Subscriber& operator =(fge::Subscriber& n){return *this;};
    //Move operator prohibited
    fge::Subscriber& operator =(const fge::Subscriber&& n) = delete;
    fge::Subscriber& operator =(fge::Subscriber&& n) = delete;

protected:
    virtual void onDetach(fge::Subscription* subscription){}

    void detachAll();
    void detach(fge::Subscription* subscription);

private:
    using SubscriberDataType = std::unordered_set<fge::Subscription*>;

    void detachSilent(fge::Subscription* subscription);
    void attachSilent(fge::Subscription* subscription);

    fge::Subscriber::SubscriberDataType g_subData;
    friend fge::Subscription;
};

}//end fge

#endif // _FGE_C_SUBSCRIPTION_HPP_INCLUDED
