#include <gtest/gtest.h>

int main(int argc, char **argv) {
  std::cout << CMAKE_CURRENT_SOURCE_DIR << std::endl;
  
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
