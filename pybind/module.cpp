#include "common.hpp"

PYBIND11_MODULE(pyentsync, mod)
{
  entsync::Context_Init(mod);
}
