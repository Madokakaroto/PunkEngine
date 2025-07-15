#include "ECS/RTTI/RuntimeTypeSystem.h"

namespace punk
{
    runtime_type_registry_t* runtime_type_registry_t::create_instance()
    {
        return new runtime_type_system_impl{};
    }
}