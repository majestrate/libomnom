#pragma once
#include "crypto_hash.hpp"
#include "time.hpp"
#include <oxenmq/bt_serialize.h>
#include <string>
#include <set>
#include <functional>

namespace omnom
{
    /// a single reachable address on a peer
    struct PeerAddr
    {
        PeerAddr() = default;
        PeerAddr(oxenmq::bt_value val);

        /// the advertised address in the form: protocol://address
        std::string addr;
        /// lower rank means we try it first
        uint64_t rank;

        bool
        operator<(const PeerAddr& other) const
        {
            return rank < other.rank;
        };

        oxenmq::bt_value
        to_bt_value() const;
    };

    /// information about a peer on the network
    struct PeerInfo
    {
        /// maybe create a PeerInfo by querying via dns for our lokinet address
        /// use a custom dnsEndpoint in the format "ip:port" if we dont want to use the system resolver
        static std::optional<PeerInfo>
        LokinetPeerInfo(std::optional<std::string> dnsEndpoint = std::nullopt);

        PeerInfo() = default;
        PeerInfo(oxenmq::bt_value val);

        /// all reachable addresses if it has any
        std::set<PeerAddr> addrs;
        /// a unique id used to identify this peer
        /// picked at random
        CryptoHash uid;

        oxenmq::bt_value
        to_bt_value() const;

        std::string
        ToString() const;
    };

    /// state about a peer
    struct PeerState
    {
        /// persist this connection forever
        bool persist = false;
        time::TimePoint lastKeepAlive;
        PeerInfo peerInfo;
        /// true if we initiated this connection false otherwise
        bool outbound;
    };

}  // namespace omnom

namespace std
{
    template <>
    struct hash<omnom::PeerInfo>
    {
        size_t
        operator()(const omnom::PeerInfo& info) const
        {
            std::string_view view{reinterpret_cast<const char*>(info.uid.data()), info.uid.size()};
            return std::hash<std::string_view>{}(view);
        }
    };
}  // namespace std
