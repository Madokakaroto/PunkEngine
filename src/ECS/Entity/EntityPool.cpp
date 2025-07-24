#include "ECS/Entity/EntityPool.h"

namespace punk
{
    entity_t entity_pool_impl_t::allocate_entity()
    {
        uint32_t index = 0, version = 0;
        {
            scoped_spin_lock_t lock{ spin_lock_ };
            auto [version_ptr, idx] = entities_version_.construct();
            assert(version_ptr);
            version = version_ptr->version++;
            index = static_cast<uint32_t>(idx);
        }
        return entity_t::compose(entity_handle_t{ index }, version);
    }

    void entity_pool_impl_t::deallocate_entity(entity_t entity)
    {
        auto const handle = entity.get_handle();
        auto const version = entity.get_version();
        scoped_spin_lock_t lock{ spin_lock_ };
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
        scoped_spin_lock_t lock{ spin_lock_ };
        auto const version_ptr = entities_version_.get(handle.get_value());
        return version_ptr && version_ptr->version == version;
    }

    entity_t entity_pool_impl_t::restore_entity(entity_handle_t handle)
    {
        scoped_spin_lock_t lock{ spin_lock_ };
        auto [version_ptr, _] = entities_version_.construct_at(handle.get_value(), false);
        return entity_t::compose(handle, version_ptr->version);
    }

    entity_pool_t* entity_pool_t::create_entity_pool()
    {
        return new entity_pool_impl_t{};
    }
}