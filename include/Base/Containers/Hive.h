#pragma once

#include "Base/Types.h"
#include "Base/Containers/DynamicBitset.h"
#include <type_traits>

#ifndef PUNK_HIVE_GROUP_CAPACITY
#define PUNK_HIVE_GROUP_CAPACITY 128
#endif

namespace punk
{
    template <typename T, typename Alloc = std::allocator<T>>
    class hive_group
    {
    public:
        template <typename U, typename Allocator>
        friend class hive;
        using value_type = T;
        using const_value = std::add_const_t<value_type>;
        using pointer = std::add_pointer_t<value_type>;
        using const_pointer = std::add_pointer_t<const_value>;
        static constexpr size_t value_size = sizeof(value_type);
        static constexpr size_t value_align = alignof(value_type);
        union element_storage
        {
            alignas(T) std::array<uint8_t, value_size> bytes_;
            struct
            {
                uint16_t next_available_index;
                uint16_t prev_available_index;
            };
        };
        using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<element_storage>;
        using storage_type = std::vector<element_storage, allocator_type>;

    private:
        storage_type        storage_;
        dynamic_bitset<>    storage_bits_;
        size_t              first_available_index_;
        size_t              available_element_count_;
        size_t const        first_global_index_;

    public:
        explicit hive_group(size_t element_count, size_t first_global_index)
            : storage_(element_count)
            , storage_bits_(element_count, false)
            , first_available_index_(0)
            , available_element_count_(element_count)
            , first_global_index_(first_global_index)
        {
            init(element_count);
        }

        pointer get(size_t index)
        {
            return const_cast<pointer>(const_cast<hive_group const*>(this)->get(index));
        }

        const_pointer get(size_t index) const
        {
            assert(index < storage_.size());
            if(index < storage_.size() && test(index))
            {
                return get_ptr_as<value_type>(index);
            }
            return nullptr;
        }

        size_t size() const noexcept
        {
            return capacity() - available_element_count_;
        }

        size_t capacity() const noexcept
        {
            return storage_.size();
        }

        bool test(size_t index) const
        {
            return storage_bits_.test(index);
        }

        bool memory_in_range(const_pointer ptr) const noexcept
        {
            return ptr >= get_ptr_as<value_type>(0) && ptr <= get_ptr_as<value_type>(storage_.size() - 1);
        }

        bool memory_aligned(const_pointer ptr) const noexcept
        {
            std::ptrdiff_t distance = get_ptr_as<value_type>(0) - ptr;
            return distance % value_size == 0;
        }

        bool has_available_space() const noexcept
        {
            return available_element_count_ > 0;
        }

        size_t get_first_global_index() const noexcept
        {
            return first_global_index_;
        }

    public:
        template <typename ... Args> //requires(std::constructible_from<value_type, Args&&...>)
        auto construct(Args&& ... args) -> std::pair<pointer, size_t>
        {
            // branch: when hive group has no available space to construct a new element
            if(!has_available_space())
            {
                return { nullptr, invalid_index_value() };
            }

            auto const index = first_available_index_;
            auto value_ptr = construct_at_impl(first_available_index_, std::forward<Args>(args)...);
            return { value_ptr, index };
        }

        template <typename ... Args>
        auto construct_at(size_t index, bool overwrite_when_constructed, Args&& ... args) -> std::pair<pointer, bool>
        {
            // we are constructing on a constructed index
            if(test_allocated(index))
            {
                auto* value_ptr = get(index);
                if(!overwrite_when_constructed)
                {
                    return { value_ptr , false };
                }

                /// overwrite a new object
                // call destructor
                if constexpr(!std::is_trivially_destructible_v<value_type>)
                {
                    value_ptr->~value_type();
                }
                // call constructor
                if(!std::is_trivially_constructible_v<value_type, Args&&...>)
                {
                    new (value_ptr) value_type{ std::forward<Args>(args)... };
                }
                return { value_ptr, true };
            }

            auto value_ptr = construct_at_impl(index, std::forward<Args>(args)...);
            return { value_ptr, true };
        }

        void destruct(pointer ptr) noexcept
        {
            assert(available_element_count_ < capacity());
            assert(ptr);

            // check pointer in group & aligned
            assert(memory_in_range(ptr));
            assert(memory_aligned(ptr));
            if(!memory_in_range(ptr) || !memory_aligned(ptr))
            {
                return;
            }

            // update new state
            auto const space_index = static_cast<size_t>(std::distance(get_ptr_as<value_type>(0), ptr));
            assert(space_index < capacity());

            // check double free
            if(!test_allocated(space_index))
            {
                return;
            }

            // call destructor
            if constexpr(!std::is_trivially_destructible_v<value_type>)
            {
                ptr->~value_type();
            }

            // update hive group state
            set_next_index(space_index, static_cast<uint16_t>(first_available_index_));
            set_prev_index(space_index, invalid_short_index_value());
            mark_destroyed(space_index);
            first_available_index_ = space_index;
            available_element_count_++;
            assert(available_element_count_ < capacity());
        }

