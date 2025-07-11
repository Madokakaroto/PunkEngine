#pragma once

#include "ECS/CoreTypes.h"
#include "Base/Containers/Hive.h"

namespace punk
{
    struct entity_version_t
    {
        uint32_t padded;
        uint32_t version;
    };

    class entity_pool_impl_t : public entity_pool_t
    {
    public:
        virtual entity_t allocate_entity() override;
        virtual void deallocate_entity(entity_t entity) override;
        virtual bool is_alive(entity_t entity) override;
        virtual entity_t restore_entity(entity_handle_t handle) override;
        
    private:
        hive<entity_version_t> entities_version_;
    };
}