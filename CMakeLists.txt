cmake_minimum_required(VERSION 3.7)

project(libomnom CXX C)

include(GNUInstallDirs)

set(OMNOM_VERSION_MAJOR 0)
set(OMNOM_VERSION_MINOR 1)
set(OMNOM_VERSION_PATCH 0)
set(OMNOM_VERSION "${OMNOM_VERSION_MAJOR}.${OMNOM_VERSION_MINOR}.${OMNOM_VERSION_PATCH}")
message(STATUS "libomnom v${OMNOM_VERSION}")

set(OMNOM_LIBVERSION ${OMNOM_VERSION_MAJOR})

option(BUILD_SHARED_LIBS "Build shared libraries instead of static ones" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

configure_file(omnom/version.hpp.in omnom/version.hpp @ONLY)
configure_file(libomnom.pc.in libomnom.pc @ONLY)

add_library(omnom
  omnom/context.cpp
  omnom/entity.cpp
  omnom/gossip.cpp
  omnom/handler.cpp
  omnom/peer_manager.cpp
  omnom/peer_limiter.cpp
  omnom/searcher.cpp
)

set_target_properties(omnom PROPERTIES SOVERSION ${OMNOM_LIBVERSION})

include(FindPkgConfig)
pkg_check_modules(oxenqmq REQUIRED liboxenmq IMPORTED_TARGET)
target_link_libraries(omnom PRIVATE oxenmq)


target_compile_options(omnom PRIVATE -Wall -Wextra -Werror)
set_target_properties(omnom PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

add_library(omnom::omnom ALIAS omnom)

export(
    TARGETS omnom
    NAMESPACE omnom::
    FILE omnomTargets.cmake
)
install(
    TARGETS omnom
    EXPORT omnomConfig
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/omnom/version.hpp
  omnom/context.hpp
  omnom/crypto_hash.hpp
  omnom/entity.hpp
  omnom/gossip.hpp
  omnom/handler.hpp
  omnom/logger.hpp
  omnom/peer.hpp
  omnom/peer_limiter.hpp
  omnom/peer_manager.hpp
  omnom/searcher.hpp
  omnom/storage.hpp
  omnom/time.hpp
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/omnom
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libomnom.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)


option(WITH_PYBIND "build pybind layer for integration tests" OFF)

if(WITH_PYBIND)
  add_subdirectory(external/pybind11)
  add_subdirectory(pybind)
endif()
  
option(WITH_TESTS "run integration tests" OFF)

if(WITH_TESTS)
  add_subdirectory(tests)
endif()
