#include "common.hpp"
#include "entsync/context.hpp"

namespace entsync
{

  constexpr auto LogPrint =
    [](oxenmq::LogLevel lvl, const char * file, int line, std::string msg)
    { std::cout << lvl << " " << file << ":" << line << " " << msg << std::endl; };
    

  class PyContext : public Context
  {
  public:
    oxenmq::OxenMQ lmq;

    explicit PyContext(std::string dialect)
      : Context(lmq, std::move(dialect)),
        lmq{LogPrint, oxenmq::LogLevel::info}
    {
    }

    std::vector<std::string>
    GetPeers() const
    {
      return m_Peers.GetPeerAddresses();
    }

  };


  void
  Context_Init(py::module & mod)
  {
    py::class_<PyContext>(mod, "Context")
      .def(py::init<std::string>())
      .def("set_storage_for",
           [](PyContext & self, EntityKind kind, EntityStorage & storage)
           {
             self.Gossip().SetEntityStorage(kind, storage);
           })
      .def("add_entity_handler",
           [](PyContext & self, EntityKind kind, py::function func)
           {
             self.Gossip().AddEntityHandler(
               kind,
               [func](PeerState state, Entity ent)
               {
                 py::gil_scoped_acquire acquire;
                 func(ent, state.peerInfo);
               });
           })

      .def("broadcast_entity",
           [](PyContext & self, Entity ent, std::function<bool(const PeerInfo &)> filter)
           {
             py::gil_scoped_release release;
             self.Gossip().Broadcast(
               std::move(ent),
               [filter](auto, PeerState state)
               {
                 py::gil_scoped_acquire acquire;
                 try
                 {
                   return filter(state.peerInfo);
                 }
                 catch(std::exception & ex)
                 {
                   std::cout << ex.what() << std::endl;
                   return false;
                 }
               });
           })
      .def("listen",
          [](PyContext & self, std::string peerAddr)
          {
            PeerAddr addr;
            addr.addr = peerAddr;
            addr.rank = 1;
            self.Listen(peerAddr, {addr});
          })
      .def("add_peer",
           [](PyContext & self, std::string addr)
           {
             self.AddPersistingPeer(oxenmq::address{addr});
           })
      .def("start",
           [](PyContext & self)
           {
             self.Start();
             self.lmq.start();
           })
      .def("peers", &PyContext::GetPeers)
      .def_readwrite("maxPeers", &PyContext::maxPeers)
      ;
  }
}
