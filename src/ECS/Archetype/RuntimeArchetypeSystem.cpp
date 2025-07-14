#include "ECS/Archetype/RuntimeArchetypeSystem.h"

namespace punk
{
    runtime_archetype_system* runtime_archetype_system::create_instance(runtime_type_system* rtt_system)
    {
        assert(rtt_system);
        if(!rtt_system)
        {
            return nullptr;
        }
        return new runtime_archetype_system_impl{ rtt_system };
    }

    archetype_ptr runtime_archetype_system::get_or_create_archetype(type_info_t const** component_types, size_t component_count)
    {
        if (component_count == 0)
        {
            return nullptr;
        }

        std::ranges::subrange all_comps{ component_types, component_types + component_count };

        // failed to create archetype when any of the components has no component tag
        if (std::ranges::any_of(all_comps,
            [](auto const* type_info)
            {
                return get_type_component_tag(type_info) == component_tag_t::none;
            }))
        {
            return nullptr;
        }

        // stable sort components by type hash value
        std::ranges::stable_sort(all_comps,
            [](auto const* lhs, auto const* rhs)
            {
                return get_type_name_hash(lhs) < get_type_name_hash(rhs);
            });

        // remove unique components
        auto [end, _] = std::ranges::unique(all_comps,
            [](auto const* lhs, auto const* rhs)
            {
                return get_type_name_hash(lhs) == get_type_name_hash(rhs);
            });

        // adapt the component count
        component_count = std::ranges::distance(all_comps.begin(), end);

        // forward to implementation
        return get_or_create_archetype_impl(component_types, component_count);
    }

    archetype_ptr runtime_archetype_system::archetype_include_components(archetype_ptr const& archetype,
        size_t component_count, type_info_t const** component_types, uint32_t* include_orders)
    {
        if(!component_types || component_count == 0)
        {
            return archetype;
        }
        include_orders = include_orders ? include_orders : PUNK_ALLOCA(uint32_t, component_count);

        std::ranges::subrange orders{ include_orders, include_orders + component_count };
        // generate an index sequence for orders as the initial value
        std::ranges::generate(orders, [index{ 0u }]() mutable { return index++; });
        // sort the index, just like the indirect array of component_types
        std::ranges::stable_sort(orders,
            [component_types](auto const lhs, auto const rhs)
            {
                return get_type_name_hash(component_types[lhs]) < get_type_name_hash(component_types[rhs]);
            });
        // remove duplicate components
        auto [invalid_begin, invalid_end] = std::ranges::unique(orders,
            [component_types](auto const lhs, auto const rhs)
            {
                return get_type_name_hash(component_types[lhs]) == get_type_name_hash(component_types[rhs]);
            });
        // mark the removed duplicate components` order as invalid value
        std::ranges::for_each(invalid_begin, invalid_end, [](auto& order) { order = invalid_index_value(); });

        component_count = std::ranges::distance(orders.begin(), invalid_begin);
        return archetype_include_components_impl(archetype, component_count, component_types, include_orders);
    }

    archetype_ptr runtime_archetype_system::archetype_exclude_components(archetype_ptr const& archetype, type_info_t const** component_types, size_t component_count)
    {
        if(!component_types || component_count == 0)
        {
            return archetype;
        }

        std::ranges::stable_sort(component_types, component_types + component_count,
            [](auto const* lhs, auto const* rhs)
            {
                return get_type_name_hash(lhs) < get_type_name_hash(rhs);
            });

        return archetype_exclude_components(archetype, component_types, component_count);
    }
}


namespace punk
{
    runtime_archetype_system_impl::runtime_archetype_system_impl(runtime_type_system* runtime_type_system)
        : runtime_archetype_system(runtime_type_system) {}

    archetype_ptr runtime_archetype_system_impl::get_archetype(uint32_t hash)
    {
        scoped_spin_lock_t lock{ archetype_lock };
        auto itr = all_archetypes.find(hash);
        if (itr != all_archetypes.end())
        {
            auto result = itr->second.lock();
            return result;
        }
        return nullptr;
    }

