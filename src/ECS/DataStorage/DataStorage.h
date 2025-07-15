#pragma once

#include "ECS/CoreTypes.h"
#include "ECS/Archetype/ArchetypeInstanceRegistry.h"
#include "Base/Containers/Hive.h"

namespace punk
{
    class data_storage_impl final : public data_storage_t
    {
    private:
        archetype_registry_t*           archetype_registry_;
        hive<uint32_t>                  entity_archetype_instance_indices_;
        archetype_instance_registry     archetype_instances_;

    public:
        explicit data_storage_impl(archetype_registry_t* archetype_registry);
    };
}