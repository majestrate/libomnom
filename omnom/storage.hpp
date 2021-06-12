#pragma once
#include "entity.hpp"

#include <set>

namespace omnom
{
    class EntityStorage
    {
       public:
        virtual ~EntityStorage() = default;

        /***
            return true if we have an entity stored already
            return false if we don't have an entity stored
         */
        virtual bool
        HasEntity(Entity ent) const = 0;

        /***
            store an entity in a persistent manner
         */
        virtual void
        StoreEntity(Entity ent) = 0;

        /***
            maybe fetch an entity from our storage given it's id
         */
        virtual std::optional<Entity>
        GetEntityByID(EntityID id) const = 0;

        /***
            get the entity id that is the "highest value"
        */
        virtual EntityID
        GetTopEntityID() const = 0;
    };
}  // namespace omnom
