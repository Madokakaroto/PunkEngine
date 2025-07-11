#include "ECS/Entity/EntityPool.h"

namespace punk
{
    entity_t entity_pool_impl_t::allocate_entity()
    {
        auto [version_ptr, index] = entities_version_.construct();
        assert(version_ptr);
        auto const version = version_ptr->version++;
        return entity_t::compose(entity_handle_t{ static_cast<uint32_t>(index) }, version);
    }

    void entity_pool_impl_t::deallocate_entity(entity_t entity)
    {
        auto const handle = entity.get_handle();
        auto const version = entity.get_version();
        auto const version_ptr = entities_version_.get(handle.get_value());
        if(version_ptr && version_ptr->version == version)
        {
            entities_version_.destruct(handle.get_value());
        }
    }

    bool entity_pool_impl_t::is_alive(entity_t entity)
    {
        auto const handle = entity.get_handle();
        auto const version = entity.get_version();
        auto const version_ptr = entities_version_.get(handle.get_value());
        return version_ptr && version_ptr->version == version;
    }

    entity_t entity_pool_impl_t::restore_entity(entity_handle_t handle)
    {
        auto [version_ptr, _] = entities_version_.construct_at(handle.get_value(), false);
        return entity_t::compose(handle, version_ptr->version);
    }

    entity_pool_t* entity_pool_t::create_entity_pool()
    {
        return new entity_pool_impl_t{};
    }
}