cmake_minimum_required(VERSION 3.7)

project(libentsync CXX C)

include(GNUInstallDirs)

set(ENTSYNC_VERSION_MAJOR 0)
set(ENTSYNC_VERSION_MINOR 1)
set(ENTSYNC_VERSION_PATCH 0)
set(ENTSYNC_VERSION "${ENTSYNC_VERSION_MAJOR}.${ENTSYNC_VERSION_MINOR}.${ENTSYNC_VERSION_PATCH}")
message(STATUS "libentsync v${ENTSYNC_VERSION}")

set(ENTSYNC_LIBVERSION ${ENTSYNC_VERSION_MAJOR})

option(BUILD_SHARED_LIBS "Build shared libraries instead of static ones" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

configure_file(entsync/version.hpp.in entsync/version.hpp @ONLY)
configure_file(libentsync.pc.in libentsync.pc @ONLY)

add_library(entsync
  entsync/context.cpp
  entsync/entity.cpp
  entsync/gossip.cpp
  entsync/handler.cpp
  entsync/peer_manager.cpp
  entsync/peer_limiter.cpp
  entsync/searcher.cpp
)

set_target_properties(entsync PROPERTIES SOVERSION ${ENTSYNC_LIBVERSION})

include(FindPkgConfig)
pkg_check_modules(oxenqmq REQUIRED liboxenmq IMPORTED_TARGET)
target_link_libraries(entsync PRIVATE oxenmq)


target_compile_options(entsync PRIVATE -Wall -Wextra -Werror)
set_target_properties(entsync PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

add_library(entsync::entsync ALIAS entsync)

export(
    TARGETS entsync
    NAMESPACE entsync::
    FILE entsyncTargets.cmake
)
install(
    TARGETS entsync
    EXPORT entsyncConfig
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/entsync/version.hpp
  entsync/context.hpp
  entsync/crypto_hash.hpp
  entsync/entity.hpp
  entsync/gossip.hpp
  entsync/handler.hpp
  entsync/logger.hpp
  entsync/peer.hpp
  entsync/peer_limiter.hpp
  entsync/peer_manager.hpp
  entsync/searcher.hpp
  entsync/storage.hpp
  entsync/time.hpp
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/entsync
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libentsync.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)


option(WITH_PYBIND "build pybind layer for integration tests" ON)

if(WITH_PYBIND)
  add_subdirectory(external/pybind11)
  add_subdirectory(pybind)
endif()
  
