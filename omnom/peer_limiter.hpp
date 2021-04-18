#pragma once
#include "time.hpp"
#include <string>
#include <unordered_map>

namespace omnom
{

  class PeerLimiter
  {
    /// maps connection address to last connect attempt and current backoff interval
    std::unordered_map<std::string, std::pair<time::TimePoint, std::chrono::milliseconds>> m_ConnectBackoff;
    /// map of address to number of connections we have of them
    std::unordered_map<std::string, int> m_Connections;
  public:

    void
    MarkConnectFail(std::string addr);

    /// should we try to connect to this peer?
    bool
    ShouldTryConnecting(std::string addr);

    void
    MarkConnectSuccess(std::string addr);

    /// should we rate limit this peer?
    bool
    ShouldLimit(std::string addr);

  };
  
}
