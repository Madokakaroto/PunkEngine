#include "gtest/gtest.h"
#include "Base/Math/Math.h"
#include "Base/Reflection/StaticReflection.h"
#include "DirectXMath.h"
#include "ECS/ECS.h"
#include "ECS/CoreTypes.h"

using punk::entity_handle_t;
struct transform_group{};

struct hierarchy_component_t
{
    using component_tag = punk::data_component_tag;
    using component_group = transform_group;

    entity_handle_t parent;
    entity_handle_t child;
    entity_handle_t next_sibling;
    entity_handle_t prev_sibling;
};

struct transform_component_t
{
    using component_tag = punk::data_component_tag;
    using component_group = transform_group;

    punk::vector4   translation;
    punk::vector4   rotation;
    punk::vector4   scale;
    punk::matrix4x4 transform;
    punk::matrix4x4 inverse_transform;
};
PUNK_REFLECT(transform_component_t, translation, rotation, scale, transform, inverse_transform);

struct aabb_component_t
{
    using component_tag = punk::data_component_tag;

    punk::float3    min;
    punk::float3    max;
};

struct name_component_t
{
    using component_tag = punk::data_component_tag;

    std::string     name;
};

TEST(ECS, DummyTest) 
{
    EXPECT_TRUE(true);
    std::unique_ptr<punk::runtime_type_system> rtts
    { 
        punk::runtime_type_system::create_instance()
    };
    std::unique_ptr<punk::runtime_archetype_system> archetype_system
    { 
        punk::runtime_archetype_system::create_instance(rtts.get())
    };

    auto archetype_ptr = archetype_system->get_or_create_archetype
    <
        name_component_t,
        transform_component_t,
        aabb_component_t,
        hierarchy_component_t
    >();

    std::cout << archetype_ptr->hash << std::endl;
}