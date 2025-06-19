#include "ECS/ECS.h"
#include "CoreTypes.h"
#include "Base/Containers/Hive.h"

namespace punk
{
    class archetype_instance
    {
    public:
        static constexpr uint32_t non_archetype_index() { return (std::numeric_limits<uint32_t>::max)(); }

    private:
        uint32_t        index_;
        archetype_ptr   archtype_;

    public:
        explicit archetype_instance(archetype_ptr archetype)
            : index_(non_archetype_index())
            , archtype_(std::move(archetype)) {}
        archetype_instance() 
            : archetype_instance(nullptr)
        {}
        ~archetype_instance() = default;
        archetype_instance(archetype_instance const&) = default;
        archetype_instance& operator=(archetype_instance const&) = default;
        archetype_instance(archetype_instance&&) = default;
        archetype_instance& operator=(archetype_instance&&) = default;

    public:
        uint32_t get_index() const noexcept { return index_; }
        void set_index(uint32_t index) noexcept { index_ = index; }
        uint32_t get_hash() const noexcept { return archtype_ ? archtype_->hash : 0; }
        bool is_non_archetype() const noexcept { return get_index() == 0; }
        archetype_ptr const& get_archetype() const { return archtype_; }
    };

    class archetype_instance_manager final
    {
    private:
        hive<archetype_instance>                archetype_instances_;
        std::unordered_map<uint32_t, uint32_t>  archetype_hash_to_instance;

    public:
        archetype_instance_manager() = default;
        ~archetype_instance_manager() = default;
        archetype_instance_manager(archetype_instance_manager const&) = delete;
        archetype_instance_manager& operator=(archetype_instance_manager const&) = delete;
        archetype_instance_manager(archetype_instance_manager&&) = default;
        archetype_instance_manager& operator=(archetype_instance_manager&&) = default;

        uint32_t attach_archetype(archetype_ptr archetype)
        {
            if(archetype)
            {
                return 0;
            }
            auto itr = archetype_hash_to_instance.find(archetype->hash);
            if(itr != archetype_hash_to_instance.end())
            {
                return itr->second;
            }

            auto [archetype_instance, index] = archetype_instances_.construct(std::move(archetype));
            archetype_instance->set_index(static_cast<uint32_t>(index));
            archetype_hash_to_instance.emplace(archetype->hash, archetype_instance->get_index());
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
            auto itr = archetype_hash_to_instance.find(hash);
            if(itr != archetype_hash_to_instance.end())
            {
                archetype_instances_.destruct(itr->second);
                archetype_hash_to_instance.erase(itr);
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
            archetype_hash_to_instance.erase(archetype_instance->get_hash());
        }

        archetype_instance const* get_archetype_instance(archetype_ptr const& archetype) const
        {
            return get_archetype_instance_by_hash(archetype->hash);
        }

        archetype_instance* get_archetype_instance(archetype_ptr const& archetype)
        {
            return const_cast<archetype_instance*>(const_cast<archetype_instance_manager const*>(this)->get_archetype_instance(archetype));
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
            return const_cast<archetype_instance*>(const_cast<archetype_instance_manager const*>(this)->get_archetype_instance_by_index(index));
        }

        archetype_instance const* get_archetype_instance_by_hash(uint32_t hash) const
        {
            auto itr = archetype_hash_to_instance.find(hash);
            if(itr != archetype_hash_to_instance.end())
            {
                return get_archetype_instance_by_index(itr->second);
            }
            return nullptr;
        }

        archetype_instance* get_archetype_instance_by_hash(uint32_t hash)
        {
            return const_cast<archetype_instance*>(const_cast<archetype_instance_manager const*>(this)->get_archetype_instance_by_hash(hash));
        }
    };
}

