#include "../Errors.hpp"

#include <gtest/gtest.h>

TEST(Errors, no_error_to_str) {
  EXPECT_EQ(SINQAmorSim::Err2Str(0),std::string("no_error"));
}

TEST(Errors, unknown_error_value_to_str) {
  EXPECT_EQ(SINQAmorSim::Err2Str(1),std::string("Unknown error"));
}

TEST(Errors, configuration_error_to_str) {
  EXPECT_EQ(SINQAmorSim::Err2Str(-10),
            std::string("error_input_file"));
  EXPECT_EQ(SINQAmorSim::Err2Str(-11),
            std::string("error_parsing_json"));
  EXPECT_EQ(SINQAmorSim::Err2Str(-12),
            std::string("error_parsing_command_line"));
  EXPECT_EQ(SINQAmorSim::Err2Str(-13),
            std::string("error_configuration_invalid"));
}
