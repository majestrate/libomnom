#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace py = pybind11;

namespace omnom
{
  void
  Context_Init(py::module& mod);
  
  void
  Entity_Init(py::module& mod);
  
  void
  Gossip_Init(py::module& mod);

  void
  Storage_Init(py::module& mod);

  void
  Peer_Init(py::module & mod);

  void
  Search_Init(py::module & mod);
}
