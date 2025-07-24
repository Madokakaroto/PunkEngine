#pragma once

#include "ECS/Archetype/ArchetypeInstance.h"
#include "Base/Containers/Hive.h"

namespace punk
{
    class archetype_instance_registry final
    {
        using hash2instance_container = unordered_map<uint32_t, archetype_instance_handle_t>;

    private:
        hive<archetype_instance>    archetype_instances_;
        hash2instance_container     archetype_hash_to_instance_;

    public:
        archetype_instance_registry() = default;
        ~archetype_instance_registry() = default;
        archetype_instance_registry(archetype_instance_registry const&) = delete;
        archetype_instance_registry& operator=(archetype_instance_registry const&) = delete;
        archetype_instance_registry(archetype_instance_registry&&) = default;
        archetype_instance_registry& operator=(archetype_instance_registry&&) = default;

        archetype_instance_handle_t attach_archetype(archetype_ptr const& archetype);
        void detach_archetype(archetype_ptr const& archetype);
        void detach_archetype_by_hash(uint32_t hash);
        void detach_archetype_by_index(uint32_t index);
        archetype_instance const* get_archetype_instance(archetype_ptr const& archetype) const;
        archetype_instance* get_archetype_instance(archetype_ptr const& archetype);
        archetype_instance const* get_archetype_instance(archetype_instance_handle_t index) const;
        archetype_instance* get_archetype_instance(archetype_instance_handle_t index);
        archetype_instance const* get_archetype_instance(uint32_t hash) const;
        archetype_instance* get_archetype_instance(uint32_t hash);

    };
}