#pragma once
#include "crypto_hash.hpp"
#include "entity.hpp"
#include <oxenmq/oxenmq.h>
#include <future>
#include <optional>
#include <unordered_map>
#include <variant>

namespace entsync
{

  class Context;
  
  class EntitySearcher
  {
    Context * const _ctx;
    
    template<typename Key_type>
    struct Bucket
    {
      std::unordered_map<Key_type, std::function<void(std::optional<Entity>)>> m_PendingSearches;
    };
   
    std::unordered_map<EntityKind, Bucket<EntityID>>  m_EntitySearches;
    
  public:
    explicit EntitySearcher(Context * ctx);
    
    EntitySearcher(const EntitySearcher &) = delete;
    EntitySearcher(EntitySearcher &&) = delete;

    /// obtain an entity of kind by id
    void
    ObtainByID(EntityKind kind, EntityID id, std::function<void(std::optional<Entity>)> handler);
    
  };
}
