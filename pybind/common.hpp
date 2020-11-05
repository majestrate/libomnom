#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace py = pybind11;

namespace entsync
{
  void
  Context_Init(py::module& mod);
  
  void
  Entity_Init(py::module& mod);
  
  void
  Gossip_Init(py::module& mod);
  
}