        void destruct(size_t pos) noexcept
        {
            if(pos >= capacity() || !test_allocated(pos))
            {
                return;
            }

            destruct(get_ptr_as<value_type>(pos));
        }

    private:
        void init(size_t element_count)
        {
            for(uint16_t loop = 0; loop < element_count; ++loop)
            {
                storage_[loop].next_available_index = loop + 1;
                storage_[loop].prev_available_index = loop - 1;
            }
            storage_[0].prev_available_index = invalid_short_index_value();
            storage_[element_count - 1].next_available_index = invalid_short_index_value();
        }

        uint8_t* get_ptr(size_t index)
        {
            return storage_[index].bytes_.data();
        }

        uint8_t const* get_ptr(size_t index) const
        {
            return storage_[index].bytes_.data();
        }

        uint16_t get_next_index(size_t index) const
        {
            return storage_[index].next_available_index;
        }

        uint16_t get_prev_index(size_t index) const
        {
            return storage_[index].prev_available_index;
        }

        void set_next_index(size_t index, uint16_t next_index)
        {
            storage_[index].next_available_index = next_index;
        }

        void set_prev_index(size_t index, uint16_t prev_index)
        {
            storage_[index].prev_available_index = prev_index;
        }

        template <typename U>
        U* get_ptr_as(size_t index)
        {
            return std::launder(reinterpret_cast<U*>(get_ptr(index)));
        }

        template <typename U>
        U const* get_ptr_as(size_t index) const
        {
            return std::launder(reinterpret_cast<U const*>(get_ptr(index)));
        }

        bool test_allocated(size_t pos) const
        {
            assert(pos < capacity());
            return storage_bits_.test(pos);
        }

        void mark_allocated(size_t pos)
        {
            assert(pos < capacity());
            storage_bits_.set(pos);
        }

        void mark_destroyed(size_t pos)
        {
            assert(pos < capacity());
            storage_bits_.reset(pos);
        }

        template <typename ... Args>
        pointer construct_at_impl(size_t index, Args&& ... args)
        {
            assert(!test_allocated(index));

            // cache next & prev available index
            auto const next_available_index = get_next_index(index);
            auto const prev_available_index = get_prev_index(index);

            // call constructor
            pointer value_ptr = nullptr;
            if(!std::is_trivially_constructible_v<value_type, Args&&...>)
            {
                auto* storage_ptr = get_ptr(index);
                value_ptr = new (storage_ptr) value_type{ std::forward<Args>(args)... };
            }

            // update linked list
            if(next_available_index != invalid_index_value())
            {
                set_prev_index(next_available_index, prev_available_index);
            }
            if(prev_available_index != invalid_index_value())
            {
                set_next_index(prev_available_index, next_available_index);
            }

            // update allocate states
            if(index == first_available_index_)
            {
                first_available_index_ = next_available_index;
            }
            available_element_count_--;
            mark_allocated(index);

            return value_ptr;
        }
    };

    template <typename T, typename Alloc = std::allocator<T>>
    class hive
    {
    public:
        using hive_group_type = hive_group<T, Alloc>;
        using value_type = typename hive_group_type::value_type;
        using pointer = typename hive_group_type::pointer;
        using const_pointer = typename hive_group_type::const_pointer;
        using allocator_type = Alloc;

    private:
        using hive_group_ptr = std::unique_ptr<hive_group_type>;
        using hive_group_ptr_allocator = typename std::allocator_traits<allocator_type>::template rebind_alloc<hive_group_ptr>;
        using hive_group_allocator = typename std::allocator_traits<allocator_type>::template rebind_alloc<hive_group_type>;
        std::vector<hive_group_ptr, hive_group_ptr_allocator> hive_groups_;
        static constexpr size_t hive_group_capacity = PUNK_HIVE_GROUP_CAPACITY;
        static constexpr size_t hive_group_memory_size = hive_group_capacity * sizeof(value_type);
        
        static_assert(hive_group_capacity < (std::numeric_limits<uint16_t>::max)());

