#include "common.hpp"
#include "entsync/context.hpp"

namespace entsync
{

  constexpr auto LogPrint =
    [](lokimq::LogLevel lvl, const char * file, int line, std::string msg)
    { std::cout << lvl << " " << file << ":" << line << " " << msg << std::endl; };
    

  class PyContext : public Context
  {
  public:
    lokimq::LokiMQ lmq;

    PyContext(std::string dialect)
      : Context(lmq, std::move(dialect)),
        lmq{LogPrint, lokimq::LogLevel::info}
    {
    }
  };

  
  void
  Context_Init(py::module & mod)
  {
    py::class_<PyContext>(mod, "Context")
      .def(py::init<std::string>())
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
             self.AddPersistingPeer(lokimq::address{addr});
           })
      .def("start",
           [](PyContext & self)
           {
             self.Start();
             self.lmq.start();
           })
      .def_readwrite("maxPeers", &PyContext::maxPeers)
      ;
  }
}
