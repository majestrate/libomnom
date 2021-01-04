#pragma once


#include "crypto_hash.hpp"
#include "peer.hpp"
#include "peer_limiter.hpp"
#include "time.hpp"

#include <lokimq/lokimq.h>

#include <unordered_map>

namespace entsync
{

  class Context;


  /// exception thrown if we have too many connections
  class too_many_connections : public std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };
  
  class PeerManager
  {
    Context * const _ctx;
    std::optional<lokimq::TaggedThreadID> m_Logic;
    PeerInfo m_OurInfo;
    std::unordered_map<lokimq::ConnectionID, PeerState> m_Peers;
    PeerLimiter m_Limiter;
    
    std::vector<std::pair<lokimq::address, bool>> m_OutboundPeerAttempts;
    
    void
    OnNewOutboundPeer(lokimq::ConnectionID conn, lokimq::address addr);

    void
    HandleRegisterConn(lokimq::Message & msg);

    void
    HandleListPeers(lokimq::Message & msg);
    
    void
    Tick();

    void
    CallSafe(std::function<void()> f) const;

    void
    RegisterPeer(lokimq::ConnectionID conn, PeerInfo info);


    bool
    HasConnectionToPeer(PeerInfo info) const;

    bool
    HasConnectionByAddress(std::string addr) const;
    
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

    std::vector<std::string>
    GetPeerAddresses() const;

    void
    ForEachPeer(std::function<void(PeerState peer, std::function<void(lokimq::bt_value)>sendMessage)> visit);
    
  };
}