    archetype_ptr runtime_archetype_system_impl::get_or_create_archetype_impl(type_info_t const** sorted_component_types, size_t component_count)
    {
        auto* hash_ptr = PUNK_ALLOCA(uint32_t, component_count);
        std::ranges::subrange all_hash{ hash_ptr, hash_ptr + component_count };
        std::ranges::subrange all_comps{ sorted_component_types, sorted_component_types + component_count };
        std::ranges::transform(all_comps, all_hash.begin(),
            [](auto const* type_info)
            {
                return get_type_name_hash(type_info);
            });
        auto const archetype_hash = hash_memory(reinterpret_cast<char const*>(all_hash.data()), sizeof(uint32_t) * all_hash.size());

        auto archetype = get_archetype(archetype_hash);
        if(archetype)
        {
            return archetype;
        }

        archetype = allocate_archetype(archetype_hash, component_count);
        initialize_archetype(archetype.get(), sorted_component_types, component_count);
        archetype = register_archetype(archetype);
        return archetype;
    }

    archetype_ptr runtime_archetype_system_impl::archetype_include_components_impl(archetype_ptr const& archetype, 
        size_t component_count, type_info_t const** component_types, uint32_t* include_orders)
    {
        auto const current_archetype_component_count = archetype->component_infos.size();
        auto const new_archetype_components_count = current_archetype_component_count + component_count;
        auto* component_infos_ptr = PUNK_ALLOCA(type_info_t const*, new_archetype_components_count);
        std::ranges::subrange merge_comp_type_infos
        {
            component_infos_ptr,
            component_infos_ptr + new_archetype_components_count
        };

        std::ranges::subrange orders{ include_orders, include_orders + component_count };
        uint32_t i = 0, j = 0, index = 0;
        while(i < current_archetype_component_count && j < component_count)
        {
            auto const* current_component_type = archetype->component_types[i];
            auto const current_index = orders[j];
            auto const* current_append_type = component_types[current_index];

            auto const current_hash = get_type_name_hash(current_component_type);
            auto const append_hash = get_type_name_hash(current_append_type);

            if(current_hash < append_hash)
            {
                merge_comp_type_infos[index++] = current_component_type;
                ++i;
            }
            else if(current_hash > append_hash)
            {
                orders[current_index] = index++;
                merge_comp_type_infos[index] = current_append_type;
                ++j;
            }
            else //(current_hash == append_hash)
            {
                orders[current_index] = invalid_index_value();
                ++j;
            }
        }

        while(i < current_archetype_component_count)
        {
            auto const* current_component_type = archetype->component_types[j];
            merge_comp_type_infos[index++] = current_component_type;
            ++i;
        }
        while(j < component_count)
        {
            auto const current_index = orders[j];
            auto const* current_append_type = component_types[current_index];
            orders[current_index] = index++;
            merge_comp_type_infos[index] = current_append_type;
            ++j;
        }

        return get_or_create_archetype_impl(merge_comp_type_infos.data(), merge_comp_type_infos.size());
    }

    archetype_ptr runtime_archetype_system_impl::archetype_exclude_components_impl(archetype_ptr const& archetype,type_info_t const** component_types, size_t component_count)
    {
        auto const components_count = archetype->component_infos.size();
        auto* diff_comp_begin = PUNK_ALLOCA(type_info_t const*, components_count);
        std::ranges::subrange difference_type_infos{ diff_comp_begin, diff_comp_begin + components_count };
        std::ranges::subrange subtract_type_infos{ component_types, component_types + component_count };

        auto [_, diff_comp_end] = std::ranges::set_difference(
            archetype->component_types,
            subtract_type_infos,
            difference_type_infos.begin(),
            [](type_info_t const* lhs, type_info_t const* rhs)
            {
                return get_type_name_hash(lhs) < get_type_name_hash(rhs);
            });

        return get_or_create_archetype_impl(diff_comp_begin, std::ranges::distance(diff_comp_end, diff_comp_begin));
    }

