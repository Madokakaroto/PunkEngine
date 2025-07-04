#pragma once

#include "ECS/Detail/Meta.h"
#include "Base/Utils/StaticFor.h"
#include "ECS/Detail/TypeInfoTraits.h"
#include "Base/Async/AsyncStaticFor.h"

namespace punk
{
    template <typename T>
    using Lazy = async_simple::coro::Lazy<T>;

    // runtime type system manages all runtime information about types, components, component groups & archetypes
    class runtime_type_system
    {
    public:
        struct type_info_deleter
        {
            void operator()(type_info_t* type_info) const noexcept
            {
                destroy_type_info(type_info);
            }
        };
        using type_info_ptr = std::unique_ptr<type_info_t, type_info_deleter>;

    public:
        runtime_type_system() = default;
        runtime_type_system(runtime_type_system const&) = delete;
        runtime_type_system& operator=(runtime_type_system const&) = delete;
        runtime_type_system(runtime_type_system&&) = delete;
        runtime_type_system& operator=(runtime_type_system&&) = delete;
        virtual ~runtime_type_system() = default;

        // factory
        static runtime_type_system* create_instance();

    public:
        // get type_info
        virtual type_info_t const* get_type_info(char const* type_name) const = 0;
        virtual type_info_t const* get_type_info(uint32_t type_name_hash) const = 0;

        // register a type info object created from meta interface
        virtual type_info_t const* register_type_info(type_info_t* type_info) = 0;

        // generic get_or_create_type_info
        template <typename T>
        type_info_t const* get_or_create_type_info()
        {
            using type_info_traits_t = type_info_traits<T>;
            auto const type_name = type_info_traits_t::get_type_name();
            auto const type_name_hash = hash_memory(type_name.c_str(), type_name.length());

            // query exist type info
            auto* type_info = get_type_info(type_name_hash);
            if (type_info)
            {
                return type_info;
            }

            // create a new type info
            using component_group = decltype(type_info_traits_t::get_component_group());
            type_create_info create_info
            {
                .type_name = type_name.c_str(),
                .size = type_info_traits_t::get_size(),
                .alignment = type_info_traits_t::get_alignment(),
                .vtable = type_info_traits_t::get_vtable(),
                .field_count = type_info_traits_t::get_field_count(),
                .component_tag = type_info_traits_t::get_component_tag(),
                .component_group = type_info_traits<component_group>::get_hash()
            };
            type_info_ptr new_type_info { create_type_info(create_info) };

            // initialize field
            if constexpr (type_info_traits_t::get_field_count() > 0)
            {
                static_for<0, type_info_traits_t::get_field_count()>(
                    [&]<size_t Index>()
                {
                    // query field info
                    auto* field_info = get_mutable_type_field_info(new_type_info.get(), Index);
                    assert(field_info);

                    // set type_info
                    using field_type = decltype(type_info_traits_t::template get_field_type<Index>());
                    auto* field_type_info = get_or_create_type_info<field_type>();
                    assert(field_type_info);
                    set_field_type(field_info, field_type_info);

                    // set type offset
                    auto const offset = type_info_traits_t::template get_field_offset<Index>();
                    set_field_offset(field_info, offset);
                });

                // initialize type hash component2
                update_hash_for_fields(new_type_info.get());
            }

            // 2-phrase commit
            type_info = register_type_info(new_type_info.get());
            if (type_info == new_type_info.get())
            {
                new_type_info.release();
            }
            return type_info;
        }

    public: // co-routine interface
        virtual Lazy<type_info_t const*> async_get_type_info(char const* type_name) const = 0;
        virtual Lazy<type_info_t const*> async_get_type_info(uint32_t type_name_hash) const = 0;
        virtual Lazy<type_info_t const*> async_register_type_info(type_info_t* type_info) = 0;
        
        template <typename T>
        Lazy<type_info_t const*> async_get_or_create_type_info()
        {
            using type_info_traits_t = type_info_traits<T>;
            auto const type_name = type_info_traits_t::get_type_name();
            auto const type_name_hash = hash_memory(type_name.c_str(), type_name.length());

            // query exist type info
            auto* type_info = co_await async_get_type_info(type_name_hash);
            if (type_info)
            {
                co_return type_info;
            }

            // create a new type info
            using component_group = decltype(type_info_traits_t::get_component_group());
            type_create_info create_info
            {
                .type_name = type_name.c_str(),
                .size = type_info_traits_t::get_size(),
                .alignment = type_info_traits_t::get_alignment(),
                .vtable = type_info_traits_t::get_vtable(),
                .field_count = type_info_traits_t::get_field_count(),
                .component_tag = type_info_traits_t::get_component_tag(),
                .component_group = type_info_traits<component_group>::get_hash()
            };
            type_info_ptr new_type_info { create_type_info(create_info) };

            // async nitialize field
            if constexpr (type_info_traits_t::get_field_count() > 0)
            {
                co_await async_static_for<0, type_info_traits_t::get_field_count()>(
                    [&]<size_t Index>() -> Lazy<void>
                {
                    // query field info
                    auto* field_info = get_mutable_type_field_info(new_type_info.get(), Index);
                    assert(field_info);

                    // set type_info
                    using field_type = decltype(type_info_traits_t::template get_field_type<Index>());
                    auto* field_type_info = co_await async_get_or_create_type_info<field_type>();
                    assert(field_type_info);
                    set_field_type(field_info, field_type_info);

                    // set type offset
                    auto const offset = type_info_traits_t::template get_field_offset<Index>();
                    set_field_offset(field_info, offset);
                });

                // initialize type hash component2
                update_hash_for_fields(new_type_info.get());
            }

            // 2-phrase commit
            type_info = co_await async_register_type_info(new_type_info.get());
            if (type_info == new_type_info.get())
            {
                new_type_info.release();
            }
            co_return type_info;
        }
    };
}

