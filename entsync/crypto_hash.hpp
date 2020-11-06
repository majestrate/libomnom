#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace entsync
{
  using CryptoHash = std::array<uint8_t, 32>;
}

namespace std
{
  template<>
  struct hash<entsync::CryptoHash>
  {
    size_t
    operator()(const entsync::CryptoHash & h)
    {
      return std::hash<std::string_view>{}(std::string_view{reinterpret_cast<const char*>(h.data()), h.size()});
    }
  };
}
