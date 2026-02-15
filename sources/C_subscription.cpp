/*
 * Copyright 2025 Guillaume Guillet
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

#include "FastEngine/C_subscription.hpp"

namespace fge
{

//Subscription

Subscription::Subscription(fge::Subscription&& r) noexcept :
        g_subData(std::move(r.g_subData))
{
    for (auto& data: this->g_subData)
    {
        data.first->detachSilent(&r);
        data.first->attachSilent(this);
    }
}

fge::Subscription& Subscription::operator=(fge::Subscription&& r) noexcept
{
    this->detachAll();
    this->g_subData = std::move(r.g_subData);
    for (auto& data: this->g_subData)
    {
        data.first->detachSilent(&r);
        data.first->attachSilent(this);
    }
    return *this;
}

void Subscription::detachAll()
{
    for (auto& data: this->g_subData)
    {
        data.first->detachSilent(this);
    }
    this->g_subData.clear();
}

void Subscription::detachSilent(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr)
    {
        return;
    }

    auto it = this->g_subData.find(subscriber);
    if (it != this->g_subData.end())
    {
        this->g_subData.erase(it);
        this->onDetach(subscriber);
    }
}

bool Subscription::detach(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr)
    {
        return true;
    }

    auto it = this->g_subData.find(subscriber);
    if (it != this->g_subData.end())
    {
        subscriber->detachSilent(this);
        this->g_subData.erase(it);
        return true;
    }
    return false;
}
fge::Subscription::SubscriberCount Subscription::detachOnce(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr)
    {
        return 1;
    }

    auto it = this->g_subData.find(subscriber);
    if (it != this->g_subData.end())
    {
        if (--it->second == 0)
        {
            subscriber->detachSilent(this);
            this->g_subData.erase(it);
            return 0;
        }
        return it->second;
    }
    return 0;
}

fge::Subscription::SubscriberCount Subscription::attach(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr)
    {
        return 1;
    }

    auto it = this->g_subData.find(subscriber);
    if (it != this->g_subData.end())
    {
        return ++it->second;
    }

    this->g_subData[subscriber] = 1;
    subscriber->attachSilent(this);
    return 1;
}

fge::Subscription::SubscriberCount Subscription::getCount(fge::Subscriber* subscriber) const
{
    if (subscriber == nullptr)
    {
        return 0;
    }

    auto it = this->g_subData.find(subscriber);
    if (it != this->g_subData.cend())
    {
        return it->second;
    }
    return 0;
}

//UniqueSubscription

UniqueSubscription::UniqueSubscription(fge::UniqueSubscription&& r) noexcept :
        g_subscriber(r.g_subscriber)
{
    if (this->g_subscriber != nullptr)
    {
        this->g_subscriber->detachSilent(&r);
        this->g_subscriber->attachSilent(this);
        r.g_subscriber = nullptr;
    }
}

fge::UniqueSubscription& UniqueSubscription::operator=(fge::UniqueSubscription&& r) noexcept
{
    this->detachAll();
    this->g_subscriber = r.g_subscriber;
    if (this->g_subscriber != nullptr)
    {
        this->g_subscriber->detachSilent(&r);
        this->g_subscriber->attachSilent(this);
    }
    return *this;
}

void UniqueSubscription::detachAll()
{
    if (this->g_subscriber != nullptr)
    {
        this->g_subscriber->detachSilent(this);
        this->g_subscriber = nullptr;
    }
}

void UniqueSubscription::detachSilent(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr || this->g_subscriber != subscriber)
    {
        return;
    }

    this->onDetach(subscriber);
    this->g_subscriber = nullptr;
}

bool UniqueSubscription::detach(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr)
    {
        return true;
    }

    if (this->g_subscriber != subscriber)
    {
        return false;
    }

    subscriber->detachSilent(this);
    this->g_subscriber = nullptr;
    return true;
}
fge::UniqueSubscription::SubscriberCount UniqueSubscription::detachOnce(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr)
    {
        return 1;
    }

    if (this->g_subscriber != subscriber)
    {
        return 0;
    }

    subscriber->detachSilent(this);
    this->g_subscriber = nullptr;
    return 0;
}

fge::UniqueSubscription::SubscriberCount UniqueSubscription::attach(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr)
    {
        return 1;
    }

    if (subscriber == this->g_subscriber)
    {
        return 1;
    }

    if (this->g_subscriber != nullptr)
    {
        return 0;
    }

    this->g_subscriber = subscriber;
    subscriber->attachSilent(this);
    return 1;
}

fge::UniqueSubscription::SubscriberCount UniqueSubscription::getCount(fge::Subscriber* subscriber) const
{
    if (subscriber == nullptr || this->g_subscriber == nullptr)
    {
        return 0;
    }

    if (subscriber != this->g_subscriber)
    {
        return 0;
    }

    return 1;
}

//Subscriber
void Subscriber::detachAll()
{
    for (auto* data: this->g_subData)
    {
        data->detachSilent(this);
    }
    this->g_subData.clear();
}
void Subscriber::detachSilent(fge::BaseSubscription* subscription)
{
    if (this->g_subData.erase(subscription) > 0)
    {
        this->onDetach(subscription);
    }
}
void Subscriber::detach(fge::BaseSubscription* subscription)
{
    if (this->g_subData.erase(subscription) > 0)
    {
        subscription->detachSilent(this);
    }
}
void Subscriber::attachSilent(fge::BaseSubscription* subscription)
{
    this->g_subData.insert(subscription);
}

} // namespace fge
