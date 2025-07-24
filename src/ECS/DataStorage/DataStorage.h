#pragma once

#include "ECS/CoreTypes.h"
#include "ECS/Archetype/ArchetypeInstanceRegistry.h"
#include "Base/Containers/Hive.h"

namespace punk
{
    class data_storage_impl final : public data_storage_t
    {
    private:
        archetype_registry_t*               archetype_registry_;                    // manages archetypes
        entity_pool_t*                      entity_pool_;                           // allocate entity handles and versions
        archetype_instance_registry         archetype_instance_registry_;           // manages archetype instances
        hive<archetype_instance_handle_t>   entity_archetype_instance_indices_;     // maps entity handle to archetype instance handle

    public:
        data_storage_impl(archetype_registry_t* archetype_registry, entity_pool_t* entity_pool);

    public:

    protected:
        virtual archetype_instance_handle_t get_archetype_instance(entity_t entity) override;
        virtual archetype_instance_handle_t attach_archetype(archetype_ptr const& archetype) override;
    };
}