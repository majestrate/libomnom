#include "context.hpp"


namespace entsync
{

  Context::Context(lokimq::LokiMQ & lmq, std::string dialect) :
    m_Dialect{std::move(dialect)},
    m_LMQ{lmq},
    m_Search{this},
    m_Handler{this},
    m_Peers{this}
  {
  }

  void
  Context::Start()
  {
    m_LMQ.add_category(m_Dialect, lokimq::Access{}, 1, 1024);
    m_Peers.Start();
  }
  
  void
  Context::Listen(std::string lmqAddr, std::set<PeerAddr> adverts)
  {
    m_LMQ.listen_plain(lmqAddr);
    for(auto & addr : adverts)
      m_Peers.AddReachableAddr(std::move(addr));
  }

  void
  Context::AddPersistingPeer(lokimq::address peerAddr)
  {
    m_Peers.AddPersistingPeer(std::move(peerAddr));
  }

  void
  Context::AddRequestHandler(std::string method, lokimq::LokiMQ::CommandCallback handler)
  {
    m_LMQ.add_request_command(m_Dialect, std::move(method), std::move(handler));
  }
}
