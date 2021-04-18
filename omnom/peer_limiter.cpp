#include "peer_limiter.hpp"
namespace omnom
{
  using namespace std::literals;

  bool
  PeerLimiter::ShouldTryConnecting(std::string addr)
  {
    const auto itr = m_ConnectBackoff.find(addr);
    if(itr == m_ConnectBackoff.end())
      return true;
    return itr->second.first + itr->second.second >= time::Now();
  }


  bool
  PeerLimiter::ShouldLimit(std::string addr)
  {
    constexpr auto ConnectionPerAddressLimit = 1;
    return m_Connections[addr] > ConnectionPerAddressLimit;
  }
  

  void
  PeerLimiter::MarkConnectFail(std::string addr)
  {
    constexpr auto InitialBackoff = 100ms;
    const auto itr = m_ConnectBackoff.find(addr);
    if(itr == m_ConnectBackoff.end())
    {
      m_ConnectBackoff[addr] = std::make_pair(time::Now(), InitialBackoff);
    }
    else
    {
      /// do expontential backoff
      itr->second.first = time::Now();
      itr->second.second *= 1.25;
    }
  }


  void
  PeerLimiter::MarkConnectSuccess(std::string addr)
  {
    m_ConnectBackoff.erase(addr);
  }
}
