#pragma once

#include "ECS/RTTI/RuntimeTypeSystem.h"
#include "Base/Utils/Hash.h"
#include "Base/Async/Async.h"
#include "Base/Async/AsyncStaticFor.h"

#ifndef PUNK_ALLOCA
#define PUNK_ALLOCA(type, count) static_cast<std::add_pointer_t<type>>(alloca(sizeof(type) * (count)))
#endif

namespace punk
{
    class runtime_archetype_system_impl final : public runtime_archetype_system
    {
    public:
        using spin_lock_t = async_simple::coro::SpinLock;
        using scoped_spin_lock_t = async_simple::coro::ScopedSpinLock;
        using archetype_container = std::unordered_map<uint32_t, archetype_weak>;

    private:
        archetype_container all_archetypes;
        spin_lock_t         archetype_lock;

    public:
        explicit runtime_archetype_system_impl(runtime_type_system* runtime_type_system);

        virtual archetype_ptr get_archetype(uint32_t hash) override;

    protected:
        virtual archetype_ptr get_or_create_archetype_impl(type_info_t const** sorted_component_types, size_t component_count) override;
        virtual archetype_ptr archetype_include_components_impl(archetype_ptr const& archetype, 
            size_t component_count, type_info_t const** component_types, uint32_t* include_orders) override;
        virtual archetype_ptr archetype_exclude_components_impl(archetype_ptr const& archetype,type_info_t const** component_types, size_t component_count) override;

    private:
        archetype_ptr allocate_archetype(uint32_t hash, size_t component_count);
        void destroy_archetype(archetype_t* archetype);
        archetype_ptr register_archetype(archetype_ptr& archetype);
        void unregister_archetype(archetype_t* archetype);
        void initialize_archetype(archetype_t* archetype, type_info_t const** component_types, size_t count);
        void search_chunck_offset_and_capacity(archetype_t* archetype);
        uint32_t calculate_chunk_size_and_offsets(archetype_t* archetype, uint32_t capacity, std::vector<uint32_t>& offsets);
    };
}