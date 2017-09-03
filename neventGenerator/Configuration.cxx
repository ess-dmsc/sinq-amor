#include "Configuration.hpp"

#include "rapidjson.h"
#include "rapidjson/istreamwrapper.h"

#include <fstream>

int
SINQAmorSim::ConfigurationParser::parse_configuration_file(const std::string& input) {
  std::ifstream ifs(input);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document d;
  d.ParseStream(isw);
  return Error::error_unknown;
}

int SINQAmorSim::ConfigurationParser::parse_command_line() {
  return Error::error_unknown;
}

int
SINQAmorSim::ConfigurationParser::parse_configuration_file_impl(rapidjson::Document& d) {
  return Error::error_unknown;
}

int SINQAmorSim::ConfigurationParser::parse_string_uri() {
  return Error::error_unknown;
}
