#pragma once

#include <string>
#include <exception>

namespace SINQAmorSim {
const std::string Err2Str(const int &id);

enum GenericError { error_unknown = -1001 };

enum ConfigurationError {
  error_no_configuration_error = 0,
  error_input_file = -10,
  error_parsing_json = -11,
  error_parsing_command_line = -12,
  error_configuration_invalid = -13
};


  class ConfigurationParsingException: public std::exception {
  public:
    virtual const char* what() const throw() {
      return "Error parsing configuration";
    }
  };
  

}
