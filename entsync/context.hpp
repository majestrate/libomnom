#pragma once

#include <oxenmq/oxenmq.h>
#include "crypto_hash.hpp"
#include "entity.hpp"
#include "gossip.hpp"
#include "handler.hpp"
#include "searcher.hpp"
#include "peer_manager.hpp"

#include <future>
#include <optional>
#include <variant>
#include "storage.hpp"

namespace entsync
{
  /// An Entity Sync Context contains all members and components needed to 
  /// synchronize entities to other instances of the context regardless of if it is across network or ipc pipes
  class Context
  {
    const std::string m_Dialect;
  protected:
    oxenmq::OxenMQ & m_LMQ;
    
    EntitySearcher m_Search;
    EntityHandler m_Handler;
    PeerManager m_Peers;
    Gossiper m_Gossip;
    
  public:
    explicit Context(oxenmq::OxenMQ & lmq, std::string dialect);
    /// non copyable
    Context(const Context &) = delete;
    /// non movable
    Context(Context &&) = delete;

    Gossiper &
    Gossip() { return m_Gossip; };

    EntitySearcher &
    Search() { return m_Search; };
    
    /// add a listener to oxenmq on lmqAddr to accept inbound connections
    /// advertise a set of reachable endpoints that it will publish if applicable in order of rank
    void
    Listen(std::string lmqAddr, std::set<PeerAddr> adverts);

    void
    AddPersistingPeer(oxenmq::address peerAddr);

    /// start the internal components
    void
    Start();

    /// maximum number of allowed peers
    int maxPeers = -1;

    oxenmq::OxenMQ & lmq() { return m_LMQ; };

    template<typename ...T>
    void
    Request(oxenmq::ConnectionID id, std::string_view method, oxenmq::OxenMQ::ReplyCallback callback, const T& ... opts)
    {
      const std::string call = m_Dialect + "." + std::string{method};
      m_LMQ.request(std::move(id), call, std::move(callback), std::forward<const T&>(opts)...);
    }

    template<typename ...T>
    void
    Send(oxenmq::ConnectionID id, std::string_view method, const T& ... opts)
    {
      const std::string call = m_Dialect + "." + std::string{method};
      m_LMQ.send(std::move(id), call, std::forward<const T&>(opts)...);
    }

    void
    AddRequestHandler(std::string method, oxenmq::OxenMQ::CommandCallback handler);

    void
    AddCommandHandler(std::string method, oxenmq::OxenMQ::CommandCallback handler);

    
  };
}
