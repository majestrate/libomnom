#include "searcher.hpp"

namespace omnom
{
  EntitySearcher::EntitySearcher(Context * ctx) : _ctx{ctx} {}


  /// ask all our peers and get the highest entity id
  void
  EntitySearcher::GetHighestEntityID(EntityKind kind, std::function<void(EntityID)> handler)
  {
    (void) kind;
    handler(EntityID{0});
  }

  /// ask all our peers and get the highest entity id
  void
  EntitySearcher::MaybeObtainEntityByID(EntityKind kind, EntityID id, std::function<void(std::optional<Entity>)> handler)
  {
    (void) kind;
    (void) id;
    handler(std::nullopt);
  }


}
