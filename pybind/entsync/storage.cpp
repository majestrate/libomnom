#include "common.hpp"
#include "entsync/storage.hpp"
#include <memory>

namespace entsync
{

  class PyStorage : public EntityStorage
  { 
  public:
    using EntityStorage::EntityStorage;
    
    bool
    HasEntity(Entity ent) const override
    {
      py::gil_scoped_acquire acquire;
      PYBIND11_OVERRIDE_PURE(
        bool, /* Return type */
        EntityStorage,      /* Parent class */
        HasEntity,          /* Name of function in C++ (must match Python name) */
        ent      /* Argument(s) */
        );
    }

    void
    StoreEntity(Entity ent) override
    {
      py::gil_scoped_acquire acquire;
      PYBIND11_OVERRIDE_PURE(
        void, /* Return type */
        EntityStorage,      /* Parent class */
        StoreEntity,          /* Name of function in C++ (must match Python name) */
        ent      /* Argument(s) */
        );
    }
  };
  
  void
  Storage_Init(py::module& mod)
  {
    py::class_<EntityStorage, PyStorage>(mod, "EntityStorage")
      .def(py::init<>())
      .def("HasEntity", &EntityStorage::HasEntity)
      .def("StoreEntity", &EntityStorage::StoreEntity);
  }
}
