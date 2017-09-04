#include <gtest/gtest.h>

#include "command_line_args.hpp"
CLA argin;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  argin.argc = argc;
  argin.argv = argv;  

  return RUN_ALL_TESTS();
}
