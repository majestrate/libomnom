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
    ctx->AddCommandHandler("gossip", [=](oxenmq::Message & msg)
    {
      Entity ent{oxenmq::bt_get(msg.data[0])};
      HandleGossip(msg.conn, ent);
    });
  }

  void
  Gossiper::CallSafe(std::function<void(void)> call) const
  {
    _peerManager->GetContext()->lmq().job(std::move(call), *m_Logic);
  }

  void
  Gossiper::HandleGossip(oxenmq::ConnectionID id, Entity ent)
  {
    CallSafe(
      [=]()
      {
        auto visit =
          [=](std::optional<PeerState> state)
          {
            // inform handlers
            if(state == std::nullopt)
            {
              return;
            }
            CallSafe(
              [=, peerState=*state]()
              {
                const auto range = m_Handlers.equal_range(ent.Kind);
                auto itr = range.first;
                while(itr != range.second)
                {
                  itr->second(peerState, ent);
                  itr++;
                }
              });
          };
        _peerManager->VisitPeerStateForConnection(id, visit);
      });
  }

  void
  Gossiper::Broadcast(Entity ent, std::function<bool(oxenmq::ConnectionID, const PeerState &)> filter)
  {
    std::promise<void> promise;
    _peerManager->CallSafe([=, &promise]() {
      _peerManager->ForEachPeer([ent=oxenmq::bt_serialize(ent.to_bt_value()), ctx=_peerManager->GetContext(), filter](oxenmq::ConnectionID id, PeerState state)
      {
        if(filter and not filter(id, state))
          return;
        ctx->Send(id, "gossip", ent);
      });
      promise.set_value();
    });
    promise.get_future().wait();
  }

  void
  Gossiper::SetEntityStorage(EntityKind kind, EntityStorage & storage)
  {
    m_Storage.emplace(kind, storage);
  }


  void
  Gossiper::AddEntityHandler(EntityKind kind, std::function<void(const PeerState&, Entity)> handler)
  {
    m_Handlers.emplace(kind, handler);
  }

}
