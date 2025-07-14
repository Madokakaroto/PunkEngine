#pragma once

#include "ECS/CoreTypes.h"
#include "ECS/Detail/Meta.h"
#include <memory>

namespace punk
{
    struct chunk_node_t
    {
        chunk_t*        chunk;
        chunk_node_t*   next;
        chunk_node_t*   prev;
    };

    class chunk_root_node : public std::enable_shared_from_this<chunk_root_node>
    {
    private:
        type_hash_t    hash_;
        chunk_node_t*  chunk_head_;
        chunk_node_t*  chunk_tail_;
        chunk_node_t*  free_chunk_head_;

    public:
        chunk_root_node(type_hash_t hash, size_t preallocate_chunk_count);
        ~chunk_root_node();
        chunk_root_node(chunk_root_node const&) = delete;
        chunk_root_node& operator=(chunk_root_node const&) = delete;
        chunk_root_node(chunk_root_node&&) = default;
        chunk_root_node& operator=(chunk_root_node&&) = default;

    private:
        chunk_node_t* allocate_chunk_node();
        void free_chunk_node(chunk_node_t* node);
        void insert_chunk_node(chunk_node_t* node);
        void remove_chunk_node(chunk_node_t* node);
        void clear();
    };
}