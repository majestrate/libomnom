#include "context.hpp"


namespace omnom
{

  Context::Context(oxenmq::OxenMQ & lmq, std::string dialect) :
    m_Dialect{std::move(dialect)},
    m_LMQ{lmq},
    m_Search{this},
    m_Handler{this},
    m_Peers{this},
    m_Gossip{&m_Peers}
  {
  }

  void
  Context::Start()
  {
    m_LMQ.add_category(m_Dialect, oxenmq::Access{}, 1, 1024);
    m_Peers.Start();
    m_Gossip.Start();
  }
  
  void
  Context::Listen(std::string lmqAddr, std::set<PeerAddr> adverts)
  {
    m_LMQ.listen_plain(lmqAddr);
    for(auto & addr : adverts)
      m_Peers.AddReachableAddr(std::move(addr));
  }

  void
  Context::AddPersistingPeer(oxenmq::address peerAddr)
  {
    m_Peers.AddPersistingPeer(std::move(peerAddr));
  }

  void
  Context::AddRequestHandler(std::string method, oxenmq::OxenMQ::CommandCallback handler)
  {
    m_LMQ.add_request_command(m_Dialect, std::move(method), std::move(handler));
  }

  void
  Context::AddCommandHandler(std::string method, oxenmq::OxenMQ::CommandCallback handler)
  {
    m_LMQ.add_command(m_Dialect, std::move(method), std::move(handler));
  }  
}
