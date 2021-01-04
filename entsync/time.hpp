#pragma once
#include <chrono>

namespace entsync::time
{
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  constexpr auto Now = Clock::now;
}
