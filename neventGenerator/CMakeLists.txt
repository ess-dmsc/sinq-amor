cmake_minimum_required (VERSION 2.6)
set(PROJECT_NAME_STR neventGenerator)
project(${PROJECT_NAME_STR} CXX)

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

add_custom_target(
  extern_lib
  COMMAND make
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/cJSON
)

# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}/cJSON"
  "${PROJECT_SOURCE_DIR}/rapidjson/include"
  "${PROJECT_SOURCE_DIR}"
  "${RDKAFKA_DIR}/include"
  "${NEXUS_DIR}/include"
  "${FLATBUFFERS_DIR}/include")


link_directories("/usr/lib")
link_directories("/usr/lib64")
link_directories("/opt/local/lib")
link_directories("/usr/local/lib")
link_directories("/usr/local/lib64")
link_directories("${RDKAFKA_DIR}/lib")
link_directories("${NEXUS_DIR}/lib")
link_directories("${FLATBUFFERS_DIR}/lib")
link_directories("${PROJECT_SOURCE_DIR}/cJSON")

add_executable(AMORgenerator AMORgenerator.cpp)
add_executable(AMORreceiver AMORreceiver.cpp)
target_link_libraries (AMORgenerator zmq rdkafka rdkafka++ NeXus sodium cjson hdf5)
target_link_libraries (AMORreceiver zmq rdkafka rdkafka++ NeXus sodium cjson hdf5)

add_dependencies(AMORgenerator extern_lib)
add_dependencies(AMORreceiver extern_lib)


enable_testing()
set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_test)

include_directories(${GTEST_INCLUDE})
file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/tests/*.cpp)
add_executable(${PROJECT_TEST_NAME} ${TEST_SRC_FILES})
add_dependencies(${PROJECT_TEST_NAME} googletest)

target_link_libraries(${PROJECT_TEST_NAME}
  ${GTEST_LIB}/libgtest.a
  ${GTEST_LIB}/libgtest_main.a)

add_test(test1 ${PROJECT_TEST_NAME})

