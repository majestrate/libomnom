#include "common.hpp"
#include "omnom/storage.hpp"
#include <memory>

namespace omnom
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
                bool,          /* Return type */
                EntityStorage, /* Parent class */
                HasEntity,     /* Name of function in C++ (must match Python name) */
                ent            /* Argument(s) */
            );
        }

        void
        StoreEntity(Entity ent) override
        {
            py::gil_scoped_acquire acquire;
            PYBIND11_OVERRIDE_PURE(
                void,          /* Return type */
                EntityStorage, /* Parent class */
                StoreEntity,   /* Name of function in C++ (must match Python name) */
                ent            /* Argument(s) */
            );
        }

        std::optional<Entity>
        GetEntityByID(EntityID id) const override
        {
            py::gil_scoped_acquire acquire;
            PYBIND11_OVERRIDE_PURE(
                std::optional<Entity>, /* Return type */
                EntityStorage,         /* Parent class */
                GetEntityByID,         /* Name of function in C++ (must match Python name) */
                id);
        }

        EntityID
        GetTopEntityID() const override
        {
            py::gil_scoped_acquire acquire;
            PYBIND11_OVERRIDE_PURE(
                EntityID,      /* Return type */
                EntityStorage, /* Parent class */
                GetTopEntityID /* Name of function in C++ (must match Python name) */
            );
        }
    };

    void
    Storage_Init(py::module& mod)
    {
        py::class_<EntityStorage, PyStorage>(mod, "EntityStorage")
            .def(py::init<>())
            .def("HasEntity", &EntityStorage::HasEntity)
            .def("GetEntityByID", &EntityStorage::GetEntityByID)
            .def("GetTopEntityID", &EntityStorage::GetTopEntityID)
            .def("StoreEntity", &EntityStorage::StoreEntity);
    }
}  // namespace omnom