    archetype_ptr runtime_archetype_system_impl::allocate_archetype(uint32_t hash, size_t component_count)
    {
        archetype_ptr archetype = archetype_ptr
        {
            new archetype_t{}, [this](archetype_t* archetype) { destroy_archetype(archetype); }
        };
        archetype->hash = hash;
        archetype->capacity_in_chunk = 0;
        archetype->registered = false;
        archetype->component_types.reserve(component_count);
        archetype->component_infos.reserve(component_count);
        return archetype;
    }

    void runtime_archetype_system_impl::destroy_archetype(archetype_t* archetype)
    {
        if(archetype)
        {
            if(archetype->registered)
            {
                unregister_archetype(archetype);
                archetype->registered = false;
            }

            delete archetype;
        }
    }

    archetype_ptr runtime_archetype_system_impl::register_archetype(archetype_ptr& archetype)
    {
        assert(archetype);
        scoped_spin_lock_t lock{ archetype_lock };
        auto [weak_archetype, result] = all_archetypes.emplace(archetype->hash, archetype);
        if(result)
        {
            archetype->registered = true;
            return archetype;
        }
        auto result_archetype = weak_archetype->second.lock();
        assert(result_archetype);
        return result_archetype;
    }

    void runtime_archetype_system_impl::unregister_archetype(archetype_t* archetype)
    {
        scoped_spin_lock_t lock{ archetype_lock };
        auto itr = all_archetypes.find(archetype->hash);
        if(itr != all_archetypes.end())
        {
            all_archetypes.erase(itr);
        }
    }

    void runtime_archetype_system_impl::initialize_archetype(archetype_t* archetype, type_info_t const** component_types, size_t count)
    {
        assert(archetype->component_infos.capacity() == count);

        // copy to component types
        std::ranges::copy(component_types, component_types + count, std::back_inserter(archetype->component_types));

        // create component infos
        std::ranges::transform(archetype->component_types, std::back_inserter(archetype->component_infos),
            [index{ 0u }](auto const* comp_type_info) mutable
            {
                return component_info_t
                {
                    .index_in_archetype = index++,
                    .offset_in_chunk = 0,
                };
            });

        // calculate offsets for all components
        search_chunck_offset_and_capacity(archetype);
    }

    void runtime_archetype_system_impl::search_chunck_offset_and_capacity(archetype_t* archetype)
    {
        assert(archetype);

        auto const all_comp_size = std::reduce(
            archetype->component_types.begin(),
            archetype->component_types.end(),
            0u, [archetype](uint32_t acc, auto const* component_type)
            {
                assert(component_type);
                return acc + component_type->size;
            });

        constexpr uint32_t data_block_size = chunk_t::chunke_size - sizeof(chunk_t);
        uint32_t capacity = data_block_size / all_comp_size + 1;

        uint32_t chunk_size;
        std::vector<uint32_t> offsets(archetype->component_infos.size(), 0u);
        do
        {
            capacity--;
            chunk_size = calculate_chunk_size_and_offsets(archetype, capacity, offsets);
        } while (data_block_size < chunk_size);

        archetype->capacity_in_chunk = capacity;
        for(uint32_t loop = 0; loop < offsets.size(); ++loop)
        {
            archetype->component_infos[loop].offset_in_chunk = offsets[loop];
        }
    }

    uint32_t runtime_archetype_system_impl::calculate_chunk_size_and_offsets(archetype_t* archetype, uint32_t capacity, std::vector<uint32_t>& offsets)
    {
        assert(archetype);
        uint32_t size = sizeof(chunk_t);

        offsets.clear();
        std::ranges::transform(archetype->component_types, std::back_inserter(offsets),
            [&size, capacity](auto const* component_type)
            {
                auto const offset = align_up(size, component_type->alignment);
                size += component_type->size * capacity;
                return offset;
            });

        return size;
    }
}