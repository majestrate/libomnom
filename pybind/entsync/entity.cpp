#include "common.hpp"
#include "entsync/entity.hpp"
namespace entsync
{
  void
  Entity_Init(py::module & mod)
  {
    py::class_<EntityKind>(mod, "EntityKind")
      .def(py::init<std::string, bool>())
      .def_readwrite("name", &EntityKind::name)
      .def_readwrite("ephemeral", &EntityKind::ephemeral);

    py::class_<EntityID>(mod, "EntityID")
      .def(py::init<uint64_t>())
      .def("value",
           [](const EntityID & self) -> std::string {
             return self.ToString();
           })
      .def("__repr__",
           [](const EntityID & self) -> std::string
           {
             return self.ToString();
           });
    
    py::class_<Entity>(mod, "Entity")
      .def(py::init<>())
      .def("set_data", [](Entity & ent, std::string data)
      {
        ent.Data = data;
      })
      .def("__repr__",
           [](const Entity & ent) -> std::string
           {
             return ent.ToString();
           })
      .def_readwrite("Kind", &Entity::Kind)
      .def_readwrite("ID", &Entity::ID);
  }

}
