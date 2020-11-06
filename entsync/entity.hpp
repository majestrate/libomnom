#pragma once
#include <string>

namespace entsync
{


  /// an entity's kind, a unique identifier for this distinct entity type
  struct EntityKind
  {
    /// the name of this entity, must be distinct
    std::string name;
    /// if set to true then this entity is not meant to be stored
    bool ephemeral;

    bool operator==(const EntityKind & other) const { return name == other.name; };
    
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
}
