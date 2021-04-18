#include "common.hpp"
#include "omnom/searcher.hpp"

#include <future>

namespace omnom
{
  void
  Search_Init(py::module & mod)
  {
    py::class_<EntitySearcher>(mod, "Searcher")
      .def("search_for_entity",
           [](EntitySearcher & self, EntityKind kind, EntityID id)
           {
             py::gil_scoped_release release;
             std::promise<std::optional<Entity>> promise;
             self.
               MaybeObtainEntityByID(
                 std::move(kind),
                 std::move(id),
                 [&promise](std::optional<Entity> result)
                 {
                   promise.set_value(result);
                 });
             return promise.get_future().get();
           })
      .def("find_highest_id",
           [](EntitySearcher & self, EntityKind kind) -> EntityID
           {
             std::promise<EntityID> result;
             py::gil_scoped_release release;
             self.GetHighestEntityID(std::move(kind), [&result](EntityID id) { result.set_value(id); });
             auto future = result.get_future();
             return future.get();
           });
  }
}
