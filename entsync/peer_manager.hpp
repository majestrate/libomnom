#pragma once


#include "crypto_hash.hpp"

#include <lokimq/lokimq.h>

#include <chrono>
#include <set>
#include <unordered_map>

namespace entsync
{

  class Context;
  
  namespace time
  {
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    constexpr auto Now = Clock::now;
  }

  /// a single reachable address on a peer
  struct PeerAddr
  {
    PeerAddr() = default;
    PeerAddr(lokimq::bt_value val);
    
    /// the advertised address in the form: protocol://address
    std::string addr;
    /// lower rank means we try it first
    uint64_t rank;

    bool operator < (const PeerAddr & other) const { return rank < other.rank;  };


    lokimq::bt_value
    to_bt_value() const;
    
  };
  
  /// information about a peer on the network
  struct PeerInfo
  {

    PeerInfo() = default;
    PeerInfo(lokimq::bt_value val);
    
    /// all reachable addresses if it has any
    std::set<PeerAddr> addrs;
    /// a unique id used to identify this peer
    /// picked at random
    CryptoHash uid;

    lokimq::bt_value
    to_bt_value() const;
  };

  /// state about a peer
  struct PeerState
  {
    /// persist this connection forever
    bool persist = false;
    time::TimePoint lastKeepAlive;
    PeerInfo peerInfo;
    /// true if we initiated this connection false otherwise
    bool outbound;
  };


  /// exception thrown if we have too many connections
  class too_many_connections : public std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };
  

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
  
  class PeerManager
  {
    Context * const _ctx;
    lokimq::TaggedThreadID m_Logic;
    PeerInfo m_OurInfo;
    std::unordered_map<lokimq::ConnectionID, PeerState> m_Peers;
    PeerLimiter m_Limiter;
    
    std::vector<std::pair<lokimq::address, bool>> m_OutboundPeerAttempts;
    
    void
    OnNewOutboundPeer(lokimq::ConnectionID conn, lokimq::address addr);

    void
    HandleRegisterConn(lokimq::Message & msg);

    void
    Tick();

    void
    CallSafe(std::function<void()> f);

    void
    RegisterPeer(lokimq::ConnectionID conn, PeerInfo info);
    
  public:
    explicit PeerManager(Context * ctx);
    PeerManager(const PeerManager&) = delete;
    PeerManager(PeerManager &&) = delete;

    /// setup and start peer manager
    void
    Start();

    /// add an advertised address to our peer info
    void
    AddReachableAddr(PeerAddr addr);

    /// add a peer to bootstrap from
    void
    AddBootstrapPeer(lokimq::address addr);
    
    /// add a forever connected peer
    void
    AddPersistingPeer(lokimq::address addr);
    
  };
}
