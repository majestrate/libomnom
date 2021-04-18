#include "common.hpp"
#include "omnom/gossip.hpp"
#include "omnom/peer.hpp"

namespace omnom
{
  void
  Gossip_Init(py::module & mod)
  {
    py::class_<Gossiper>(mod, "Gossiper")
      .def("broadcast_entity",
           [](Gossiper & self, Entity ent, std::function<bool(const PeerInfo &)> filter)
           {
             // XXX: why yes, we do need this, how could you tell... [gigachad.jpeg]
             py::gil_scoped_release release;
             self.Broadcast(
               std::move(ent),
               [filter](auto, const PeerState & state)
               {
                 try
                 {
                   py::gil_scoped_acquire acquire;
                   return filter(state.peerInfo);
                 }
                 catch(std::exception & ex)
                 {
                   std::cout << ex.what() << std::endl;
                   return false;
                 }
               });
           })
      .def("add_entity_handler",
           [](Gossiper & self, EntityKind kind, py::function func)
           {
             auto handler =
               [func=std::move(func)] (const PeerState & state, Entity ent)
               {
                 try
                 {
                   py::gil_scoped_acquire acquire;
                   func(ent, state.peerInfo);
                 }
                 catch(std::exception & ex)
                 {
                   std::cerr << ex.what() << std::endl;
                 }
               };
             self.AddEntityHandler(kind, handler);
           });



  }
}
