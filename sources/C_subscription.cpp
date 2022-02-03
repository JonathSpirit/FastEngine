#include "FastEngine/C_subscription.hpp"

namespace fge
{

///Subscription
void FGE_API Subscription::detachAll()
{
    for (fge::Subscription::SubscriptionDataType::iterator it = this->g_subData.begin(); it != this->g_subData.end(); ++it)
    {
        it->first->detachSilent(this);
    }
    this->g_subData.clear();
}

void FGE_API Subscription::detachSilent(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr)
    {
        return;
    }

    fge::Subscription::SubscriptionDataType::iterator it = this->g_subData.find(subscriber);
    if ( it != this->g_subData.end() )
    {
        this->g_subData.erase(it);
        this->onDetach(subscriber);
    }
}

bool FGE_API Subscription::detach(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr)
    {
        return true;
    }

    fge::Subscription::SubscriptionDataType::iterator it = this->g_subData.find(subscriber);
    if ( it != this->g_subData.end() )
    {
        subscriber->detachSilent(this);
        this->g_subData.erase(it);
        return true;
    }
    return false;
}
fge::Subscription::SubscriberCount FGE_API Subscription::detachOnce(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr)
    {
        return 1;
    }

    fge::Subscription::SubscriptionDataType::iterator it = this->g_subData.find(subscriber);
    if ( it != this->g_subData.end() )
    {
        if ( --it->second == 0)
        {
            subscriber->detachSilent(this);
            this->g_subData.erase(it);
            return 0;
        }

        return it->second;
    }
    return 0;
}

fge::Subscription::SubscriberCount FGE_API Subscription::attach(fge::Subscriber* subscriber)
{
    if (subscriber == nullptr)
    {
        return 1;
    }

    fge::Subscription::SubscriptionDataType::iterator it = this->g_subData.find(subscriber);
    if ( it != this->g_subData.end() )
    {
        return ++it->second;
    }
    else
    {
        this->g_subData[subscriber] = 1;
        subscriber->attachSilent(this);
        return 1;
    }
}

fge::Subscription::SubscriberCount FGE_API Subscription::getCount(fge::Subscriber* subscriber) const
{
    if (subscriber == nullptr)
    {
        return 0;
    }

    fge::Subscription::SubscriptionDataType::const_iterator it = this->g_subData.find(subscriber);
    if ( it != this->g_subData.cend() )
    {
        return it->second;
    }
    return 0;
}

///Subscriber

void FGE_API Subscriber::detachAll()
{
    for (fge::Subscriber::SubscriberDataType::iterator it = this->g_subData.begin(); it != this->g_subData.end(); ++it)
    {
        (*it)->detachSilent(this);
    }
    this->g_subData.clear();
}
void FGE_API Subscriber::detachSilent(fge::Subscription* subscription)
{
    if ( this->g_subData.erase(subscription) )
    {
        this->onDetach(subscription);
    }
}
void FGE_API Subscriber::detach(fge::Subscription* subscription)
{
    if ( this->g_subData.erase(subscription) )
    {
        subscription->detachSilent(this);
    }
}
void FGE_API Subscriber::attachSilent(fge::Subscription* subscription)
{
    this->g_subData.insert(subscription);
}

}//end fge
