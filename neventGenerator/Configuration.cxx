#include "Configuration.hpp"

#include "rapidjson/istreamwrapper.h"

#include <fstream>
#include <regex>

std::string get_protocol(const std::string &s, const std::string &deft = "") {
  std::smatch m;
  if (std::regex_search(s, m, std::regex("^[A-Za-z]+"))) {
    return std::move(std::string(m[0]));
  }
  return std::move(deft);
}
std::string get_broker(const std::string &s, const std::string &deft = "") {
  std::smatch m;
  if (std::regex_search(s, m, std::regex("//[A-Za-z\\d.]+")))
    return std::move(std::string(m[0]).substr(2));
  return std::move(deft);
}
std::string get_port(const std::string &s, const std::string &deft = "") {
  std::smatch m;
  if (std::regex_search(s, m, std::regex(":\\d+/"))) {
    std::string result(m[0]);
    return std::move(result.substr(1, result.length() - 2));
  }
  return std::move(deft);
}
std::string get_topic(const std::string &s, const std::string &deft = "") {
  std::smatch m;
  if (std::regex_search(s, m, std::regex("/[A-Za-z0-9-_:.]*$")))
    return std::move(std::string(m[0]).erase(0,1));
  return std::move(deft);
}

SINQAmorSim::KafkaConfiguration SINQAmorSim::ConfigurationParser::parse_string_uri(const std::string &uri,const bool use_defaults) {
  KafkaConfiguration configuration;
  if(use_defaults) {
    configuration.broker = get_broker(uri, "localhost") + ":" + get_port(uri, "9092");
    configuration.topic = get_topic(uri, "empty-topic");
  }
  else {
    auto broker = get_broker(uri);
    auto port =  get_port(uri);
    if(broker!="" && port != "") {
      configuration.broker =  broker + ":" + port;
    }
    configuration.topic = get_topic(uri);
  }
  return configuration;
}

int SINQAmorSim::ConfigurationParser::parse_configuration_file(
    const std::string &input) {
  std::ifstream ifs(input);
  if( !ifs.good()) {
    return ConfigurationError::error_input_file;
  }
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document d;
  d.ParseStream(isw);
  if(d.HasParseError () ) {
    return d.GetParseError();
  }
  parse_configuration_file_impl(d);
  return ConfigurationError::error_no_configuration_error;
}

int SINQAmorSim::ConfigurationParser::parse_configuration_file_impl(
    rapidjson::Document &d) {

  
  return ConfigurationError::error_unknown;
}


int SINQAmorSim::ConfigurationParser::parse_command_line(int argc,
                                                         char** argv) {
  return ConfigurationError::error_unknown;
}

