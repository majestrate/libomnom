#include "common.hpp"
#include "omnom/peer.hpp"

#include "oxenmq/hex.h"

namespace omnom
{
  void
  Peer_Init(py::module & mod)
  {
    py::class_<PeerAddr>(mod, "PeerAddr")
      .def_readwrite("addr", &PeerAddr::addr);


    py::class_<PeerInfo>(mod, "PeerInfo")
      .def("uid",
          [](const PeerInfo & info)
          {
            return oxenmq::to_hex(info.uid.begin(), info.uid.end());
          })
      .def("addrs",
          [](const PeerInfo & info)
          {
            std::vector<PeerAddr> addrs{};
            for(const auto & addr : info.addrs)
              addrs.push_back(addr);
            return addrs;
          });
  }
}
    
