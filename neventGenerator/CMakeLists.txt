cmake_minimum_required (VERSION 2.8)
set(PROJECT_NAME_STR neventGenerator)
project(${PROJECT_NAME_STR} CXX)

set (CMAKE_CXX_FLAGS "-std=c++11 -pthread  -D_GLIBCXX_USE_NANOSLEEP ${CMAKE_CXX_FLAGS}")

add_custom_target(
  extern_lib
  COMMAND make
)

# flatbuffers
find_path(path_include_flatbuffers NAMES flatbuffers/flatbuffers.h PATHS
 "$ENV{flatbuffers_dir}/include"
 /opt/local/flatbuffers.git/include
)
find_path(path_flatc NAMES flatc PATHS
  $ENV{flatc_dir}
)

message(STATUS flatbuffers_inc: ${path_include_flatbuffers})
message(STATUS flatc: ${path_flatc})
set(flatbuffers_schemata
  amo0_psi_sinq_schema
)
set(flatbuffers_generated_headers "")
foreach (f ${flatbuffers_schemata})
  set(fbs "${PROJECT_SOURCE_DIR}/sample/${f}.fbs")
  set(fbh "${f}_generated.h")
  message(STATUS ${fbs})
  message(STATUS ${fbh})
#  message(STATUS ${})
  add_custom_command(
    OUTPUT ${fbh}
    COMMAND "${path_flatc}/flatc" --cpp "${fbs}"
    DEPENDS "${fbs}"
    COMMENT "Process ${fbs} using ${path_flatc}/flatc"
    )
  list(APPEND flatbuffers_generated_headers "${fbh}")
  message(STATUS flatbuffers_generated_headers:${flatbuffers_generated_headers})

endforeach()
add_custom_target(flatbuffers_generate ALL DEPENDS ${flatbuffers_generated_headers})



#rapidjson
find_path(path_include_rapidjson NAMES rapidjson/document.h PATHS
 "$ENV{rapidjson_dir}/include"
 /opt/local/rapidjson.git/include
)
#find_library(path_lib_rapidjson NAMES librapidjson PATHS
#"$ENV{rapidjson_dir}/lib"
# /opt/local/rapidjson.git/lib
#)
message(STATUS rapidjson_inc: ${path_include_rapidjson})
#message(STATUS ${path_lib_rapidjson})
#add_library(librapidjson SHARED IMPORTED)
#set_property(TARGET librapidjson PROPERTY IMPORTED_LOCATION ${path_lib_rapidjson})

#0mq
find_path(path_include_zmq NAMES zmq.h PATHS
 "$ENV{zmq_dir}/include"
)
find_library(path_lib_zmq NAMES zmq PATHS
"$ENV{zmq_dir}/lib"
)
message(STATUS zmq_inc: ${path_include_zmq})
message(STATUS zmq_lib: ${path_lib_zmq} )

add_library(libzmq SHARED IMPORTED)
set_property(TARGET libzmq PROPERTY IMPORTED_LOCATION ${path_lib_zmq})

#libsodium
find_library(path_lib_sodium NAMES sodium PATHS
"$ENV{zmq_dir}/lib"
)
message(STATUS sodium_lib: ${path_lib_sodium} )

add_library(libsodium SHARED IMPORTED)
set_property(TARGET libsodium PROPERTY IMPORTED_LOCATION ${path_lib_sodium})


#rdkafka
find_path(path_include_rdkafka NAMES librdkafka/rdkafka.h PATHS
 "$ENV{librdkafka_dir}/include"
 /opt/local/rdkafka/install/include
 /usr/local/include
)
find_library(path_lib_rdkafka NAMES rdkafka PATHS
"$ENV{librdkafka_dir}/lib"
/opt/local/rdkafka/install/lib
/usr/local/lib
)
add_library(librdkafka SHARED IMPORTED)
set_property(TARGET librdkafka PROPERTY IMPORTED_LOCATION ${path_lib_rdkafka})

find_library(path_lib_rdkafka++ NAMES rdkafka++ PATHS
 "$ENV{librdkafka_dir}/lib"
 /opt/local/rdkafka/install/lib
 /usr/local/lib
)
add_library(librdkafka++ SHARED IMPORTED)
set_property(TARGET librdkafka++ PROPERTY IMPORTED_LOCATION ${path_lib_rdkafka++})

#nexus
find_path(path_include_nexus NAMES napi.h PATHS
  "$ENV{nexus_dir}/include/nexus"
  "$ENV{nexus_dir}/include"
)
find_library(path_lib_nexus NAMES NeXus PATHS
  "$ENV{nexus_dir}/lib"
  "$ENV{nexus_dir}/lib/nexus"
  )
message(STATUS NeXus_inc: ${path_include_nexus})
message(STATUS NeXus_lib: ${path_lib_nexus})
add_library(libNeXus SHARED IMPORTED)
set_property(TARGET libNeXus PROPERTY IMPORTED_LOCATION ${path_lib_nexus})


#hdf5
find_path(path_include_hdf5 NAMES hdf5.h PATHS
  "$ENV{hdf5_dir}/include"
)
find_library(path_lib_hdf5 NAMES hdf5-shared PATHS
  "$ENV{hdf5_dir}/lib"
  )
#find_library(path_lib_hdf5cpp NAMES hdf5_cpp-shared PATHS
#  "$ENV{hdf5_dir}/lib"
#  )
if( NOT path_lib_hdf5 )
  find_library(path_lib_hdf5 NAMES hdf5 PATHS
  "$ENV{hdf5_dir}/lib"
  )
#  find_library(path_lib_hdf5cpp NAMES hdf5_cpp PATHS
#    "$ENV{hdf5_dir}/lib"
#    )
endif()

message(STATUS hdf5_inc: ${path_include_hdf5})
message(STATUS hdf5_lib: ${path_lib_hdf5})

add_library(libhdf5 SHARED IMPORTED)
set_property(TARGET libhdf5 PROPERTY IMPORTED_LOCATION ${path_lib_hdf5})

#add_library(libhdf5cpp SHARED IMPORTED)
#set_property(TARGET libhdf5cpp PROPERTY IMPORTED_LOCATION ${path_lib_hdf5cpp})


include_directories(
  ${path_include_rdkafka}
  ${path_include_flatbuffers}
  ${path_include_rapidjson}
  ${path_include_zmq}
  ${path_include_hdf5}
  ${CMAKE_CURRENT_BINARY_DIR}
)

link_libraries(librdkafka
  librdkafka++
  libzmq
  libsodium
)

add_executable(AMORgenerator AMORgenerator.cpp)
add_executable(AMORreceiver AMORreceiver.cpp)
target_link_libraries (AMORgenerator libNeXus libhdf5)
#target_link_libraries (AMORreceiver)

add_dependencies(AMORgenerator flatbuffers_generate)
add_dependencies(AMORreceiver flatbuffers_generate)
