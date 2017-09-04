#include "Configuration.hpp"

#include "rapidjson/istreamwrapper.h"

#include <fstream>
#include <regex>
#include <iostream>
#include <getopt.h>

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
  return parse_configuration_file_impl(d);
}

int SINQAmorSim::ConfigurationParser::parse_configuration_file_impl(
    rapidjson::Document &document) {
  assert(document.IsObject());

  for (auto& m : document.GetObject()) {
    if( m.name.GetString() == std::string("producer_broker")) {
      if(!m.value.IsString()) {
        return ConfigurationError::error_parsing_json;
      }
      config.producer = parse_string_uri(m.value.GetString());      
    }
    if( m.name.GetString() == std::string("multiplier")) {
      if(!m.value.IsInt()) {
        return ConfigurationError::error_parsing_json;
      }
      config.multiplier = m.value.GetInt();
    }
    if( m.name.GetString() == std::string("rate")) {
      if(!m.value.IsInt()) {
        return ConfigurationError::error_parsing_json;
      }
      config.rate = m.value.GetInt();
    }
    if( m.name.GetString() == std::string("source")) {
      if(!m.value.IsString()) {
        return ConfigurationError::error_parsing_json;
      }
      config.source = m.value.GetString();      
    }

  }
  return ConfigurationError::error_no_configuration_error;
}


int SINQAmorSim::ConfigurationParser::parse_command_line(int argc,
                                                         char** argv) {

  if(argc == 1) {
    return ConfigurationError::error_no_configuration_error;
  }


  
  return ConfigurationError::error_unknown;
}

