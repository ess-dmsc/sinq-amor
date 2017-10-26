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
  std::string configuration_file{""};
  std::string source{""};
  std::string source_name{"AMOR.area.detector"};
  std::string timestamp_generator{"none"};
  int multiplier{0};
  int rate{0};
  bool valid{true};
};

class ConfigurationParser {
  // Looks into command line if any configuration file is specified. If so
  // generates an initial configuration based on it, else looks for a default
  // config file. If the latter is missing expects that all the relevant
  // informations have been provided as command line argument.
public:
  int parse_configuration_file(const std::string &input);
  int parse_configuration(int argc, char **argv);
  void override_configuration_with(const Configuration &other);
  int validate();
  void print();

  int parse_configuration_file_impl(rapidjson::Document &d);
  Configuration parse_command_line(int argc, char **argv);

  KafkaConfiguration parse_string_uri(const std::string &uri,
                                      const bool use_defaults = false);

  Configuration config;
};
}
