#include "ECS/DataStorage/DataStorage.h"

namespace punk
{
    data_storage_t* data_storage_t::create_instance(archetype_registry_t* archetype_registry, entity_pool_t* entity_pool)
    {
        return nullptr;
    }
}

namespace punk
{
    data_storage_impl::data_storage_impl(archetype_registry_t* archetype_registry, entity_pool_t* entity_pool)
        : data_storage_t()
        , archetype_registry_(archetype_registry)
        , entity_pool_(entity_pool)
    {
        assert(archetype_registry_);
        assert(entity_pool_);
    }

    archetype_instance_handle_t data_storage_impl::get_archetype_instance(entity_t entity)
    {
        assert(entity_pool_);

        if (!entity.is_valid())
        {
            return archetype_instance_handle_t::invalid_handle();
        }
        if(!entity_pool_->is_alive(entity))
        {
            return archetype_instance_handle_t::invalid_handle();
        }

        auto const entity_handle = entity.get_handle();
        auto const* archetype_instance_handle_ptr = entity_archetype_instance_indices_.get(entity_handle.get_value());
        if (!archetype_instance_handle_ptr)
        {
            return archetype_instance_handle_t::invalid_handle();
        }

        return *archetype_instance_handle_ptr;
    }

    archetype_instance_handle_t data_storage_impl::attach_archetype(archetype_ptr const& archetype)
    {
        auto const instance_handle = archetype_instance_registry_.attach_archetype(archetype);
        return instance_handle;
    }
}