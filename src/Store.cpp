#include "Types/Store.h"
#include "Types/RTTI.h"
#include "CoreTypes.h"
#include "Utils/Hive.hpp"

namespace punk
{
    class archetype_instance
    {
    private:
        uint32_t        index_;
        archetype_ptr   archtype_;

    public:
        explicit archetype_instance(archetype_ptr archetype)
            : archtype_(std::move(archetype)) {}
        archetype_instance() : archetype_instance(nullptr) {}
        ~archetype_instance() = default;
        archetype_instance(archetype_instance const&) = default;
        archetype_instance& operator=(archetype_instance const&) = default;
        archetype_instance(archetype_instance&&) = default;
        archetype_instance& operator=(archetype_instance&&) = default;

    public:
        uint32_t get_index() const noexcept { return index_; }
        void set_index(uint32_t index) noexcept { index_ = index; }
        bool is_non_archetype() const noexcept { return get_index() == 0; }
        archetype_ptr const& get_archetype() const { return archtype_; }
    };

    class archetype_instance_container final
    {
    private:
        hive<archetype_instance>                archetype_instances_;
        std::unordered_map<uint32_t, uint32_t>  archetype_hash_to_instance;

    public:
        explicit archetype_instance_container(archetype_ptr const& non_archetype)
        {
            attach_archetype(non_archetype);
        }

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
    };
}

