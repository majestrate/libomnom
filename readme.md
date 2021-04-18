# libomnom

**O**pen **M**eta **N**etwork **O**verlay **M**esh

p2p entity gossip layer atop zmq

## build

depends:

* cmake
* pkg-config
* c++17 compiler
* oxenmq >= 1.2


clone the source (this is a recursive repo):

    $ git clone --recursive https://github.com/majestrate/libomnom
    $ cd libomnom

compile as a library:

    $ mkdir -p build && cd build
    $ cmake .. && make



## tests

if you are building integration tests you also need:

* python 3.x
* pybind11
* pytest

compile and run integration tests:

    $ mkdir -p build && cd build
    $ cmake .. -DWITH_PYBIND=ON -DWITH_TESTS=ON
    $ make check