namespace punk
{
    template <typename ... Args>
    concept atleast_one_component_types = requires
    {
        // at least one
        sizeof...(Args) > 0;

        // all types are distinct
        std::negation_v<tuple_has_repeated_types<Args...>>;

        // all types are reflectable
        (reflectable<Args> && ...);
    };

    class runtime_archetype_system
    {
    protected:
        explicit runtime_archetype_system(runtime_type_system* runtime_type_system)
            : runtime_type_system_(runtime_type_system) {}

    public:
        runtime_archetype_system(runtime_archetype_system const&) = delete;
        runtime_archetype_system& operator=(runtime_archetype_system const&) = delete;
        runtime_archetype_system(runtime_archetype_system&&) = delete;
        runtime_archetype_system& operator=(runtime_archetype_system&&) = delete;
        virtual ~runtime_archetype_system() = default;

        // factory
        static runtime_archetype_system* create_instance(runtime_type_system* rtt_system);

    public:
        virtual archetype_ptr get_archetype(uint32_t hash) = 0;

        // runtime version of interfaces
        archetype_ptr get_or_create_archetype(type_info_t const** component_types, size_t component_count);
        archetype_ptr archetype_include_components(archetype_ptr const& archetype, size_t component_count, type_info_t const** component_types, uint32_t* include_orders = nullptr);
        archetype_ptr archetype_exclude_components(archetype_ptr const& archetype, type_info_t const** component_types, size_t component_count);

        // generic version of interfaces
        template <typename ... Args> requires atleast_one_component_types<Args...>
        archetype_ptr get_or_create_archetype()
        {
            assert(runtime_type_system_);

            // collect all runtime type information
            constexpr size_t count = sizeof...(Args);
            std::array<type_info_t const*, count> type_infos = { runtime_type_system_->get_or_create_type_info<Args>() ... };

            // sort types by hash
            std::stable_sort(type_infos.begin(), type_infos.end(),
                [](auto const* lhs, auto const* rhs)
                {
                    return get_type_hash(lhs) < get_type_hash(rhs);
                });
            return get_or_create_archetype_impl(type_infos.data(), count);
        }

        template <typename ... Args> requires atleast_one_component_types<Args...>
        auto archetype_include_components(archetype_ptr const& archetype) -> std::pair<archetype_ptr, std::array<size_t, sizeof...(Args)>>
        {
            assert(runtime_type_system_);
            constexpr size_t component_count = sizeof...(Args);

            // prepare component types
            std::array<type_info_t const*, component_count> component_types
            {
                (runtime_type_system_->get_or_create_type_info<Args>(), ...)
            };

            // prepare order
            std::array<size_t, component_count> orders{};

            // forward to runtime interface
            auto result_archetype = archetype_include_components(archetype, component_count, component_types.data(), orders.data());

            // return { result_archetype, orders } pair
            return { result_archetype, orders };
        }

        template <typename ... Args> requires atleast_one_component_types<Args...>
        archetype_ptr archetype_exclude_components(archetype_ptr const& archetype)
        {
            assert(runtime_type_system_);
            constexpr size_t component_count = sizeof...(Args);
            std::array<type_info_t const*, component_count> component_types
            {
                (runtime_type_system_->get_or_create_type_info<Args>(), ...)
            };
            return archetype_exclude_components(archetype, component_types.data(), component_count);
        }

    protected:
        virtual archetype_ptr get_or_create_archetype_impl(type_info_t const** sorted_component_types, size_t component_count) = 0;
        virtual archetype_ptr archetype_include_components_impl(archetype_ptr const& archetype, 
            size_t component_count, type_info_t const** component_types, uint32_t* include_orders) = 0;
        virtual archetype_ptr archetype_exclude_components_impl(archetype_ptr const& archetype, type_info_t const** component_types, size_t component_count) = 0;

    protected:
        runtime_type_system* runtime_type_system_;
    };
}