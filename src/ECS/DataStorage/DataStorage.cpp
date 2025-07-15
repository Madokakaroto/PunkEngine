#include "ECS/DataStorage/DataStorage.h"

namespace punk
{
    data_storage_t* data_storage_t::create_instance(archetype_registry_t* archetype_registry)
    {
        return nullptr;
    }
}

namespace punk
{
    data_storage_impl::data_storage_impl(archetype_registry_t* archetype_registry)
        : data_storage_t()
        , archetype_registry_(archetype_registry)
    {
        assert(archetype_registry_);
    }

}