#include "searcher.hpp"

namespace omnom
{
  EntitySearcher::EntitySearcher(Context * ctx) : _ctx{ctx} {}


  /// ask all our peers and get the highest entity id
  void
  EntitySearcher::GetHighestEntityID(EntityKind kind, std::function<void(EntityID)> handler)
  {
    (void) kind;
    (void) handler;
  }

  /// ask all our peers and get the highest entity id
  void
  EntitySearcher::MaybeObtainEntityByID(EntityKind kind, EntityID id, std::function<void(std::optional<Entity>)> handler)
  {
    (void) kind;
    (void) id;
    (void) handler;
  }


}
