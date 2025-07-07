#pragma once

#include "ECS/Detail/Entity.h"

namespace punk
{
    class entity_pool_t
    {
    public:
        entity_pool_t() = default;
        virtual ~entity_pool_t() = default;
        entity_pool_t(entity_pool_t const&) = delete;
        entity_pool_t& operator=(entity_pool_t const&) = delete;
        entity_pool_t(entity_pool_t&&) = delete;
        entity_pool_t& operator=(entity_pool_t&&) = delete;

        static entity_pool_t* create_entity_pool();

    public:
        virtual entity_t allocate_entity(uint16_t tag) = 0;
        virtual void deallocate_entity(entity_t entity) = 0;
        virtual bool is_alive(entity_t entity) = 0;
        virtual entity_t restore_entity(entity_handle_t handle) = 0;
    };
}