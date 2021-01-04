#pragma once

#include "entity.hpp"
#include "storage.hpp"
#include <memory>
#include <functional>
#include <unordered_map>

namespace entsync
{

  class PeerManager;
  
  class Gossiper
  {
    PeerManager * const _peerManager;

    std::unordered_multimap<EntityKind, std::function<void(Entity)>> m_Handlers;

    std::unordered_map<EntityKind, std::unique_ptr<EntityStorage>> m_Storage;
    
  public:
    explicit Gossiper(PeerManager * peerManager);
    Gossiper(const Gossiper &) = delete;
    Gossiper(Gossiper &&) = delete;

    /// broadcast an entity to the network
    void
    Broadcast(Entity ent);

    /// add a handler to handle new gossiped entities
    void
    AddEntityHandler(EntityKind kind, std::function<void(Entity)> handler);

    /// set persistent storage for this kind of entity
    /// throws std::invalid_argument if entity's kind is ephemeral
    /// replaces any existing storage
    void
    SetEntityStorage(EntityKind kind, std::unique_ptr<EntityStorage> storage);
    
  };
  
}
