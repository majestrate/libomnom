#pragma once
#include <string>
#include <variant>

#include "crypto_hash.hpp"

#include <oxenmq/bt_value.h>

namespace omnom
{


  /// an entity's kind, a unique identifier for this distinct entity type
  struct EntityKind
  {
    
    EntityKind() = default;
    EntityKind(oxenmq::bt_value val);
    explicit EntityKind(std::string name, bool ephemeral);
    
    /// the name of this entity, must be distinct
    std::string name;
    /// if set to true then this entity is not meant to be stored
    bool ephemeral;

    bool operator==(const EntityKind & other) const { return name == other.name; };
    bool operator!=(const EntityKind & other) const { return not(*this == other); }

    oxenmq::bt_value to_bt_value() const;
    
  };


  /// a unique identifier for an entity 
  struct EntityID
  {
    
    EntityID() = default;
    EntityID(oxenmq::bt_value val);
    explicit EntityID(uint64_t index);
    /// either a blockchain height or a hash value
    std::variant<uint64_t, CryptoHash> ID;

    bool operator==(const EntityID & other) const { return ID == other.ID; }
    bool operator!=(const EntityID & other) const { return not(*this == other); }

    oxenmq::bt_value to_bt_value() const;

    std::string ToString() const;
    
  };


  /// a value that can be gossiped accross the network
  struct Entity
  {

    Entity() = default;
    Entity(oxenmq::bt_value val);

    EntityKind Kind;
    EntityID ID;
    oxenmq::bt_value Data;


    bool operator==(const Entity & other) const
    {
      return ID == other.ID and Kind == other.Kind and Data == other.Data;
    }
    
    bool operator!=(const Entity & other) const { return not(*this == other); }

    oxenmq::bt_value to_bt_value() const;

    std::string ToString() const;
    
  };
  
}

namespace std
{
  template<>
  struct hash<omnom::EntityKind>
  {
    size_t
    operator()(const omnom::EntityKind & kind) const
    {
      return std::hash<decltype(kind.name)>{}(kind.name);
    }
  };

  template<>
  struct hash<omnom::EntityID>
  {
    size_t
    operator()(const omnom::EntityID & id) const
    {
      return std::hash<decltype(id.ID)>{}(id.ID);
    }
  };
}
