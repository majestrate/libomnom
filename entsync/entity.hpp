#pragma once
#include <string>
#include <variant>

#include "crypto_hash.hpp"

#include <lokimq/bt_value.h>

namespace entsync
{


  /// an entity's kind, a unique identifier for this distinct entity type
  struct EntityKind
  {
    
    EntityKind() = default;
    EntityKind(lokimq::bt_value val);
    
    /// the name of this entity, must be distinct
    std::string name;
    /// if set to true then this entity is not meant to be stored
    bool ephemeral;

    bool operator==(const EntityKind & other) const { return name == other.name; };
    bool operator!=(const EntityKind & other) const { return not(*this == other); }

    lokimq::bt_value to_bt_value() const;
    
  };


  /// a unique identifier for an entity 
  struct EntityID
  {
    
    EntityID() = default;
    EntityID(lokimq::bt_value val);
    
    /// either a blockchain height or a hash value
    std::variant<uint64_t, CryptoHash> ID;

    bool operator==(const EntityID & other) const { return ID == other.ID; }
    bool operator!=(const EntityID & other) const { return not(*this == other); }

    lokimq::bt_value to_bt_value() const;
    
  };


  /// a value that can be gossiped accross the network
  struct Entity
  {

    Entity() = default;
    Entity(lokimq::bt_value val);

    EntityKind Kind;
    EntityID ID;
    lokimq::bt_value Data;


    bool operator==(const Entity & other) const
    {
      return ID == other.ID and Kind == other.Kind and Data == other.Data;
    }
    
    bool operator!=(const Entity & other) const { return not(*this == other); }

    lokimq::bt_value to_bt_value() const;
    
  };
  
}

namespace std
{
  template<>
  struct hash<entsync::EntityKind>
  {
    size_t
    operator()(const entsync::EntityKind & kind) const
    {
      return std::hash<std::string>{}(kind.name);
    }
  };

  template<>
  struct hash<entsync::EntityID>
  {
    size_t
    operator()(const entsync::EntityID & id) const
    {
      return std::hash<decltype(id.ID)>{}(id.ID);
    }
  };
}
