#include "common.hpp"
#include "entsync/entity.hpp"
namespace entsync
{
  void
  Entity_Init(py::module & mod)
  {
    py::class_<Entity>(mod, "Entity");
  }

}
