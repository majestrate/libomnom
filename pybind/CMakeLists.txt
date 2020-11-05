
pybind11_add_module(pyentsync MODULE
  module.cpp
  entsync/entity.cpp
  entsync/gossip.cpp
)
target_link_libraries(pyentsync PUBLIC entsync)
target_include_directories(pyentsync PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

