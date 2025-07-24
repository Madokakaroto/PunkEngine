#include "ECS/CoreTypes.h"
#include "ECS/Archetype/ArchetypeInstanceRegistry.h"

namespace punk
{
    archetype_instance_handle_t archetype_instance_registry::attach_archetype(archetype_ptr const& archetype)
    {
        if (!archetype)
        {
            return archetype_instance_handle_t::invalid_handle();
        }
        auto itr = archetype_hash_to_instance_.find(archetype->hash);
        if (itr != archetype_hash_to_instance_.end())
        {
            return itr->second;
        }

        auto [archetype_instance, index] = archetype_instances_.construct(archetype);
        archetype_instance->set_index(static_cast<uint32_t>(index));
        archetype_hash_to_instance_.emplace(archetype->hash, archetype_instance->get_index());
        return archetype_instance->get_handle();
    }

    void archetype_instance_registry::detach_archetype(archetype_ptr const& archetype)
    {
        if (!archetype)
        {
            return;
        }

        detach_archetype_by_hash(archetype->hash);
    }

    void archetype_instance_registry::detach_archetype_by_hash(uint32_t hash)
    {
        auto itr = archetype_hash_to_instance_.find(hash);
        if (itr != archetype_hash_to_instance_.end())
        {
            archetype_instances_.destruct(itr->second.get_value());
            archetype_hash_to_instance_.erase(itr);
        }
    }

    void archetype_instance_registry::detach_archetype_by_index(uint32_t index)
    {
        if (index == archetype_instance::non_archetype_index())
        {
            return;
        }

        auto const* archetype_instance = archetype_instances_.get(index);
        if (!archetype_instance)
        {
            return;
        }

        archetype_instances_.destruct(index);
        archetype_hash_to_instance_.erase(archetype_instance->get_hash());
    }

    archetype_instance const* archetype_instance_registry::get_archetype_instance(archetype_ptr const& archetype) const
    {
        return get_archetype_instance(archetype->hash);
    }

    archetype_instance* archetype_instance_registry::get_archetype_instance(archetype_ptr const& archetype)
    {
        return const_cast<archetype_instance*>(const_cast<archetype_instance_registry const*>(this)->get_archetype_instance(archetype));
    }

    archetype_instance const* archetype_instance_registry::get_archetype_instance(archetype_instance_handle_t index) const
    {
        if (!index.is_valid())
        {
            return nullptr;
        }
        return archetype_instances_.get(index.get_value());
    }

    archetype_instance* archetype_instance_registry::get_archetype_instance(archetype_instance_handle_t index)
    {
        return const_cast<archetype_instance*>(const_cast<archetype_instance_registry const*>(this)->get_archetype_instance(index));
    }

    archetype_instance const* archetype_instance_registry::get_archetype_instance(uint32_t hash) const
    {
        auto itr = archetype_hash_to_instance_.find(hash);
        if (itr != archetype_hash_to_instance_.end())
        {
            return get_archetype_instance(itr->second);
        }
        return nullptr;
    }

    archetype_instance* archetype_instance_registry::get_archetype_instance(uint32_t hash)
    {
        return const_cast<archetype_instance*>(const_cast<archetype_instance_registry const*>(this)->get_archetype_instance(hash));
    }
}
