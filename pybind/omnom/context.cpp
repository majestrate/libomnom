#include "common.hpp"
#include "omnom/context.hpp"

namespace omnom
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
        lmq{}
    {
      lmq.set_general_threads(1);
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
      .def("searcher",
           [](PyContext & self) -> omnom::EntitySearcher &
           {
             return self.Search();
           }, py::return_value_policy::reference)
      .def("gossiper",
           [](PyContext & self) -> omnom::Gossiper &
           {
             return self.Gossip();
           }, py::return_value_policy::reference)
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
