#pragma once
#include "ECS/CoreTypes.h"
#include "ECS/Chunk/ChunkNode.h"

namespace punk
{
    class archetype_instance
    {
    public:
        static constexpr uint32_t non_archetype_index() { return (std::numeric_limits<uint32_t>::max)(); }

    private:
        uint32_t                    index_;
        archetype_ptr               archetype_;
        chunk_root_node             chunk_nodes_;

    public:
        explicit archetype_instance(archetype_ptr archetype)
            : index_(non_archetype_index())
            , archetype_(std::move(archetype))
            , chunk_nodes_(archetype_->hash, 0) // TODO... pre-allocated chunk
        {
        }

        ~archetype_instance() = default;
        archetype_instance(archetype_instance const&) = default;
        archetype_instance& operator=(archetype_instance const&) = default;
        archetype_instance(archetype_instance&&) = default;
        archetype_instance& operator=(archetype_instance&&) = default;

    public:
        uint32_t get_index() const noexcept { return index_; }
        void set_index(uint32_t index) noexcept { index_ = index; }
        uint32_t get_hash() const noexcept { return archetype_ ? archetype_->hash : 0; }
        bool is_non_archetype() const noexcept { return get_index() == 0; }
        archetype_ptr const& get_archetype() const { return archetype_; }
    };
}