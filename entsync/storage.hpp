#pragma once


namespace entsync
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
  };
}
