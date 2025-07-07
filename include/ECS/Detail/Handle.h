#pragma once

#include <limits>
#include <concepts>
#include <type_traits>

namespace punk
{
    namespace detail
    {
        template <typename Tag, typename UnderlyingType>
        concept has_invalue_handle_value = requires(Tag)
        {
            { Tag::invalue_handle_value() } -> std::same_as<UnderlyingType>;
        };
    }

    template <typename Tag, std::integral T>
    struct handle_base
    {
    public:
        using underlying_type = T;
        using tag_type = Tag;

    public:
        underlying_type value_;

        constexpr explicit handle_base(underlying_type value) noexcept
            : value_(value)
        {
        }
        ~handle_base() noexcept = default;
        handle_base(handle_base const&) noexcept = default;
        handle_base& operator=(handle_base const&) noexcept = default;
        handle_base(handle_base&&) noexcept = default;
        handle_base& operator=(handle_base&&) noexcept = default;

    public:
        friend constexpr auto operator <=> (handle_base const& lhs, handle_base const& rhs) noexcept
        {
            return lhs.value_ <=> rhs.value_;
        }

        static constexpr underlying_type invalid_handle_value() noexcept
        {
            if constexpr(detail::has_invalue_handle_value<tag_type, underlying_type>)
            {
                return tag_type::invalue_handle_value();
            }
            return (std::numeric_limits<underlying_type>::max)();
        }

        explicit constexpr operator underlying_type() const noexcept
        {
            return value_;
        }

        constexpr underlying_type get_value() const noexcept
        {
            return value_;
        }

        constexpr bool is_valid() const noexcept
        {
            return value_ != invalid_handle_value();
        }
    };

    template <typename Tag, std::integral T>
    struct handle : handle_base<Tag, T>
    {
    public:
        using base_type = handle_base<Tag, T>;
        using underlying_type = typename base_type::underlying_type;
        using tag_type = typename base_type::tag_type;

    public:
        static constexpr handle invalid_handle() noexcept
        {
            return handle{ base_type::invalid_handle_value() };
        }

        constexpr handle() noexcept
            : handle(invalid_handle())
        {
        }

        explicit constexpr handle(underlying_type value) noexcept
            : base_type(value)
        {
        }
    };

    template <typename Tag, std::integral T>
        requires(sizeof(Tag) == sizeof(T) && std::is_pod_v<Tag>)
    struct handle<Tag, T> : handle_base<Tag, T>
    {
    public:
        using base_type = handle_base<Tag, T>;
        using underlying_type = typename base_type::underlying_type;
        using tag_type = typename base_type::tag_type;

    public:
        static constexpr handle invalid_handle() noexcept
        {
            return handle{ base_type::invalid_handle_value() };
        }

        constexpr handle() noexcept
            : handle(invalid_handle())
        {
        }

        explicit constexpr handle(tag_type const& tag_value) noexcept
            : base_type(*std::launder(reinterpret_cast<underlying_type const*>(&tag_value)))
        {
        }

        explicit constexpr handle(underlying_type value) noexcept
            : base_type(value)
        {
        }

        constexpr tag_type get_tag_value() const noexcept
        {
            return *std::launder(reinterpret_cast<tag_type const*>(&this->value_));
        }
    };
}