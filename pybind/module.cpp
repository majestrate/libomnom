#include "common.hpp"

PYBIND11_MODULE(pyentsync, mod)
{
  entsync::Context_Init(mod);
  entsync::Entity_Init(mod);
  entsync::Gossip_Init(mod);
}
