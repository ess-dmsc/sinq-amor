#include "../Configuration.hpp"
#include "command_line_args.hpp"

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
  EXPECT_NE(result,
            SINQAmorSim::ConfigurationError::error_input_file);
}

TEST(ConfigurationParser, parse_json_invalid_string) {
  SINQAmorSim::ConfigurationParser parser;
  rapidjson::Document document;
  document.Parse("{ \"producer_broker\" : 1 }");
  auto result = parser.parse_configuration_file_impl(document);
  EXPECT_EQ(result,
            SINQAmorSim::ConfigurationError::error_parsing_json);
  document.Parse("{ \"source\" : 1 }");
  result = parser.parse_configuration_file_impl(document);
  EXPECT_EQ(result,
            SINQAmorSim::ConfigurationError::error_parsing_json);
}

TEST(ConfigurationParser, parse_json_invalid_int) {
  SINQAmorSim::ConfigurationParser parser;
  rapidjson::Document document;
  document.Parse("{ \"multiplier\" : \"1\" }");
  auto result = parser.parse_configuration_file_impl(document);
  EXPECT_EQ(result,
            SINQAmorSim::ConfigurationError::error_parsing_json);
  document.Parse("{ \"rate\" : \"1\" }");
  result = parser.parse_configuration_file_impl(document);
  EXPECT_EQ(result,
            SINQAmorSim::ConfigurationError::error_parsing_json);
}

TEST(ConfigurationParser, parse_valid_json) {
  SINQAmorSim::ConfigurationParser parser;
  rapidjson::Document document;
  document.Parse("{ \"multiplier\" : 1,"
                 "\"rate\" : 2,"
                 "\"source\" : \"file.h5\","
                 "\"producer_broker\" : \"//localhost:9092/my-topic\"}");
  auto result = parser.parse_configuration_file_impl(document);
  EXPECT_EQ(result,
            SINQAmorSim::ConfigurationError::error_no_configuration_error);
  EXPECT_EQ(parser.config.producer.broker, std::string("localhost:9092"));
  EXPECT_EQ(parser.config.producer.topic,std::string("my-topic"));
  EXPECT_EQ(parser.config.source,std::string("file.h5"));  
  EXPECT_EQ(parser.config.multiplier,1);
  EXPECT_EQ(parser.config.rate,2);
}

TEST(ConfigurationParser, verify_json) {
  SINQAmorSim::ConfigurationParser parser;
  rapidjson::Document document;
  document.Parse("{ \"multiplier\" : 1,"
                 "\"rate\" : 2,"
                 "\"source\" : \"file.h5\","
                 "\"producer_broker\" : \"//localhost:9092/my-topic\"}");
  auto result = parser.parse_configuration_file_impl(document);
  EXPECT_EQ(parser.config.producer.broker, std::string("localhost:9092"));
  EXPECT_EQ(parser.config.producer.topic,std::string("my-topic"));
  EXPECT_EQ(parser.config.source,std::string("file.h5"));  
  EXPECT_EQ(parser.config.multiplier,1);
  EXPECT_EQ(parser.config.rate,2);
}

extern CLA argin;

TEST(ConfigurationParser, no_error_empty_command_line) {
  SINQAmorSim::ConfigurationParser parser;
  EXPECT_NE(parser.parse_command_line(2,argin.argv),
            SINQAmorSim::ConfigurationError::error_no_configuration_error);
  EXPECT_EQ(parser.parse_command_line(1,argin.argv),
            SINQAmorSim::ConfigurationError::error_no_configuration_error);
}

TEST(ConfigurationParser, command_line_error) {
  SINQAmorSim::ConfigurationParser parser;
  EXPECT_EQ(parser.parse_command_line(1,argin.argv),
            SINQAmorSim::ConfigurationError::error_no_configuration_error);
  EXPECT_NE(parser.parse_command_line(2,argin.argv),
            SINQAmorSim::ConfigurationError::error_no_configuration_error);
}
