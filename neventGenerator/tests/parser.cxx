#include "../Configuration.hpp"
#include <gtest/gtest.h>

auto source_dir=std::string(CMAKE_CURRENT_SOURCE_DIR);
  
TEST(ConfigurationParser, use_default_if_wrong_syntax) {
  SINQAmorSim::ConfigurationParser parser;
  auto configuration = parser.parse_string_uri("hello",true);
  EXPECT_EQ(configuration.broker, "localhost:9092");
  EXPECT_EQ(configuration.topic, "empty-topic");
}

TEST(ConfigurationParser, leave_empty_if_wrong_syntax) {
  SINQAmorSim::ConfigurationParser parser;
  auto configuration = parser.parse_string_uri("hello",false);
  EXPECT_EQ(configuration.broker, "");
  EXPECT_EQ(configuration.topic, "");

  configuration = parser.parse_string_uri("//localhost:9092/",false);
  EXPECT_EQ(configuration.broker, "localhost:9092");
  EXPECT_EQ(configuration.topic, "");

  configuration = parser.parse_string_uri("//:/my-topic",false);
  EXPECT_EQ(configuration.broker, "");
  EXPECT_EQ(configuration.topic, "my-topic");  
}

TEST(ConfigurationParser, parse_broker_port_topic) {
  SINQAmorSim::ConfigurationParser parser;
  auto configuration = parser.parse_string_uri("//localhost:8080/my-topic");
  EXPECT_EQ(configuration.broker, "localhost:8080");
  EXPECT_EQ(configuration.topic, "my-topic");
}

TEST(ConfigurationParser, missing_json_configuration) {
  SINQAmorSim::ConfigurationParser parser;
  auto filename = source_dir+"/missing_configuration.json";
  auto result = parser.parse_configuration_file(filename);
  EXPECT_EQ(result,SINQAmorSim::ConfigurationError::error_input_file);
}

TEST(ConfigurationParser, invalid_json_configuration) {
  SINQAmorSim::ConfigurationParser parser;
  
  auto filename = source_dir+"/invalid_configuration.json";
  auto result = parser.parse_configuration_file(filename);
  EXPECT_NE(result,
            SINQAmorSim::ConfigurationError::error_no_configuration_error);
}

TEST(ConfigurationParser, read_json_configuration) {
  SINQAmorSim::ConfigurationParser parser;
  auto filename = source_dir+"/valid_configuration.json";
  auto result = parser.parse_configuration_file(filename);
  EXPECT_EQ(result,
            SINQAmorSim::ConfigurationError::error_no_configuration_error);
}

