#pragma once

#include "Errors.hpp"

#include "rapidjson/document.h"

#include <string>

namespace SINQAmorSim {

class KafkaConfiguration {
public:
  std::string broker{""};
  std::string topic{""};
};

class Configuration {

public:
  KafkaConfiguration producer;
  //  std::string protocol{""};
  std::string configuration_file{""};
  std::string source{""};
  int multiplier{1};
  int rate{0};
};

class ConfigurationParser {

public:
  int parse_configuration_file(const std::string &input);
  int parse_command_line(int argc, char** argv);

  int parse_configuration_file_impl(rapidjson::Document &d);
  KafkaConfiguration parse_string_uri(const std::string &uri,
                                      const bool use_defaults=false);

  Configuration config;
};
}
