#include "entity.hpp"
#include <algorithm>
#include <stdexcept>

#include <oxenmq/hex.h>

namespace omnom
{
  EntityKind::EntityKind(std::string _name, bool _ephemeral) :
    name{std::move(_name)},
    ephemeral{_ephemeral}
  {}

  EntityKind::EntityKind(oxenmq::bt_value val)
  {
    if(auto list = std::get_if<oxenmq::bt_list>(&val))
    {
      if(list->size() != 2)
        throw std::invalid_argument{"entity kind bad size: " + std::to_string(list->size())};
      if(const auto str = std::get_if<std::string>(&list->front()))
      {
        name = *str;
      }
      else
        throw std::invalid_argument{"entity kind first element not a string"};
      list->pop_front();
      if(const auto integer = std::get_if<uint64_t>(&list->front()))
      {
        ephemeral = *integer == 0;
      }
      else
        throw std::invalid_argument{"entity kind second element not an integer"};

    }
    else
      throw std::invalid_argument{"entity kind not a list"};
  }

  oxenmq::bt_value
  EntityKind::to_bt_value() const
  {
    return oxenmq::bt_list{oxenmq::bt_value{name}, oxenmq::bt_value{ephemeral ? uint64_t{0} : uint64_t{1}}};
  }

  EntityID::EntityID(uint64_t index) : ID{index} {}

  EntityID::EntityID(oxenmq::bt_value val)
  {
    if(const auto integer = std::get_if<uint64_t>(&val))
    {
      ID = *integer;
    }
    else if(const auto str = std::get_if<std::string>(&val))
    {
      CryptoHash hash;
      if(str->size() != hash.size())
        throw std::invalid_argument{"entity id hash size "+ std::to_string(str->size()) + " != " + std::to_string(hash.size())};
      std::copy_n(reinterpret_cast<const uint8_t *>(str->data()), str->size(), hash.data());
      ID = std::move(hash);
    }
    else
      throw std::invalid_argument{"entity id is not an integer or string"};
  }

  oxenmq::bt_value
  EntityID::to_bt_value() const
  {
    if(const auto integer = std::get_if<uint64_t>(&ID))
    {
      return oxenmq::bt_value{*integer};
    }
    if(const auto hash = std::get_if<CryptoHash>(&ID))
    {
      return oxenmq::bt_value{std::string{reinterpret_cast<const char*>(hash->data()), hash->size()}};
    }
    throw std::logic_error{"entity id holds invalid data"};
  }

  std::string
  EntityID::ToString() const
  {
    if(const auto integer = std::get_if<uint64_t>(&ID))
    {
      return std::to_string(*integer);
    }
    if(const auto hash = std::get_if<CryptoHash>(&ID))
    {
      return oxenmq::to_hex(hash->begin(), hash->end());
    }
    throw std::logic_error{"entity id holds invalid data"};
  }


  Entity::Entity(oxenmq::bt_value val)
  {
    if(auto list = std::get_if<oxenmq::bt_list>(&val))
    {
      if(list->size() != 3)
        throw std::invalid_argument{"entity size missmatch: " + std::to_string(list->size()) + " != 3"};

      Kind = EntityKind{list->front()};
      list->pop_front();
      ID = EntityID{list->front()};
      list->pop_front();
      Data = std::move(list->front());

    }
    else
      throw std::invalid_argument{"entity is not a list"};
  }

  std::string
  Entity::ToString() const
  {
    return "Entity: kind="+Kind.name+" at "+ID.ToString();
  }

  oxenmq::bt_value
  Entity::to_bt_value() const
  {
    return oxenmq::bt_list{Kind.to_bt_value(), ID.to_bt_value(), Data};
  }
}
