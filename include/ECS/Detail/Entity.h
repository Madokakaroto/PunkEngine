#pragma once

#include <compare>
#include "Base/Types.h"
#include "ECS/Detail/Handle.h"

namespace punk
{
    // tagged type entity_handle: uint32_t handle value with entity_t
    using entity_handle_t = handle<class entity_t, uint32_t>;

    // the entity type
    class entity_t final
    {
    public:
        using value_type = uint64_t;
        using entity_tag = uint16_t;
        using entity_version = uint16_t;

        struct alignas(alignof(value_type)) entity_compose_type
        {
            uint32_t        handle;
            entity_tag      tag;
            entity_version  version;

            static constexpr value_type invalid_handle_value() noexcept
            {
                return entity_handle_t::invalid_handle_value();
            }
        };
        using entity_composed_value_t = handle<entity_compose_type, value_type>;

    private:
        entity_composed_value_t composed_value_;

    public:
        constexpr entity_t() noexcept = default;

        explicit constexpr entity_t(value_type value) noexcept
            : composed_value_(value)
        {
        }
        constexpr entity_t(uint32_t handle_value, entity_tag tag = 0, entity_version version = 0) noexcept
            : composed_value_(entity_compose_type{ handle_value, tag, version })
        {
        }
        ~entity_t() noexcept = default;
        entity_t(entity_t const&) noexcept = default;
        entity_t& operator=(entity_t const&) noexcept = default;
        entity_t(entity_t&&) noexcept = default;
        entity_t& operator=(entity_t&&) noexcept = default;

    public:
        static constexpr entity_t compose(uint32_t handle_value, entity_tag tag = 0, entity_version version = 0) noexcept
        {
            entity_t temp{ handle_value, tag, version };
            return temp;
        }

        static constexpr entity_t invalid_entity() noexcept
        {
            return compose(entity_handle_t::invalid_handle_value());
        }

        friend constexpr auto operator <=> (entity_t const& lhs, entity_t const& rhs) noexcept
        {
            return lhs.composed_value_ <=> rhs.composed_value_;
        }

    private:
        static constexpr entity_compose_type invalid_entity_compose() noexcept
        {
            return { entity_handle_t::invalid_handle_value(), 0, 0 };
        }

    public:
        constexpr bool is_valid() const noexcept
        {
            return composed_value_.is_valid();
        }

        explicit constexpr operator bool() const noexcept
        {
            return is_valid();
        }

        constexpr entity_handle_t get_handle() const noexcept
        {
            return entity_handle_t{ composed_value_.get_tag_value().handle };
        }

        constexpr entity_tag get_tag() const noexcept
        {
            return composed_value_.get_tag_value().tag;
        }

        constexpr entity_version get_version() const noexcept
        {
            return composed_value_.get_tag_value().version;
        }

        constexpr value_type get_value() const noexcept
        {
            return composed_value_.get_value();
        }

        explicit constexpr operator value_type() const noexcept
        {
            return get_value();
        }
    };
}
