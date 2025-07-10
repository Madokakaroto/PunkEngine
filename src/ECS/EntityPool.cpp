#include "ECS/ECS.h"
#include "Base/Containers/Hive.h"

namespace punk
{
    struct entity_version_t
    {
        uint32_t padded;
        uint32_t version;
    };

    class entity_pool_impl_t : public entity_pool_t
    {
    public:
        virtual entity_t allocate_entity() override
        {
            auto [version_ptr, index] = entities_version_.construct();
            assert(version_ptr);
            auto const version = version_ptr->version++;
            return entity_t::compose(entity_handle_t{ static_cast<uint32_t>(index) }, version);
        }

        virtual void deallocate_entity(entity_t entity) override
        {
            auto const handle = entity.get_handle();
            auto const version = entity.get_version();

            auto const version_ptr = entities_version_.get(handle.get_value());
            if(version_ptr && version_ptr->version == version)
            {
                entities_version_.destruct(handle.get_value());
            }
        }

        virtual bool is_alive(entity_t entity) override
        {
            auto const handle = entity.get_handle();
            auto const version = entity.get_version();

            auto const version_ptr = entities_version_.get(handle.get_value());
            return version_ptr && version_ptr->version == version;
        }

        virtual entity_t restore_entity(entity_handle_t handle) override
        {
            auto [version_ptr, _] = entities_version_.construct_at(handle.get_value(), false);
            return entity_t::compose(handle, version_ptr->version);
        }

    private:
        hive<entity_version_t> entities_version_;
    };

    entity_pool_t* entity_pool_t::create_entity_pool()
    {
        return new entity_pool_impl_t{};
    }
}