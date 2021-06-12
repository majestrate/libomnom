#include "gossip.hpp"
#include "peer_manager.hpp"
#include "context.hpp"

namespace omnom
{
    Gossiper::Gossiper(PeerManager* peerManager) : _peerManager{peerManager}
    {}

    void
    Gossiper::Start()
    {
        auto ctx = _peerManager->GetContext();
        m_Logic = ctx->lmq().add_tagged_thread("gossip-logic");
        ctx->AddCommandHandler("gossip", [this](oxenmq::Message& msg) {
            Entity ent{oxenmq::bt_get(msg.data[0])};
            HandleGossip(msg.conn, std::move(ent));
        });
        ctx->AddRequestHandler("serve", [this](oxenmq::Message& msg) {
            if (msg.data.size() != 2)
            {
                /// TODO: down rank peer
                msg.send_reply("");
                return;
            }
            EntityKind kind{oxenmq::bt_get(msg.data[0])};
            EntityID id{oxenmq::bt_get(msg.data[1])};

            CallSafe([this, defer = msg.send_later(), kind, id]() {
                if (const auto maybe = HandleServe(kind, id))
                {
                    defer.reply(oxenmq::bt_serialize(maybe->to_bt_value()));
                }
                else  // no such entity
                    defer.reply("");
            });
        });
    }

    std::optional<Entity>
    Gossiper::HandleServe(EntityKind kind, EntityID id)
    {
        if (auto itr = m_Storage.find(kind); itr != m_Storage.end())
        {
            return itr->second.GetEntityByID(id);
        }
        else
            return std::nullopt;
    }

    void
    Gossiper::CallSafe(std::function<void(void)> call) const
    {
        _peerManager->GetContext()->lmq().job(std::move(call), *m_Logic);
    }

    void
    Gossiper::HandleGossip(oxenmq::ConnectionID id, Entity ent)
    {
        CallSafe([this, ent, id]() {
            auto visit = [this, ent](std::optional<PeerState> state) {
                // inform handlers
                if (state == std::nullopt)
                {
                    return;
                }
                CallSafe([this, ent, peerState = *state]() {
                    const auto range = m_Handlers.equal_range(ent.Kind);
                    auto itr = range.first;
                    while (itr != range.second)
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
    Gossiper::Broadcast(Entity ent, std::function<bool(oxenmq::ConnectionID, const PeerState&)> filter)
    {
        std::promise<void> promise;
        _peerManager->CallSafe([this, ent, filter, &promise]() {
            _peerManager->ForEachPeer([ent = oxenmq::bt_serialize(ent.to_bt_value()), ctx = _peerManager->GetContext(), filter](oxenmq::ConnectionID id, PeerState state) {
                if (filter and not filter(id, state))
                    return;
                ctx->Send(id, "gossip", ent);
            });
            promise.set_value();
        });
        promise.get_future().wait();
    }

    void
    Gossiper::SetEntityStorage(EntityKind kind, EntityStorage& storage)
    {
        m_Storage.emplace(kind, storage);
    }

    void
    Gossiper::AddEntityHandler(EntityKind kind, std::function<void(const PeerState&, Entity)> handler)
    {
        m_Handlers.emplace(kind, handler);
    }

}  // namespace omnom
