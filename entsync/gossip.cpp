#include "gossip.hpp"


namespace entsync
{
  Gossiper::Gossiper(PeerManager * peerManager) : _peerManager{peerManager}
  {
  }

  void
  Gossiper::Broadcast(Entity ent)
  {
    (void)ent;
  }
}
