#pragma once

#include "ECS/Archetype/ArchetypeInstance.h"
#include "Base/Containers/Hive.h"

namespace punk
{
    class archetype_instance_registry final
    {
    private:
        hive<archetype_instance>                archetype_instances_;
        std::unordered_map<uint32_t, uint32_t>  archetype_hash_to_instance_;

    public:
        archetype_instance_registry() = default;
        ~archetype_instance_registry() = default;
        archetype_instance_registry(archetype_instance_registry const&) = delete;
        archetype_instance_registry& operator=(archetype_instance_registry const&) = delete;
        archetype_instance_registry(archetype_instance_registry&&) = default;
        archetype_instance_registry& operator=(archetype_instance_registry&&) = default;

        uint32_t attach_archetype(archetype_ptr const& archetype)
        {
            if(!archetype)
            {
                return 0;
            }
            auto itr = archetype_hash_to_instance_.find(archetype->hash);
            if(itr != archetype_hash_to_instance_.end())
            {
                return itr->second;
            }

            auto [archetype_instance, index] = archetype_instances_.construct(std::move(archetype));
            archetype_instance->set_index(static_cast<uint32_t>(index));
            archetype_hash_to_instance_.emplace(archetype->hash, archetype_instance->get_index());
            return archetype_instance->get_index();
        }

        void detach_archetype(archetype_ptr const& archetype)
        {
            if(!archetype)
            {
                return;
            }

            detach_archetype_by_hash(archetype->hash);
        }

        void detach_archetype_by_hash(uint32_t hash)
        {
            auto itr = archetype_hash_to_instance_.find(hash);
            if(itr != archetype_hash_to_instance_.end())
            {
                archetype_instances_.destruct(itr->second);
                archetype_hash_to_instance_.erase(itr);
            }
        }

        void detach_archetype_by_index(uint32_t index)
        {
            if(index == archetype_instance::non_archetype_index())
            {
                return;
            }

            auto const* archetype_instance = archetype_instances_.get(index);
            if(!archetype_instance)
            {
                return;
            }

            archetype_instances_.destruct(index);
            archetype_hash_to_instance_.erase(archetype_instance->get_hash());
        }

        archetype_instance const* get_archetype_instance(archetype_ptr const& archetype) const
        {
            return get_archetype_instance_by_hash(archetype->hash);
        }

        archetype_instance* get_archetype_instance(archetype_ptr const& archetype)
        {
            return const_cast<archetype_instance*>(const_cast<archetype_instance_registry const*>(this)->get_archetype_instance(archetype));
        }

        archetype_instance const* get_archetype_instance_by_index(uint32_t index) const
        {
            if(index == archetype_instance::non_archetype_index())
            {
                return nullptr;
            }
            return archetype_instances_.get(index);
        }

        archetype_instance* get_archetype_instance_by_index(uint32_t index)
        {
            return const_cast<archetype_instance*>(const_cast<archetype_instance_registry const*>(this)->get_archetype_instance_by_index(index));
        }

        archetype_instance const* get_archetype_instance_by_hash(uint32_t hash) const
        {
            auto itr = archetype_hash_to_instance_.find(hash);
            if(itr != archetype_hash_to_instance_.end())
            {
                return get_archetype_instance_by_index(itr->second);
            }
            return nullptr;
        }

        archetype_instance* get_archetype_instance_by_hash(uint32_t hash)
        {
            return const_cast<archetype_instance*>(const_cast<archetype_instance_registry const*>(this)->get_archetype_instance_by_hash(hash));
        }
    };
}