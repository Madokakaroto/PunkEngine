#include "ECS/ECS.h"
#include "CoreTypes.h"
#include "Base/Utils/Hash.h"
#include "Base/Async/Async.h"
#include "Base/Async/AsyncStaticFor.h"

#ifndef PUNK_ALLOCA
#define PUNK_ALLOCA(type, count) static_cast<std::add_pointer_t<type>>(alloca(sizeof(type) * (count)))
#endif

namespace punk
{
    class runtime_type_system_impl final : public runtime_type_system
    {
    public:
        using spin_lock_t = async_simple::coro::SpinLock;
        using scoped_spin_lock_t = async_simple::coro::ScopedSpinLock;
        using type_info_ptr = std::unique_ptr<type_info_t>;
        using type_info_container = std::unordered_map<uint32_t, type_info_ptr>;

    private:
        mutable spin_lock_t type_lock;
        type_info_container runtime_type_infos;

    public:
        virtual ~runtime_type_system_impl() override = default;

        virtual type_info_t* get_type_info(char const* type_name) const override
        {
            if(!type_name)
            {
                return nullptr;
            }
            auto const type_name_hash = hash_memory(type_name, std::strlen(type_name));
            return get_type_info(type_name_hash);
        }

        virtual type_info_t* get_type_info(uint32_t type_name_hash) const override
        {
            scoped_spin_lock_t lock{ type_lock };
            auto itr = runtime_type_infos.find(type_name_hash);
            if(itr != runtime_type_infos.end())
            {
                return itr->second.get();
            }
            return nullptr;
        }

        virtual type_info_t const* register_type_info(type_info_t* type_info) override
        {
            auto const type_name_hash = type_info->hash.components.value1;
            scoped_spin_lock_t lock{ type_lock };
            auto const emplace_result = runtime_type_infos.emplace(type_name_hash, type_info);
            return emplace_result.first->second.get();
            // TODO ... conflict when hash.component.value2 is not the same
        }

        virtual Lazy<type_info_t const*> async_get_type_info(char const* type_name) const override
        {
            if(!type_name)
            {
                co_return nullptr;
            }
            auto const type_name_hash = hash_memory(type_name, std::strlen(type_name));
            co_return co_await async_get_type_info(type_name_hash);
        }

        virtual Lazy<type_info_t const*> async_get_type_info(uint32_t type_name_hash) const override
        {
            auto scope = type_lock.coScopedLock();
            auto itr = runtime_type_infos.find(type_name_hash);
            if (itr != runtime_type_infos.end())
            {
                co_return itr->second.get();
            }
            co_return nullptr;
        }

        virtual Lazy<type_info_t const*> async_register_type_info(type_info_t* type_info) override
        {
            auto const type_name_hash = type_info->hash.components.value1;
            auto scope = type_lock.coScopedLock();
            auto const emplace_result = runtime_type_infos.emplace(type_name_hash, type_info);
            co_return emplace_result.first->second.get();
        }
    };

    runtime_type_system* runtime_type_system::create_instance()
    {
        return new runtime_type_system_impl{};
    }
}

namespace punk
{
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
        explicit runtime_archetype_system_impl(runtime_type_system* runtime_type_system)
            : runtime_archetype_system(runtime_type_system) {}

    public:
        virtual archetype_ptr get_archetype(uint32_t hash) override
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

    protected:
        virtual archetype_ptr get_or_create_archetype_impl(type_info_t const** sorted_component_types, size_t component_count) override
        {
            // calculate the sorted components hash, as the archetype hash value
            auto* hash_ptr = PUNK_ALLOCA(uint32_t, component_count);
            std::ranges::subrange all_hash{ hash_ptr, hash_ptr + component_count };
            std::ranges::subrange all_comps{ sorted_component_types, sorted_component_types + component_count };
            std::ranges::transform(all_comps, all_hash.begin(),
                [](auto const* type_info)
                {
                    return get_type_name_hash(type_info);
                });
            auto const archetype_hash = hash_memory(reinterpret_cast<char const*>(all_hash.data()), sizeof(uint32_t) * all_hash.size());

            // if found one, return
            auto archetype = get_archetype(archetype_hash);
            if(archetype)
            {
                return archetype;
            }

            // allocate a new 
            archetype = allocate_archetype(archetype_hash, component_count);

            // initialize archetype info
            initialize_archetype(archetype.get(), sorted_component_types, component_count);

            // register archetype, two phrase commit
            archetype = register_archetype(archetype);
            return archetype;
        }