    public:
        hive()
        {
            create_initial_group();
        }
        ~hive()
        {
            // TODO ...
        }
        hive(hive&&) = default; // TODO ...
        hive& operator=(hive&&) = default; // TODO ...
        hive(hive const& other) = delete; // TODO ...
        hive& operator=(hive const& other) = delete; // TODO ...

    public:
        template <typename ... Args> requires(std::constructible_from<value_type, Args&&...>)
        auto construct(Args&& ... args) -> std::pair<value_type*, size_t>
        {
            // find or create a hive_group iterator to construct our new element
            auto itr = std::ranges::find_if(hive_groups_,
                [](auto const& hive_group_ptr)
                {
                    return hive_group_ptr->has_available_space();
                });
            if(itr == hive_groups_.end())
            {
                itr = append_new_group();
            }

            auto result = (*itr)->construct(std::forward<Args>(args)...);
            result.second += (*itr)->get_first_global_index();
            return result;
        }

        template <typename ... Args> requires(std::constructible_from<value_type, Args&&...>)
        auto construct_at(size_t index, bool overwrite_when_constructed, Args&& ... args) -> std::pair<value_type*, bool>
        {
            auto const index_of_group = index / hive_group_capacity;
            auto const index_in_group = index % hive_group_capacity;

            if(index_of_group >= hive_groups_.size())
            {
                auto const group_count = index_of_group - hive_groups_.size() + 1;
                for(auto loop = 0; loop < group_count; ++loop)
                {
                    append_new_group();
                }
            }

            assert(!hive_groups_.empty());
            return hive_groups_[index_of_group].construct_at(index_in_group, overwrite_when_constructed, std::forward<Args>(args)...);
        }

        void destruct(const_pointer ptr) noexcept
        {
            auto itr = std::ranges::find_first_of(hive_groups_,
                [=](auto const& hive_group_ptr)
                {
                    return hive_group_ptr->memory_in_range(ptr) && itr->memory_aligned(ptr);
                });
            if(itr == hive_groups_.end())
            {
                return;
            }

            itr->destruct(ptr);
        }

        void destruct(size_t index) noexcept
        {
            auto const index_of_group = index / hive_group_capacity;
            auto const index_in_group = index % hive_group_capacity;
            
            if(index_of_group >= hive_groups_.size())
            {
                return;
            }
            auto const& hive_group_ptr = hive_groups_[index_of_group];
            assert(hive_group_ptr);
            assert(index >= hive_group_ptr->get_first_global_index());
            assert(index_in_group < hive_group_ptr->capacity());
            hive_group_ptr->destruct(index_in_group);
        }

        const_pointer get(size_t global_index) const
        {
            auto itr = get_hive_group(global_index);
            if(itr == hive_groups_.cend())
            {
                return nullptr;
            }

            auto const index_in_group = global_index - (*itr)->get_first_global_index();
            return (*itr)->get(index_in_group);
        }

        pointer get(size_t global_index)
        {
            return const_cast<pointer>(const_cast<hive const*>(this)->get(global_index));
        }

    private:
        void create_initial_group()
        {
            auto initial_group = create_new_group(hive_group_capacity, 0);
            hive_groups_.push_back(std::move(initial_group));
        }

        auto append_new_group()
        {
            assert(!hive_groups_.empty());
            auto const first_global_index = hive_groups_.back()->get_first_global_index() + hive_group_capacity;
            // create new group
            auto new_hive_group = create_new_group(hive_group_capacity, first_global_index);
            return hive_groups_.insert(hive_groups_.end(), std::move(new_hive_group));
        }

        hive_group_ptr create_new_group(size_t capacity, size_t first_global_index)
        {
            auto* ptr = hive_group_allocator{}.allocate(1);
            new (ptr) hive_group_type{ capacity, first_global_index };
            hive_group_ptr result{ ptr };
            return result;
        }

        auto get_hive_group(size_t global_index) const
        {
            auto const index_of_group = global_index / hive_group_capacity;
            auto const index_in_group = global_index % hive_group_capacity;
            if(index_of_group >= hive_groups_.size())
            {
                return hive_groups_.cend();
            }
            auto itr = hive_groups_.cbegin() + index_of_group;
            assert(*itr);
            assert(global_index >= (*itr)->get_first_global_index());
            assert(index_in_group < (*itr)->capacity());
            return itr;
        }

        auto get_hive_group(size_t global_index)
        {
            auto citr = const_cast<hive*>(this)->get_hive_group(global_index);
            auto const distance = std::ranges::distance(hive_groups_.cbegin(), citr);
            return hive_groups_.begin() + distance;
        }
    };
}