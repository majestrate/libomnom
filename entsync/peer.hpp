#pragma once
#include "crypto_hash.hpp"
#include "time.hpp"
#include <lokimq/bt_serialize.h>
#include <string>
#include <set>

namespace entsync
{
  
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

}
