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
      .def("search_for_entity",
           [](PyContext & self, EntityKind kind, EntityID id)
           {
             py::gil_scoped_release release;
             std::promise<std::optional<Entity>> promise;
             self.Search().
               MaybeObtainEntityByID(
                 std::move(kind),
                 std::move(id),
                 [&promise](std::optional<Entity> result)
                 {
                   promise.set_value(result);
                 });
             return promise.get_future().get();
           })
      .def("add_entity_handler",
           [](PyContext & self, EntityKind kind, py::function func)
           {
             self.Gossip().AddEntityHandler(
               kind,
               [func, &self](PeerState state, Entity ent)
               {
                 std::promise<void> result;
                 self.lmq.job(
                   [state, ent, func, &result]()
                   {
                     try
                     {
                       py::gil_scoped_acquire acquire;
                       func(ent, state.peerInfo);
                     }
                     catch(std::exception & ex)
                     {
                       std::cout << ex.what() << std::endl;
                     }
                     result.set_value();
                   });
                 result.get_future().get();
               });
           })

      .def("broadcast_entity",
           [](PyContext & self, Entity ent, std::function<bool(const PeerInfo &)> filter)
           {
             // XXX: why yes, we do need this, how could you tell... [gigachad.jpeg]
             py::gil_scoped_release release;
             self.Gossip().Broadcast(
               std::move(ent),
               [filter](auto, PeerState state)
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