        virtual archetype_ptr archetype_include_components_impl(archetype_ptr const& archetype, 
            size_t component_count, type_info_t const** component_types, uint32_t* include_orders) override
        {
            auto const current_archetype_component_count = archetype->component_types.size();
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

        virtual archetype_ptr archetype_exclude_components_impl(archetype_ptr const& archetype,type_info_t const** component_types, size_t component_count) override
        {
            auto const components_count = archetype->component_types.size();
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

    private:
        archetype_ptr allocate_archetype(uint32_t hash, size_t component_count)
        {
            archetype_ptr archetype = archetype_ptr
            {
                new archetype_t{}, [this](archetype_t* archetype) { destroy_archetype(archetype); }
            };
            archetype->hash = 0;
            archetype->registered = false;
            archetype->component_types.reserve(component_count);
            archetype->component_infos.reserve(component_count);
            archetype->component_groups.reserve(component_count);
            return archetype;
        }

        void destroy_archetype(archetype_t* archetype)
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

        archetype_ptr register_archetype(archetype_ptr& archetype)
        {
            assert(archetype);
            scoped_spin_lock_t lock{ archetype_lock };
            auto [weak_archetype, result] = all_archetypes.emplace(archetype->hash, archetype);
            if(result)
            {
                return archetype;
            }
            auto result_archetype = weak_archetype->second.lock();
            assert(result_archetype);
            return result_archetype;
        }

        void unregister_archetype(archetype_t* archetype)
        {
            scoped_spin_lock_t lock{ archetype_lock };
            auto itr = all_archetypes.find(archetype->hash);
            if(itr != all_archetypes.end())
            {
                all_archetypes.erase(itr);
            }
        }

        void initialize_archetype(archetype_t* archetype, type_info_t const** component_types, size_t count)
        {
            /// initialize component types
            assert(archetype->component_types.capacity() == count);
            std::ranges::copy(component_types, component_types + count, std::back_inserter(archetype->component_types));

            /// initialize component infos
            std::ranges::transform(archetype->component_types, std::back_inserter(archetype->component_infos),
                [index{ 0u }](auto const*) mutable
                {
                    return component_info_t
                    {
                        .index_in_archetype = index++,
                        .index_in_group = invalid_index_value(),
                        .index_of_group = invalid_index_value(),
                        .offset_in_chunk = 0,
                    };
                });

            /// initialize component groups info
            // map to array of component_group_info_t
            std::ranges::transform(archetype->component_infos, std::back_inserter(archetype->component_groups),
                [archetype](auto const& component_info)
                {
                    auto const index = component_info.index_in_archetype;
                    component_group_info_t component_group
                    {
                        .hash = archetype->component_types[index]->hash.components.value1,
                        .capacity_in_chunk = 0,
                        .component_indices = { index },
                    };
                    return component_group;
                });
            // sort component group by has value 
            std::ranges::stable_sort(archetype->component_groups,
                [](auto const& lhs, auto const& rhs)
                {
                    return lhs.hash < rhs.hash;
                });
            // unique all the duplicated components, and merge the component indices
            auto [erase_begin, erase_last] = std::ranges::unique(archetype->component_groups,
                [](component_group_info_t& lhs, component_group_info_t& rhs)
                {
                    if(lhs.hash == rhs.hash)
                    {
                        lhs.component_indices.insert_range(lhs.component_indices.end(), rhs.component_indices);
                        return true;
                    }
                    return false;
                });
            archetype->component_groups.erase(erase_begin, erase_last);
            // update the component group index for all component
            std::ranges::for_each(archetype->component_groups,
                [index{ 0u }, archetype](auto& component_group) mutable
                {
                    component_group.index_in_archetype = index++;
                    std::ranges::for_each(component_group.component_indices,
                        [index{ 0u }, group_index = component_group.index_in_archetype, archetype](uint32_t component_index) mutable
                        {
                            auto& component_info = archetype->component_infos[component_index];
                            component_info.index_of_group = group_index;
                            component_info.index_in_group = index++;
                        });
                });

            // initialize memory capacity_in_chunk for component_group & offset_in_chunk for component
            search_chunck_offset_and_capacity(archetype);
        }

        void search_chunck_offset_and_capacity(archetype_t* archetype)
        {
            assert(archetype);

            for(auto& component_group : archetype->component_groups)
            {
                /// Search the capcity in chunk for each component group
                // 1. accumulate all the component size
                auto const all_comp_size = std::reduce(
                    component_group.component_indices.begin(),
                    component_group.component_indices.end(),
                    0u, [archetype](uint32_t acc, uint32_t component_index)
                    {
                        assert(component_index < archetype->component_types.size());
                        auto const* component_type = archetype->component_types[component_index];
                        return acc + component_type->size;
                    });

                // 2. We search the capacity with a initialize value of (data_block_size / all_comp_size + 1)
                constexpr uint32_t data_block_size = chunk_t::chunke_size - sizeof(chunk_t);
                uint32_t capacity = data_block_size / all_comp_size + 1;

                // 3. search the best capacity, by considering the alignments
                uint32_t chunk_size;
                std::vector<uint32_t> offsets(component_group.component_indices.size(), 0u);
                do
                {
                    capacity--;
                    chunk_size = calculate_chunk_size_and_offsets(archetype, component_group, capacity, offsets);
                } while (data_block_size <= chunk_size);

                // 4. now we have got the capacity result and offsets of all components, update into the archetype/component info
                component_group.capacity_in_chunk = capacity;
                for(uint32_t loop = 0; loop < offsets.size(); ++loop)
                {
                    archetype->component_infos[loop].offset_in_chunk = offsets[loop];
                }
            }
        }

        uint32_t calculate_chunk_size_and_offsets(archetype_t* archetype, component_group_info_t const& component_group, uint32_t capacity, std::vector<uint32_t>& offsets)
        {
            assert(archetype);
            uint32_t size = sizeof(chunk_t);

            offsets.clear();
            std::ranges::transform(component_group.component_indices, std::back_inserter(offsets),
                [archetype, &size, capacity](auto const& component_index)
                {
                    auto const* component_type = archetype->component_types[component_index];

                    // adjust alignment for each component
                    auto const offset = align_up(size, component_type->alignment);

                    // accumulate chunk memory size
                    size += component_type->size * capacity;

                    // return the offset
                    return offset;
                });

            return size;
        }
    };

    runtime_archetype_system* runtime_archetype_system::create_instance(runtime_type_system* rtt_system)
    {
        assert(rtt_system);
        if(!rtt_system)
        {
            return nullptr;
        }
        return new runtime_archetype_system_impl{ rtt_system };
    }
}