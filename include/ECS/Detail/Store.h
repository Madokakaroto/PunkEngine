#pragma once

#include "ECS/Detail/Entity.h"
#include "ECS/Detail/ErrorCode.h"

namespace punk
{
    class store
    {
    public:
        store() = default;
        virtual ~store() = default;
        store(store const&) = delete;
        store& operator=(store const&) = delete;
        store(store&&) = delete;
        store& operator=(store&&) = delete;

    public:

    };
}