#include "../nexus_reader.hpp"
#include <gtest/gtest.h>

extern std::string source_dir;

TEST(NexusReader,file_not_found) {  
  // EXPECT_ANY_THROW(NeXus::File("dummy.hdf"));
}

TEST(NexusReader,open_file) {
  auto nexusfile = source_dir + "/../files/amor2015n001774.hdf";
  // EXPECT_NO_THROW(NeXus::File(nexusfile.c_str()));
}

TEST(NexusReader,long_test) {
  auto nexusfile = source_dir + "/../files/amor2015n001774.hdf";

  
}
