file(GLOB SOURCES ./*.cpp)
file(GLOB HEADERS ./*.h)

if (WIN32 OR EMSCRIPTEN)
  add_library(pmp STATIC ${SOURCES} ${HEADERS})
elseif(UNIX)
  add_library(pmp SHARED ${SOURCES} ${HEADERS})
endif()

add_subdirectory(visualization)
include(algorithms/CMakeLists.txt)
