#include "peer_manager.hpp"
#include "context.hpp"
#include "logger.hpp"
#include <sodium/randombytes.h>

#include "oxenmq/hex.h"

namespace omnom
{
  using namespace std::literals;

  constexpr auto NACK = "NACK";
  constexpr auto OK = "OKAY";


  PeerInfo::PeerInfo(oxenmq::bt_value val)
  {
    if(const auto dict = std::get_if<oxenmq::bt_dict>(&val))
    {
      if(const auto uid_str = std::get_if<std::string>(&dict->at("uid")))
      {
        if(uid_str->size() != uid.size())
          throw std::invalid_argument{"uid invalid size: "+ std::to_string(uid_str->size())};
        std::copy_n(uid_str->data(), uid.size(), uid.data());
      }
      else
        throw std::invalid_argument{"PeerInfo has no uid"};
      if(const auto addr_list = std::get_if<oxenmq::bt_list>(&dict->at("addrs")))
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

  oxenmq::bt_value
  PeerInfo::to_bt_value() const
  {
    oxenmq::bt_list bt_addrs;
    for(const auto & addr : addrs)
    {
      bt_addrs.push_back(addr.to_bt_value());
    }
    oxenmq::bt_dict value{
      {"uid", std::string{reinterpret_cast<const char*>(uid.data()), uid.size()}},
      {"addrs", bt_addrs}
    };
    return value;
  }


  PeerAddr::PeerAddr(oxenmq::bt_value val)
  {
    if(const auto dict = std::get_if<oxenmq::bt_dict>(&val))
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

  oxenmq::bt_value
  PeerAddr::to_bt_value() const
  {
    oxenmq::bt_dict value{
      {"addr", addr},
      {"rank", rank}
    };
    return value;
  }


  std::string
  PeerInfo::ToString() const
  {
    return "PeerInfo: " + oxenmq::to_hex(uid.begin(), uid.end());
  }


  PeerManager::PeerManager(Context * ctx) :
    _ctx{ctx}
  {
  }

  void
  PeerManager::Start()
  {
    m_Logic = _ctx->lmq().add_tagged_thread("pm-logic");
    randombytes(m_OurInfo.uid.data(), m_OurInfo.uid.size());
    _ctx->AddRequestHandler(
      "register_conn",
      [&](oxenmq::Message & msg) { HandleRegisterConn(msg); });

    _ctx->AddRequestHandler(
      "list_peers",
      [&](oxenmq::Message & msg) { HandleListPeers(msg); });

    _ctx->lmq().add_timer([&]() { Tick(); }, 100ms, true, *m_Logic);
  }

  void
  PeerManager::AddReachableAddr(PeerAddr addr)
  {
    m_OurInfo.addrs.insert(std::move(addr));
  }

  void
  PeerManager::HandleListPeers(oxenmq::Message & msg)
  {
    std::vector<oxenmq::bt_value> peers{};
    CallSafe([&]() {
      for(const auto & info : m_Peers)
      {
        peers.emplace_back(info.second.peerInfo.to_bt_value());
      }
    });
    msg.send_reply(OK, oxenmq::bt_serialize(peers));
  }

  void
  PeerManager::HandleRegisterConn(oxenmq::Message & msg)
  {
    PeerInfo info;
    try
    {
      info = PeerInfo{oxenmq::bt_get(msg.data.at(0))};
    }
    catch(std::exception & ex)
    {
      LogInfo(_ctx, "failed to parse peer info ", ex.what());
      msg.send_reply(NACK, ex.what());
      return;
    }
    std::promise<std::optional<oxenmq::bt_value>> reply;
    CallSafe([&, info=std::move(info), conn=msg.conn]() {
      try
      {
        if(HasConnectionToPeer(info))
        {
          reply.set_value(std::nullopt);
        }
        RegisterPeer(conn, std::move(info));
        reply.set_value(std::optional<oxenmq::bt_value>{m_OurInfo.to_bt_value()});
      }
      catch(std::exception & ex)
      {
        LogInfo(_ctx, "failed to handle register_conn message: ", ex.what());
        reply.set_exception(std::current_exception());
      }
    });
    try
    {
      auto ftr =  reply.get_future();
      const auto maybe = ftr.get();
      if(maybe.has_value())
      {
        msg.send_reply(OK, oxenmq::bt_serialize(*maybe));
      }
      else
      {
        msg.send_reply(NACK, "rejected");
      }
    }
    catch(std::exception & ex)
    {
      msg.send_reply(NACK, ex.what());
    }
  }

  void
  PeerManager::RegisterPeer(oxenmq::ConnectionID conn, PeerInfo info)
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

  bool
  PeerManager::HasConnectionByAddress(std::string addr) const
  {
    for(const auto & [_, peer] : m_Peers)
    {
      for(const auto & peerAddr :peer.peerInfo.addrs)
      {
        if(addr == peerAddr.addr)
          return true;
      }
    }
    return false;

  }

  void
  PeerManager::OnNewOutboundPeer(oxenmq::ConnectionID id, oxenmq::address addr)
  {
    // send a register_conn command to tell the remote peer about our node
    _ctx->Request(
      id,
      "register_conn",
      [&, id=id, addr=addr.zmq_address()](bool success, std::vector<std::string> data)
      {
        auto KillConnection = [&, id, addr]()
        {
          m_Peers.erase(id);
          m_Limiter.MarkConnectFail(addr);
          _ctx->lmq().disconnect(id);
        };
        PeerInfo info;
        if(success)
        {
          if(data.at(0) == NACK)
          {
            CallSafe([&, KillConnection, reason=data.at(1)]()
            {
              LogInfo(_ctx, "remote peer rejected connection: ", reason);
              KillConnection();
            });
            return;
          }
          try
          {
            info = PeerInfo{oxenmq::bt_get(data.at(1))};
          }
          catch(std::exception & ex)
          {
            LogInfo(_ctx, "failed to parse PeerInfo: ", ex.what());
            success = false;
          }
        }
        CallSafe([&, success, info, id, addr, KillConnection]()
        {
          try
          {
            if(not success)
              throw std::runtime_error{"failed to send message"};
            RegisterPeer(id, info);
            m_Limiter.MarkConnectSuccess(addr);
          }
          catch(std::exception & ex)
          {
            LogInfo(_ctx, "Failed to register connection: ", ex.what(), " addr=", addr);
            KillConnection();
          }
        });
      }, oxenmq::bt_serialize(m_OurInfo.to_bt_value()));
  }

  void
  PeerManager::AddPersistingPeer(oxenmq::address peerAddr)
  {
    std::promise<void> wait;
    CallSafe([&, peerAddr]() {
      m_OutboundPeerAttempts.emplace_back(peerAddr, true);
      wait.set_value();
    });
    wait.get_future().get();
  }

  std::vector<std::string>
  PeerManager::GetPeerAddresses() const
  {
    std::vector<std::string> addrs;
    std::promise<void> wait;
    CallSafe([&]() {
      for(const auto & [_, peer] : m_Peers)
      {
        for(const auto & addr : peer.peerInfo.addrs)
          addrs.emplace_back(addr.addr);
      }
      wait.set_value();
    });
    wait.get_future().get();
    return addrs;
  }


  void
  PeerManager::Tick()
  {
    for(const auto & item : m_OutboundPeerAttempts)
    {
      const auto addr = item.first;
      const auto persist = item.second;
      if(HasConnectionByAddress(addr.zmq_address()))
        continue;
      if(not m_Limiter.ShouldTryConnecting(addr.zmq_address()))
        continue;
      _ctx->lmq().connect_remote(
        addr,
        [=](oxenmq::ConnectionID conn)
        {
          LogInfo(_ctx, "connected to ", addr, " via ", conn);
          CallSafe([=]() {
            OnNewOutboundPeer(conn, addr);
            m_Peers[conn].persist = persist;
          });
        },
        [=] (oxenmq::ConnectionID, std::string_view reason)
        {
          LogInfo(_ctx, "did not connect to ", addr, ": ", reason);
          CallSafe([=]() {
            m_Limiter.MarkConnectFail(addr.zmq_address());
          });
        });
    }
  }

  void
  PeerManager::VisitPeerStateForConnection(oxenmq::ConnectionID id, std::function<void(std::optional<PeerState>)> visit)
  {
    CallSafe(
      [=]()
      {
        auto itr = m_Peers.find(id);
        if(itr == m_Peers.end())
        {
          visit(std::nullopt);
        }
        else
        {
          visit(itr->second);
        }
      });
  }


  void
  PeerManager::CallSafe(std::function<void()> call) const
  {
    _ctx->lmq().job(std::move(call), *m_Logic);
  }

}
