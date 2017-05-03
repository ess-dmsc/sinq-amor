cmake_minimum_required (VERSION 2.8)
set(PROJECT_NAME_STR neventGenerator)
project(${PROJECT_NAME_STR} CXX)

set (CMAKE_CXX_FLAGS "-std=c++11 -pthread -D_GLIBCXX_USE_NANOSLEEP ${CMAKE_CXX_FLAGS}")

add_custom_target(
  extern_lib
  COMMAND make
)

# flatbuffers
find_path(path_include_flatbuffers NAMES flatbuffers/flatbuffers.h)
find_program(path_flatc NAMES flatc)
message(STATUS "path_include_flatbuffers: ${path_include_flatbuffers}")
message(STATUS "flatc: " ${path_flatc})

set(flatbuffers_schemata
  amo0_psi_sinq_schema
)

find_path(path_include_streaming_data_types NAMES schemas/amo0_psi_sinq.fbs)
message(STATUS "path_include_streaming_data_types: ${path_include_streaming_data_types}")


set(schemas_subdir "schemas")
set(head_out_dir "${CMAKE_CURRENT_BINARY_DIR}/${schemas_subdir}")
file(MAKE_DIRECTORY ${head_out_dir})
file(GLOB_RECURSE flatbuffers_schemata RELATIVE "${path_include_streaming_data_types}/schemas" "${path_include_streaming_data_types}/schemas/*.fbs")


message(STATUS)

message(STATUS "schema list: ${flatbuffers_schemata}")
foreach (f ${flatbuffers_schemata})
  string(REGEX REPLACE "\\.fbs$" "" s0 ${f})
  set(fbs "${s0}.fbs")
  set(fbh "${s0}_generated.h")
  message(STATUS "fbs: ${fbs}")
  message(STATUS "fbh: ${fbh}")

  add_custom_command(
    OUTPUT "${head_out_dir}/${fbh}"
    COMMAND ${path_flatc} --cpp --gen-mutable --gen-name-strings --scoped-enums "${path_include_streaming_data_types}/schemas/${fbs}"
    DEPENDS "${path_include_streaming_data_types}/schemas/${fbs}"
    WORKING_DIRECTORY "${head_out_dir}"
    COMMENT "Process ${fbs} using ${flatc}"
    message(STATUS "Process ${fbs} using ${flatc}")
    )
  list(APPEND flatbuffers_generated_headers "${head_out_dir}/${fbh}")

endforeach()
add_custom_target(flatbuffers_generate ALL DEPENDS ${flatbuffers_generated_headers})
message(STATUS)


#rapidjson
find_path(path_include_rapidjson NAMES rapidjson/document.h)
message(STATUS "path_include_rapidjson: " ${path_include_rapidjson})

#0mq
option(HAVE_0MQ "Enable 0MQ" FALSE)
if(${HAVE_0MQ})
  find_path(path_include_zmq NAMES zmq.h)
  find_library(path_lib_zmq NAMES zmq)
  message(STATUS "path_include_zmq: " ${path_include_zmq})
  message(STATUS "path_lib_zmq: " ${path_lib_zmq} )
  add_library(libzmq SHARED IMPORTED)
  set_property(TARGET libzmq PROPERTY IMPORTED_LOCATION ${path_lib_zmq})

  #libsodium
  find_library(path_lib_sodium NAMES sodium)
  message(STATUS "path_lib_sodium: " ${path_lib_sodium} )
  add_library(libsodium SHARED IMPORTED)
  set_property(TARGET libsodium PROPERTY IMPORTED_LOCATION ${path_lib_sodium})

include_directories( ${path_include_zmq} )
link_libraries(libzmq libsodium)
else()
  message(STATUS "Build without 0MQ support")
endif()

#rdkafka
find_path(path_include_rdkafka NAMES librdkafka/rdkafka.h)
find_library(path_lib_rdkafka NAMES rdkafka)
add_library(librdkafka SHARED IMPORTED)
set_property(TARGET librdkafka PROPERTY IMPORTED_LOCATION ${path_lib_rdkafka})

find_library(path_lib_rdkafka++ NAMES rdkafka++)
add_library(librdkafka++ SHARED IMPORTED)
set_property(TARGET librdkafka++ PROPERTY IMPORTED_LOCATION ${path_lib_rdkafka++})

message(STATUS "path_include_rdkafka: " ${path_include_rdkafka})
message(STATUS "path_lib_rdkafka: " ${path_lib_rdkafka})
message(STATUS "path_lib_rdkafka++: " ${path_lib_rdkafka++})

#nexus
find_path(path_include_nexus NAMES nexus/napi.h)
find_library(path_lib_nexus NAMES NeXus)

message(STATUS "path_include_nexus: " ${path_include_nexus})
message(STATUS "path_lib_nexus: " ${path_lib_nexus})
add_library(libNeXus SHARED IMPORTED)
set_property(TARGET libNeXus PROPERTY IMPORTED_LOCATION ${path_lib_nexus})


#hdf5
find_path(path_include_hdf5 NAMES hdf5.h)
find_library(path_lib_hdf5 NAMES hdf5-shared)
if( NOT path_lib_hdf5 )
  find_library(path_lib_hdf5 NAMES hdf5)
endif()
add_library(libhdf5 SHARED IMPORTED)
set_property(TARGET libhdf5 PROPERTY IMPORTED_LOCATION ${path_lib_hdf5})

message(STATUS "path_include_hdf5: " ${path_include_hdf5})
message(STATUS "path_lib_hdf5: " ${path_lib_hdf5})

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(
  ${path_include_rdkafka}
  ${path_include_flatbuffers}
  ${path_include_rapidjson}
  ${path_include_hdf5}
  ${path_include_nexus}
  ${CMAKE_CURRENT_BINARY_DIR}
)

link_libraries(librdkafka++
  librdkafka
  z
)

add_executable(AMORgenerator AMORgenerator.cpp)
add_executable(AMORreceiver AMORreceiver.cpp)
target_link_libraries (AMORgenerator libNeXus libhdf5)

add_dependencies(AMORgenerator flatbuffers_generate)
add_dependencies(AMORreceiver flatbuffers_generate)

if(NOT ${CMAKE_INSTALL_PREFIX} EQUAL "")
  message(STATUS "Files will be installed in : " ${CMAKE_INSTALL_PREFIX})
  install(TARGETS AMORgenerator RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  install(FILES el737counter.py DESTINATION ${CMAKE_INSTALL_PREFIX})
  install(FILES config.in DESTINATION ${CMAKE_INSTALL_PREFIX})
  install(FILES files/amor2015n001774.hdf DESTINATION ${CMAKE_INSTALL_PREFIX}/files)
else()
  install(TARGETS AMORgenerator RUNTIME DESTINATION /opt/amor/simfiles)
  install(FILES config.in DESTINATION ${CMAKE_INSTALL_PREFIX})
  install(FILES files/amor2015n001774.hdf DESTINATION ${CMAKE_INSTALL_PREFIX}/files)
endif()
