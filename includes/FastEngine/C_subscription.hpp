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

class Subscriber;

/**
 * \class Subscription
 * \ingroup utility
 * \brief This class is a useful utility to "link" multiple objects around
 * a specific use with automatic lifetime management
 *
 * A good example is the callback system. When inheriting from Subscriber,
 * you will be able to subscribe a function to a CallbackHandler (inheriting from Subscription)
 * and you don't have to worry about unsubscribing it when the object is destroyed.
 *
 * A \b nullptr subscriber mean that you are subscribing something that is not class related.
 * (sort of a global scope).
 *
 * Alone this class will not do much as you have to implement the required functionality around it.
 *
 * \see CallbackHandler
 */
class FGE_API Subscription
{
public:
    using SubscriberCount = unsigned int;

    Subscription() = default;

    ///\warning Empty copy constructor as it's not permitted (does nothing)
    Subscription([[maybe_unused]] const fge::Subscription& r){};
    Subscription(fge::Subscription&& r) noexcept;

    /**
     * \brief When the object is destroyed, it will detach all subscribers
     */
    virtual inline ~Subscription(){this->detachAll();}

    ///\warning Empty copy operator as it's not permitted (does nothing)
    fge::Subscription& operator =([[maybe_unused]] const fge::Subscription& r){return *this;};
    fge::Subscription& operator =(fge::Subscription&& r) noexcept;

protected:
    /**
     * \brief Callback called when a subscriber is detached
     *
     * The subscriber, at this point, can't be \b nullptr.
     *
     * \param subscriber The subscriber that was detached
     */
    virtual void onDetach(fge::Subscriber* subscriber) = 0;

    /**
     * \brief Detach all subscribers
     */
    void detachAll();

    /**
     * \brief Detach a specific subscriber
     *
     * Detaching a \b nullptr global scope subscriber will do nothing.
     *
     * \param subscriber The subscriber to detach
     * \return \b true if the subscriber was detached, \b false otherwise
     */
    bool detach(fge::Subscriber* subscriber);
    /**
     * \brief Detach only once a specific subscriber
     *
     * You can attach a subscriber multiple times and it will augment the SubscriberCount.
     * This function will only detach one time the subscriber and decrement the SubscriberCount.
     * If the SubscriberCount is 0, the subscriber will be detached.
     *
     * \param subscriber The subscriber to detach
     * \return The remaining SubscriberCount
     */
    fge::Subscription::SubscriberCount detachOnce(fge::Subscriber* subscriber);

    /**
     * \brief Attach a specific subscriber
     *
     * You can't directly attach a \b nullptr global scope subscriber.
     * But if you do, this function will do nothing and return 1 as the SubscriberCount.
     *
     * \param subscriber The subscriber to attach
     * \return The SubscriberCount
     */
    fge::Subscription::SubscriberCount attach(fge::Subscriber* subscriber);

    /**
     * \brief Get the SubscriberCount of a specific subscriber
     *
     * \param subscriber The subscriber to get the SubscriberCount
     * \return The SubscriberCount
     */
    fge::Subscription::SubscriberCount getCount(fge::Subscriber* subscriber) const;

private:
    using SubscriptionDataType = std::unordered_map<fge::Subscriber*, fge::Subscription::SubscriberCount>;

    /**
     * \brief Silent detach a specific subscriber
     *
     * This function is generally called directly by a Subscriber in order to
     * avoid a infinite recursive detach.
     *
     * \param subscriber The subscriber to detach
     */
    void detachSilent(fge::Subscriber* subscriber);

    fge::Subscription::SubscriptionDataType g_subData;
    friend class fge::Subscriber;
};

/**
 * \class Subscriber
 * \ingroup utility
 * \brief This class is a useful utility to "link" multiple objects around
 *
 * By inheriting from this class, you will be able to subscribe to a Subscription like
 * a CallbackHandler and you don't have to worry about unsubscribing it when the object is destroyed.
 *
 * \see Subscription
 */
class FGE_API Subscriber
{
public:
    Subscriber() = default;

    ///\warning Empty copy constructor as it's not permitted (does nothing)
    Subscriber([[maybe_unused]] const fge::Subscriber& n){};
    ///\warning Move constructor prohibited
    Subscriber(fge::Subscriber&& n) noexcept = delete;

    /**
     * \brief When the object is destroyed, it will detach from all subscriptions
     */
    virtual inline ~Subscriber(){this->detachAll();};

    ///\warning Empty copy operator as it's not permitted (does nothing)
    fge::Subscriber& operator =([[maybe_unused]] const fge::Subscriber& n){return *this;};
    ///\warning Move operator prohibited
    fge::Subscriber& operator =(fge::Subscriber&& n) noexcept = delete;

protected:
    /**
     * \brief Callback called when a subscription is detached
     *
     * \param subscription The subscription that was detached
     */
    virtual void onDetach([[maybe_unused]] fge::Subscription* subscription){}

    /**
     * \brief Detach from all subscriptions
     */
    void detachAll();
    /**
     * \brief Detach from a specific subscription
     *
     * \param subscription
     */
    void detach(fge::Subscription* subscription);

private:
    using SubscriberDataType = std::unordered_set<fge::Subscription*>;

    /**
     * \brief Silent detach from a specific subscription
     *
     * This function is generally called directly by a Subscription in order to
     * avoid a infinite recursive detach.
     *
     * \param subscription The subscription to detach
     */
    void detachSilent(fge::Subscription* subscription);
    /**
     * \brief Silent attach to a specific subscription
     *
     * \param subscription The subscription to attach
     */
    void attachSilent(fge::Subscription* subscription);

    fge::Subscriber::SubscriberDataType g_subData;
    friend class fge::Subscription;
};

}//end fge

#endif // _FGE_C_SUBSCRIPTION_HPP_INCLUDED
