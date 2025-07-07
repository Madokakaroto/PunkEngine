#include "ECS/Detail/Entity.h"
#include "ECS/ECS.h"
#include "Base/Containers/Hive.h"

namespace punk
{
    class entity_pool_impl_t : public entity_pool_t
    {
    public:
        virtual entity_t allocate_entity(uint16_t tag) override
        {
            auto [version_ptr, index] = entities_version_.construct();
            assert(version_ptr);
            auto const version = (*version_ptr)++;
            return entity_t::compose(static_cast<uint32_t>(index), tag, version);
        }

        virtual void deallocate_entity(entity_t entity) override
        {
            auto const handle = entity.get_handle();
            auto const version = entity.get_version();

            auto const version_ptr = entities_version_.get(handle.get_value());
            if(version_ptr && *version_ptr == version)
            {
                entities_version_.destruct(handle.get_value());
            }
        }

        virtual bool is_alive(entity_t entity) override
        {
            auto const handle = entity.get_handle();
            auto const version = entity.get_version();

            auto const version_ptr = entities_version_.get(handle.get_value());
            return version_ptr && *version_ptr == version;
        }

        virtual entity_t restore_entity(entity_handle_t handle) override
        {
            auto const version_ptr = entities_version_.get(handle.get_value());
            if(!version_ptr)
            {
                return entity_t::invalid_entity();
            }
            return entity_t::compose(handle.get_value(), 0, *version_ptr);
        }
        
    private:
        hive<uint32_t> entities_version_;
    };

    entity_pool_t* entity_pool_t::create_entity_pool()
    {
        return new entity_pool_impl_t{};
    }
}