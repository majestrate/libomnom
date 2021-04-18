#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace omnom
{
  using CryptoHash = std::array<uint8_t, 32>;
}

namespace std
{
  template<>
  struct hash<omnom::CryptoHash>
  {
    size_t
    operator()(const omnom::CryptoHash & h)
    {
      return std::hash<std::string_view>{}(std::string_view{reinterpret_cast<const char*>(h.data()), h.size()});
    }
  };
}
