#include "ECS/RTTI/RuntimeTypeSystem.h"

namespace punk
{
    runtime_type_system* runtime_type_system::create_instance()
    {
        return new runtime_type_system_impl{};
    }
}