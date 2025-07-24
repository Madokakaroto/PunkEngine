#pragma once

#include "ECS/CoreTypes.h"
#include "Base/Containers/Hive.h"
#include "Base/Async/Async.h"

namespace punk
{
    struct entity_version_t
    {
        uint32_t padded;
        uint32_t version;
    };

    class entity_pool_impl_t : public entity_pool_t
    {
        using spin_lock_t = async_simple::coro::SpinLock;
        using scoped_spin_lock_t = async_simple::coro::ScopedSpinLock;

    public:
        virtual entity_t allocate_entity() override;
        virtual void deallocate_entity(entity_t entity) override;
        virtual bool is_alive(entity_t entity) override;
        virtual entity_t restore_entity(entity_handle_t handle) override;

    private:
        spin_lock_t             spin_lock_;
        hive<entity_version_t>  entities_version_;
    };
}