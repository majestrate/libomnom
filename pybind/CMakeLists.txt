
pybind11_add_module(pyentsync MODULE
  module.cpp
  entsync/context.cpp
  entsync/entity.cpp
  entsync/gossip.cpp
  entsync/storage.cpp
)
target_link_libraries(pyentsync PUBLIC entsync)
target_include_directories(pyentsync PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(pyentsync PRIVATE -Wall -Wextra -Werror)
set_target_properties(pyentsync PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

