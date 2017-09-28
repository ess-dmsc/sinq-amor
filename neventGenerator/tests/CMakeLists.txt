set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../tests)

message(STATUS "Compiling tests")

set(sources
  tests.cxx
  parser.cxx
  errors.cxx
  kafka_generator.cxx
  serialiser.cxx
  $<TARGET_OBJECTS:nevent-generator__objects>
  )
include_directories(
  ${RDKAFKA_INCLUDE_DIR}
  ${FLATBUFFERS_INCLUDE_DIR}
  ${RAPIDJSON_INCLUDE_DIR}
  ${HDF5_INCLUDE_DIR}
  ${NEXUS_INCLUDE_DIR}
  ${ZMQ_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set(tgt "tests")

add_executable(${tgt} ${sources})
target_compile_definitions(${tgt} PRIVATE ${compile_defs_common})
target_compile_definitions(${tgt} PRIVATE CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${tgt} PRIVATE ${path_include_common})
target_link_libraries(${tgt} ${libraries_common})

add_gtest_to_target(${tgt})
