#pragma once

#include <compare>
#include "Base/Types.h"
#include "ECS/Detail/Handle.h"

namespace punk
{
    // tagged type entity_handle_t: uint32_t handle value with entity_t
    using entity_handle_t = handle<class entity_t, uint32_t>;

    struct alignas(alignof(uint64_t)) entity_compose_type
    {
        uint32_t        handle;
        uint32_t        version;

        static constexpr uint64_t invalid_handle_value() noexcept
        {
            return entity_handle_t::invalid_handle_value();
        }
    };
    using entity_composed_handle_t = handle<entity_compose_type, uint64_t>;

    // the entity type
    class entity_t final
    {
    private:
        entity_composed_handle_t composed_value_;

    public:
        constexpr entity_t() noexcept = default;

        explicit constexpr entity_t(uint64_t value) noexcept
            : composed_value_(value)
        {
        }
        constexpr entity_t(entity_handle_t handle_value, uint32_t version = 0) noexcept
            : composed_value_(entity_compose_type{ handle_value.get_value(), version })
        {
        }
        ~entity_t() noexcept = default;
        entity_t(entity_t const&) noexcept = default;
        entity_t& operator=(entity_t const&) noexcept = default;
        entity_t(entity_t&&) noexcept = default;
        entity_t& operator=(entity_t&&) noexcept = default;

    public:
        static constexpr entity_t compose(entity_handle_t handle, uint16_t version = 0) noexcept
        {
            entity_t temp{ handle, version };
            return temp;
        }

        static constexpr entity_t invalid_entity() noexcept
        {
            return compose(entity_handle_t::invalid_handle());
        }

        friend constexpr auto operator <=> (entity_t const& lhs, entity_t const& rhs) noexcept
        {
            return lhs.composed_value_ <=> rhs.composed_value_;
        }

    private:
        static constexpr entity_compose_type invalid_entity_compose() noexcept
        {
            return { entity_handle_t::invalid_handle_value(), 0 };
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

        constexpr uint32_t get_version() const noexcept
        {
            return composed_value_.get_tag_value().version;
        }

        constexpr uint64_t get_value() const noexcept
        {
            return composed_value_.get_value();
        }

        explicit constexpr operator uint64_t() const noexcept
        {
            return get_value();
        }
    };
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(entity_t, punk::entity_t);
}
