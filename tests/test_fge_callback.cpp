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

#include "doctest/doctest.h"
#include "FastEngine/C_callback.hpp"

TEST_CASE("testing empty callback")
{
    fge::CallbackHandler<> onEvent;

    SUBCASE("adding lambda callback")
    {
        unsigned int number = 0;
        auto func = [&](){
            ++number;
        };

        onEvent.addLambda(func);
        onEvent.call();

        REQUIRE(number == 1);
        onEvent.call();
        REQUIRE(number == 2);

        SUBCASE("removing lambda callback using ptr")
        {
            onEvent.delPtr(&func); //this should not work cause CallbackLambda internally create a new copy of the passed lambda
            onEvent.call();
            REQUIRE(number == 3);
        }

        SUBCASE("removing lambda callback using default subscriber")
        {
            onEvent.delSub(nullptr);
            onEvent.call();
            REQUIRE(number == 2);
        }

        SUBCASE("adding same lambda with a subscriber and removing")
        {
            fge::Subscriber subscriber;
            onEvent.addLambda(func, &subscriber);
            onEvent.call();
            REQUIRE(number == 4);
            onEvent.delSub(&subscriber);
            onEvent.call();
            REQUIRE(number == 5);
            onEvent.clear();
            onEvent.call();
            REQUIRE(number == 5);
        }
    }
}

void func(int* a, int b)
{
    *a += b;
}

TEST_CASE("testing callback with arguments")
{
    fge::CallbackHandler<int*, int> onEvent;

    SUBCASE("adding function callback")
    {
        int number = 0;
        onEvent.addFunctor(func);
        onEvent.call(&number, 1);

        REQUIRE(number == 1);
        onEvent.call(&number, 2);
        REQUIRE(number == 3);

        SUBCASE("removing function callback using ptr")
        {
            onEvent.delPtr(reinterpret_cast<void*>(&func));
            onEvent.call(&number, 20);
            REQUIRE(number == 3);
        }

        SUBCASE("removing function callback using default subscriber")
        {
            onEvent.delSub(nullptr);
            onEvent.call(&number, 1);
            REQUIRE(number == 3);
        }

        SUBCASE("adding same function with a subscriber and removing")
        {
            fge::Subscriber subscriber;
            onEvent.addFunctor(func, &subscriber);
            onEvent.call(&number, 10);
            REQUIRE(number == 23);
            onEvent.delSub(&subscriber);
            onEvent.call(&number, -3);
            REQUIRE(number == 20);
            onEvent.clear();
            onEvent.call(&number, 1000);
            REQUIRE(number == 20);
        }
    }
}

TEST_CASE("testing callback suppression")
{
    fge::CallbackHandler<> onEvent;

    unsigned int number = 0;
    auto funcCount = [&](){
        ++number;
    };
    auto funcCountLess = [&](){
        --number;
    };
    auto funcDestroy = [&](){
        onEvent.delSub(nullptr);
    };

    SUBCASE("suppressing callbacks after")
    {
        fge::Subscriber subscriber;
        onEvent.addLambda(funcCount, &subscriber);
        onEvent.addLambda(funcCount, &subscriber);
        onEvent.addLambda(funcCount, &subscriber);
        onEvent.addLambda(funcCountLess, &subscriber);
        onEvent.addLambda(funcDestroy, &subscriber);

        onEvent.call();
        REQUIRE(number == 2);

        number = 0;

        onEvent.addLambda(funcCount, nullptr);
        onEvent.addLambda(funcCount, nullptr);
        onEvent.addLambda(funcCount, nullptr);
        onEvent.addLambda(funcCount, &subscriber);

        onEvent.call();
        REQUIRE(number == 3);
    }

    SUBCASE("suppressing callbacks before")
    {
        fge::Subscriber subscriber;
        onEvent.addLambda(funcCount, nullptr);
        onEvent.addLambda(funcCount, nullptr);
        onEvent.addLambda(funcCount, nullptr);
        onEvent.addLambda(funcCountLess, &subscriber);
        onEvent.addLambda(funcDestroy, &subscriber);

        onEvent.call();
        REQUIRE(number == 2);

        number = 1;
        onEvent.call(); //All callbacks without subscriber should be marked as deleted
        REQUIRE(number == 0);

        number = 5;

        onEvent.addLambda(funcCount, &subscriber);
        onEvent.addLambda(funcCount, &subscriber);
        onEvent.addLambda(funcCount, &subscriber);
        onEvent.addLambda(funcCount, &subscriber);

        onEvent.call();
        REQUIRE(number == 8);
    }

    SUBCASE("suppressing itself")
    {
        fge::Subscriber subscriber;
        onEvent.addLambda(funcCount, &subscriber);
        onEvent.addLambda(funcCount, &subscriber);
        onEvent.addLambda(funcCount, &subscriber);
        onEvent.addLambda(funcCountLess, &subscriber);
        onEvent.addLambda(funcDestroy, nullptr);

        onEvent.call();
        REQUIRE(number == 2);

        number = 0;

        onEvent.addLambda(funcCount, nullptr);
        onEvent.addLambda(funcCount, nullptr);
        onEvent.addLambda(funcCount, nullptr);
        onEvent.addLambda(funcCount, &subscriber);

        onEvent.call();
        REQUIRE(number == 6);
    }
}

TEST_CASE("testing adding callbacks in a call")
{
    fge::CallbackHandler<> onEvent;

    unsigned int number = 0;
    auto funcCount = [&](){
        ++number;
    };
    auto funcCountLess = [&](){
        --number;
    };
    auto funcAdd = [&](){
        onEvent.addLambda(funcCountLess);
        onEvent.addLambda(funcCountLess);
    };

    onEvent.addLambda(funcCount);
    onEvent.addLambda(funcCount);
    onEvent.addLambda(funcCount);
    onEvent.addLambda(funcAdd);
    onEvent.addLambda(funcCount);
    onEvent.addLambda(funcCount);
    onEvent.addLambda(funcCount);
    onEvent.addLambda(funcAdd);

    onEvent.call();
    REQUIRE(number == 2);
}

///TODO: add tests for CallbackObjectFunctor, and object that inherit from Subscriber