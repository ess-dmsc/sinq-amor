cmake_minimum_required (VERSION 2.6)
project (neventGenerator)

# The version number.
set (neventGenerator_VERSION_MAJOR 1)
set (neventGenerator_VERSION_MINOR 0)

set (CMAKE_CXX_FLAGS "-std=c++11 -pthread  -D_GLIBCXX_USE_NANOSLEEP ${CMAKE_CXX_FLAGS}")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/neventGeneratorConfig.h.in"
  "${PROJECT_BINARY_DIR}/neventGeneratorConfig.h"
  )
 
# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${RDKAFKA_DIR}/include")
include_directories("${NEXUS_DIR}/include")
include_directories("${FLATBUFFERS_DIR}/include")

link_directories("/usr/lib")
link_directories("/usr/lib64")
link_directories("/usr/local/lib")
link_directories("/usr/local/lib64")
link_directories("${RDKAFKA_DIR}/lib")
link_directories("${NEXUS_DIR}/lib")
link_directories("${FLATBUFFERS_DIR}/lib")
link_directories("${PROJECT_SOURCE_DIR}/cJSON")

add_executable(AMORgenerator AMORgenerator.cpp)
target_link_libraries (AMORgenerator zmq rdkafka rdkafka++ NeXus NeXusCPP sodium cjson)


 
