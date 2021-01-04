#include "entity.hpp"
#include <algorithm>
#include <stdexcept>

namespace entsync
{
  EntityKind::EntityKind(lokimq::bt_value val)
  {
    if(auto list = std::get_if<lokimq::bt_list>(&val))
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

  EntityID::EntityID(lokimq::bt_value val)
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
  
  Entity::Entity(lokimq::bt_value val)
  {
    if(auto list = std::get_if<lokimq::bt_list>(&val))
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
}
