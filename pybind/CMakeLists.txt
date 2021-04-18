
pybind11_add_module(pyomnom MODULE
  module.cpp
  omnom/context.cpp
  omnom/entity.cpp
  omnom/gossip.cpp
  omnom/peer.cpp
  omnom/storage.cpp
)
target_link_libraries(pyomnom PUBLIC omnom)
target_include_directories(pyomnom PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(pyomnom PRIVATE -Wall -Wextra -Werror)
set_target_properties(pyomnom PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

