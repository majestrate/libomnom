#include "gossip.hpp"
#include "peer_manager.hpp"
#include "context.hpp"

namespace entsync
{
  Gossiper::Gossiper(PeerManager * peerManager) : _peerManager{peerManager}
  {
  }

  void
  Gossiper::Start()
  {
    auto ctx = _peerManager->GetContext();
    m_Logic = ctx->lmq().add_tagged_thread("gossip-logic");
    ctx->AddCommandHandler("gossip", [&](lokimq::Message & msg)
    {
      Entity ent{lokimq::bt_get(msg.data[0])};
      HandleGossip(msg.conn, ent);
    });
  }

  void
  Gossiper::CallSafe(std::function<void(void)> call) const
  {
    _peerManager->GetContext()->lmq().job(std::move(call), *m_Logic);
  }

  void
  Gossiper::HandleGossip(lokimq::ConnectionID id, Entity ent)
  {
    CallSafe([&, ent]() {
      auto itr = m_Storage.find(ent.Kind);
      if(itr == m_Storage.end() or not itr->second->HasEntity(ent))
      {
        const auto range = m_Handlers.equal_range(ent.Kind);
        auto item = range.first;
        while(item != range.second)
        {
          item->second(ent);
          item++;
        }
      } 
    });
    Broadcast(std::move(ent), [id](lokimq::ConnectionID other) { return other != id; });
  }
  
  void
  Gossiper::Broadcast(Entity ent, std::function<bool(lokimq::ConnectionID)> filter)
  {
    _peerManager->CallSafe([=]() {
      _peerManager->ForEachPeer([ent=lokimq::bt_serialize(ent.to_bt_value()), ctx=_peerManager->GetContext(), filter](lokimq::ConnectionID id, PeerState )
      {
        if(filter and not filter(id))
          return;
        ctx->Send(id, "gossip", ent);
      });
    });
  }
}
