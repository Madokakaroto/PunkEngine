#include "ECS/Chunk/ChunkNode.h"

namespace punk
{
    chunk_root_node::chunk_root_node(type_hash_t hash, size_t preallocate_chunk_count)
        : hash_(hash)
        , chunk_head_(nullptr)
        , chunk_tail_(nullptr)
        , free_chunk_head_(nullptr)
    {
        for (size_t i = 0; i < preallocate_chunk_count; ++i)
        {
            auto* node = allocate_chunk_node();
            insert_chunk_node(node);
        }
    }

    chunk_root_node::~chunk_root_node()
    {
        clear();
    }

    chunk_node_t* chunk_root_node::allocate_chunk_node()
    {
        chunk_node_t* node = nullptr;
        if (free_chunk_head_)
        {
            node = free_chunk_head_;
            free_chunk_head_ = free_chunk_head_->next;
            node->next = nullptr;
            node->prev = nullptr;
        }
        else
        {
            // allocate a new chunk node
            // TODO ... allocator
            node = new chunk_node_t{ nullptr, nullptr, nullptr };

            // malloc a new chunk memory
            // TODO ... allocator
            auto* chunk = reinterpret_cast<chunk_t*>(std::malloc(chunk_t::chunke_size));
            if(!chunk)
            {
                throw std::bad_alloc();
            }
            node->chunk = chunk;
        }
        return node;
    }

    void chunk_root_node::free_chunk_node(chunk_node_t* node)
    {
        if (!node)
        {
            return;
        }

        // add to free list
        node->next = free_chunk_head_;
        node->prev = nullptr;
        if (free_chunk_head_)
        {
            free_chunk_head_->prev = node;
        }
        free_chunk_head_ = node;
    }

    void chunk_root_node::insert_chunk_node(chunk_node_t* node)
    {
        if (!chunk_head_)
        {
            chunk_head_ = node;
            chunk_tail_ = node;
        }
        else
        {
            chunk_tail_->next = node;
            node->prev = chunk_tail_;
            chunk_tail_ = node;
        }
    }

    void chunk_root_node::remove_chunk_node(chunk_node_t* node)
    {

    }

    void chunk_root_node::clear()
    {

    }
}