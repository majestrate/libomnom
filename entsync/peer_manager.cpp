#include "peer_manager.hpp"
#include "context.hpp"
#include "logger.hpp"
#include <sodium/randombytes.h>

namespace entsync
{
  using namespace std::literals;

  PeerInfo::PeerInfo(lokimq::bt_value val)
  {
    if(const auto dict = std::get_if<lokimq::bt_dict>(&val))
    {
      if(const auto uid_str = std::get_if<std::string>(&dict->at("uid")))
      {
        if(uid_str->size() != uid.size())
          throw std::invalid_argument{"uid invalid size: "+ std::to_string(uid_str->size())};
        std::copy_n(uid_str->data(), uid.size(), uid.data());
      }
      else
        throw std::invalid_argument{"PeerInfo has no uid"};
      if(const auto addr_list = std::get_if<lokimq::bt_list>(&dict->at("addrs")))
      {
        for(const auto & item : *addr_list)
        {
          addrs.emplace(PeerAddr{item});
        }
      }
      else
        throw std::invalid_argument{"PeerInfo has no addrs list"};
    }
    else
      throw std::invalid_argument{"PeerInfo bt_value not a dict"};
  }
  
  lokimq::bt_value
  PeerInfo::to_bt_value() const
  {
    lokimq::bt_list bt_addrs;
    for(const auto & addr : addrs)
    {
      bt_addrs.push_back(addr.to_bt_value());
    }
    lokimq::bt_dict value{
      {"uid", std::string{reinterpret_cast<const char*>(uid.data()), uid.size()}},
      {"addrs", bt_addrs}
    };
    return value;
  }


  PeerAddr::PeerAddr(lokimq::bt_value val)
  {
    if(const auto dict = std::get_if<lokimq::bt_dict>(&val))
    {
      if(const auto addr_str = std::get_if<std::string>(&dict->at("addr")))
      {
        addr = *addr_str;
      }
      else
        throw std::invalid_argument{"PeerAddr addr is not a string"};
      if(const auto rank_int = std::get_if<uint64_t>(&dict->at("rank")))
      {
        rank = *rank_int;
      }
      else
        throw std::invalid_argument("PeerAddr rank is not an unsigned int");
    }
    else
      throw std::invalid_argument{"PeerAddr is not a dict"};
    
  }
  
  lokimq::bt_value
  PeerAddr::to_bt_value() const
  {
    lokimq::bt_dict value{
      {"addr", addr},
      {"rank", rank}
    };
    return value;
  }
  
  PeerManager::PeerManager(Context * ctx) :
    _ctx{ctx},
    m_Logic{lokimq::LokiMQ::run_in_proxy}
  {
  }

  void
  PeerManager::Start()
  {
    randombytes(m_OurInfo.uid.data(), m_OurInfo.uid.size());
    m_Logic = _ctx->lmq().add_tagged_thread("peer-manager-logic");
    _ctx->AddRequestHandler(
      "register_conn",
      [&](lokimq::Message & msg)
      {
        CallSafe([&]() {
          try
          {
            HandleRegisterConn(msg);
          }
          catch(std::exception & ex)
          {
            LogInfo(_ctx, "failed to handle register_conn message: ", ex.what());
            _ctx->lmq().disconnect(msg.conn);
          }
        });
      });
    _ctx->lmq().add_timer([&]() { Tick(); }, 100ms, true, m_Logic);
  }
  
  void
  PeerManager::AddReachableAddr(PeerAddr addr)
  {
    m_OurInfo.addrs.insert(std::move(addr));
  }
  
  bool
  PeerLimiter::ShouldTryConnecting(std::string addr)
  {
    const auto itr = m_ConnectBackoff.find(addr);
    if(itr == m_ConnectBackoff.end())
      return m_Connections[addr] == 0;
    return itr->second.first + itr->second.second <= time::Now();
  }


