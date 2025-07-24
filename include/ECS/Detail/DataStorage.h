#pragma once

namespace punk
{
    class data_storage_t
    {
    protected:
        data_storage_t() = default;
    public:
        virtual ~data_storage_t() = default;
        data_storage_t(data_storage_t const&) = delete;
        data_storage_t& operator=(data_storage_t const&) = delete;
        data_storage_t(data_storage_t&&) = delete;
        data_storage_t& operator=(data_storage_t&&) = delete;

    public:
        static data_storage_t* create_instance(archetype_registry_t* archetype_registry, entity_pool_t* entity_poll);

    protected:
        virtual archetype_instance_handle_t get_archetype_instance(entity_t entity) = 0;
        virtual archetype_instance_handle_t attach_archetype(archetype_ptr const& archetype) = 0;
    };
}