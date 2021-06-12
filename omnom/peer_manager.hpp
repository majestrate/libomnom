#pragma once

#include "crypto_hash.hpp"
#include "peer.hpp"
#include "peer_limiter.hpp"
#include "time.hpp"

#include <oxenmq/oxenmq.h>

#include <unordered_map>

namespace omnom
{
    class Context;

    /// exception thrown if we have too many connections
    class too_many_connections : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    class PeerManager
    {
        Context* const _ctx;
        std::optional<oxenmq::TaggedThreadID> m_Logic;
        PeerInfo m_OurInfo;
        std::unordered_map<oxenmq::ConnectionID, PeerState> m_Peers;
        PeerLimiter m_Limiter;

        std::vector<std::pair<oxenmq::address, bool>> m_OutboundPeerAttempts;

        void
        OnNewOutboundPeer(oxenmq::ConnectionID conn, oxenmq::address addr);

        void
        HandleRegisterConn(oxenmq::Message& msg);

        void
        HandleListPeers(oxenmq::Message& msg);

        void
        Tick();

        void
        RegisterPeer(oxenmq::ConnectionID conn, PeerInfo info);

        bool
        HasConnectionToPeer(PeerInfo info) const;

        bool
        HasConnectionByAddress(std::string addr) const;

       public:
        explicit PeerManager(Context* ctx);
        PeerManager(const PeerManager&) = delete;
        PeerManager(PeerManager&&) = delete;

        void
        CallSafe(std::function<void()> f) const;

        /// setup and start peer manager
        void
        Start();

        /// add an advertised address to our peer info
        void
        AddReachableAddr(PeerAddr addr);

        /// add a peer to bootstrap from
        void
        AddBootstrapPeer(oxenmq::address addr);

        /// add a forever connected peer
        void
        AddPersistingPeer(oxenmq::address addr);

        std::vector<std::string>
        GetPeerAddresses() const;

        void
        VisitPeerStateForConnection(oxenmq::ConnectionID id, std::function<void(std::optional<PeerState>)> visit);

        template <typename Visit>
        void
        ForEachPeer(Visit visit)
        {
            for (const auto& [id, st] : m_Peers)
                visit(id, st);
        }

        Context*
        GetContext()
        {
            return _ctx;
        };
    };
}  // namespace omnom