  bool
  PeerLimiter::ShouldLimit(std::string addr)
  {
    constexpr auto ConnectionPerAddressLimit = 1;
    return m_Connections[addr] > ConnectionPerAddressLimit;
  }
  
  void
  PeerManager::HandleRegisterConn(lokimq::Message & msg)
  {
    PeerInfo info{lokimq::bt_get(msg.data.at(0))};
    if(HasConnectionToPeer(info))
    {
      throw too_many_connections{"we already have a connection to this node"};
    }
    RegisterPeer(msg.conn, std::move(info));
    msg.send_reply(lokimq::bt_serialize(m_OurInfo.to_bt_value()));
  }

  void
  PeerManager::RegisterPeer(lokimq::ConnectionID conn, PeerInfo info)
  {
    for(const auto & peeraddr : info.addrs)
    {
      m_Limiter.MarkConnectSuccess(peeraddr.addr);
    }
    PeerState & state = m_Peers[conn];
    state.peerInfo = std::move(info);
    state.lastKeepAlive = time::Now();
    LogInfo(_ctx, "peer registered on ", conn);
  }
  

  bool
  PeerManager::HasConnectionToPeer(PeerInfo info) const
  {
    for(const auto & [_, peer] : m_Peers)
    {
      if(peer.peerInfo.uid == info.uid)
        return true;
    }
    return false;
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
    m_Connections[addr] += 1;
  }
  
  void
  PeerManager::OnNewOutboundPeer(lokimq::ConnectionID id, lokimq::address addr)
  {
    m_Limiter.MarkConnectSuccess(addr.zmq_address());
    m_Peers[id].outbound = true;
    /// send a register_conn command to tell the remote peer about our node
    _ctx->Request(
      id,
      "register_conn",
      [&, id=id, addr=addr.zmq_address()](bool success, std::vector<std::string> data)
      {
        PeerInfo info;
        try
        {
          info = PeerInfo{lokimq::bt_get(data.at(0))};
        }
        catch(std::exception & ex)
        {
          LogInfo(_ctx, "failed to parse PeerInfo: ", ex.what());
          success = false;
        }
        CallSafe([&, success, info, id, addr]() {
          if(not success)
          {
            m_Peers.erase(id);
            m_Limiter.MarkConnectFail(addr);
            _ctx->lmq().disconnect(id);
            return;
          }
          RegisterPeer(id, info);
        });
      }, lokimq::bt_serialize(m_OurInfo.to_bt_value()));
  }

  void
  PeerManager::AddPersistingPeer(lokimq::address peerAddr)
  {
    std::promise<void> wait;
    CallSafe([&, peerAddr]() {
      m_OutboundPeerAttempts.emplace_back(peerAddr, true);
      wait.set_value();
    });
    wait.get_future().get();
  }

  void
  PeerManager::Tick()
  {
    for(const auto & item : m_OutboundPeerAttempts)
    {
      const auto & peerAddr = item.first;
      const auto & persist = item.second;
      if(not m_Limiter.ShouldTryConnecting(peerAddr.zmq_address()))
        continue;
      _ctx->lmq().connect_remote(
        peerAddr,
        [&, addr=peerAddr, persist](lokimq::ConnectionID conn)
        {
          LogInfo(_ctx, "connected to ", addr, " via ", conn);
          CallSafe([&, conn]() {
            OnNewOutboundPeer(conn, addr);
            m_Peers[conn].persist = persist;
          });
        },
        [&, addr=peerAddr] (lokimq::ConnectionID, std::string_view reason)
        {
          LogInfo(_ctx, "did not connect to ", addr, ": ", reason);
          CallSafe([&, addr]() {
            m_Limiter.MarkConnectFail(addr.zmq_address());
          });
        });
    }
  }

  void
  PeerManager::CallSafe(std::function<void()> call)
  {
    _ctx->lmq().job(std::move(call), m_Logic);
  }
  
}
