#pragma once
#include "crypto_hash.hpp"
#include "entity.hpp"
#include <lokimq/lokimq.h>
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
      std::unordered_map<Key_type, std::function<void(std::optional<lokimq::bt_value>)>> m_PendingSearches;
    };
      
    std::unordered_map<EntityKind, Bucket<uint64_t>>  m_EntitySearchesByInt;
    std::unordered_map<EntityKind, Bucket<CryptoHash>>  m_EntitySearchesByHash;
    
  public:
    explicit EntitySearcher(Context * ctx);
    
    EntitySearcher(const EntitySearcher &) = delete;
    EntitySearcher(EntitySearcher &&) = delete;

    void
    ObtainEntityByID(std::variant<uint64_t, CryptoHash> id, EntityKind kind, std::function<void(std::optional<lokimq::bt_value>)> handler);
    
  };
}
