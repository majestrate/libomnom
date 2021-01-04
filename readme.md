# entity sync

p2p entity gossip layer atop lokimq

## build

depends:

* cmake
* pkg-config
* c++17 compiler
* lokimq >= 1.2


clone the source (this is a recursive repo):

    $ git clone --recursive https://github.com/majestrate/entity-sync
    $ cd entity-sync

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
    $ cmake .. -DWITH_PYBIND=ON
    $ make
    $ PYTHONPATH=pybind pytest -m ../tests/
