#include "common.hpp"
#include <thread>
#include <chrono>

PYBIND11_MODULE(pyentsync, mod)
{
  entsync::Context_Init(mod);
  entsync::Entity_Init(mod);
  entsync::Storage_Init(mod);
  entsync::Peer_Init(mod);
  mod.def(
    "sleep_ms",
    [](int ms)
    {
      py::gil_scoped_release rel;
      std::this_thread::sleep_for(std::chrono::milliseconds{ms});
    });
}
