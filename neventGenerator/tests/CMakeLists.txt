set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../tests)

message(STATUS "Compiling tests")

set(tgt "tests")
set(sources
  tests.cxx
  parser.cxx
  ../Configuration.cxx
)

include_directories(
  ${path_include_rdkafka}
  ${path_include_flatbuffers}
  ${path_include_rapidjson}
  ${path_include_hdf5}
  ${path_include_nexus}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

add_executable(${tgt} ${sources})
add_dependencies(${tgt} flatbuffers_generate)
target_compile_definitions(${tgt} PRIVATE ${compile_defs_common})
target_include_directories(${tgt} PRIVATE ${path_include_common})
target_link_libraries(${tgt} ${libraries_common})
add_gtest_to_target(${tgt})
