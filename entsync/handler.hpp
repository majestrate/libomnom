#pragma once
#include "crypto_hash.hpp"
#include "entity.hpp"
#include <lokimq/lokimq.h>
#include <functional>
#include <unordered_map>
#include <variant>

namespace entsync
{
  class Context;
  
  class EntityHandler
  {
    Context * const _ctx;
    std::unordered_map<EntityKind, std::function<void(std::variant<uint64_t, CryptoHash>, lokimq::bt_value)>> m_Handlers;
  public:
    explicit EntityHandler(Context * ctx);
    
    EntityHandler(const EntityHandler &) = delete;
    EntityHandler(EntityHandler &&) = delete;
   
  };

}
