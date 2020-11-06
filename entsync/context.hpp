#pragma once

#include <lokimq/lokimq.h>
#include "crypto_hash.hpp"
#include "entity.hpp"
#include "gossip.hpp"
#include "handler.hpp"
#include "searcher.hpp"
#include "peer_manager.hpp"

#include <future>
#include <optional>
#include <variant>

namespace entsync
{
  /// An Entity Sync Context contains all members and components needed to 
  /// synchronize entities to other instances of the context regardless of if it is across network or ipc pipes
  class Context
  {
    const std::string m_Dialect;
  protected:
    lokimq::LokiMQ & m_LMQ;
    
    EntitySearcher m_Search;
    EntityHandler m_Handler;
    PeerManager m_Peers;
        
  public:
    explicit Context(lokimq::LokiMQ & lmq, std::string dialect);
    /// non copyable
    Context(const Context &) = delete;
    /// non movable
    Context(Context &&) = delete;

    /// add a listener to lokimq on lmqAddr to accept inbound connections
    /// advertise a set of reachable endpoints that it will publish if applicable in order of rank
    void
    Listen(std::string lmqAddr, std::set<PeerAddr> adverts);

    void
    AddPersistingPeer(lokimq::address peerAddr);

    /// start the internal components
    void
    Start();
    

    /// maximum number of allowed peers
    int maxPeers = -1;

    lokimq::LokiMQ & lmq() { return m_LMQ; };

    template<typename ...T>
    void
    Request(lokimq::ConnectionID id, std::string_view method, lokimq::LokiMQ::ReplyCallback callback, const T& ... opts)
    {
      const std::string call = m_Dialect + "." + std::string{method};
      m_LMQ.request(std::move(id), call, std::move(callback), opts...);
    }

    void
    AddRequestHandler(std::string method, lokimq::LokiMQ::CommandCallback handler);

    
  };
}
