#include "common.hpp"
#include <thread>
#include <chrono>

PYBIND11_MODULE(pyomnom, mod)
{
  omnom::Context_Init(mod);
  omnom::Entity_Init(mod);
  omnom::Storage_Init(mod);
  omnom::Peer_Init(mod);
  omnom::Gossip_Init(mod);
  omnom::Search_Init(mod);
  mod.def(
    "sleep_ms",
    [](int ms)
    {
      py::gil_scoped_release rel;
      std::this_thread::sleep_for(std::chrono::milliseconds{ms});
    });
}
