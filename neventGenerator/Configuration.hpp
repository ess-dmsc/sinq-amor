#pragma once

#include "Errors.hpp"

#include <string>

namespace SINQAmorSim {
  
  class Configuration {
    using Error = ConfigurationError;
    
  public:
    std::string configuration_file{""};
    std::string produce_broker{""};
    std::string filename{""};
    int multiplier{1};
    int rate{0};
  };
  
  class ConfigurationParser {
    
    int parse_configuration_file(const std::string& input);
    int parse_command_line();
    
  private:
    int parse_configuration_file_impl(rapidjson::Document& d);
    int parse_string_uri(const std::string& uri);
    
    Configuration config;
    
  };

}
