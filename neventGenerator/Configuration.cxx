#include "Configuration.hpp"

#include "rapidjson/istreamwrapper.h"

#include <fstream>
#include <getopt.h>
#include <iostream>
#include <regex>
#include <sstream>

std::string get_protocol(const std::string &s, const std::string &deft = "") {
  std::smatch m;
  if (std::regex_search(s, m, std::regex("^[A-Za-z]+"))) {
    return std::move(std::string(m[0]));
  }
  return std::move(deft);
}

std::string get_broker_topic(const std::string &s, const std::string &deft = "") {
  std::smatch m;
  if (std::regex_search(s, m, std::regex("//[A-Za-z0-9.,:]+/"))) {
    auto result = std::string(m[0]).substr(2);
    result.pop_back();
    return std::move(result);
  }
  return std::move(deft);
}

std::string get_topic(const std::string &s, const std::string &deft = "") {
  std::smatch m;
  if (std::regex_search(s, m, std::regex("/[A-Za-z0-9-_:.]*$")))
    return std::move(std::string(m[0]).erase(0, 1));
  return std::move(deft);
}

bool broker_topic_is_valid(const std::string& broker, const std::string& topic) {
  if(topic.empty() ||
     broker.empty() ||
     broker.front() == ':' ||
     broker.back() == ':' ) {
    return false;
  }
  if(broker.find(":,") != std::string::npos  ||
     broker.find(",:") != std::string::npos ) {
    return false;
  }

  return true;
}

SINQAmorSim::KafkaConfiguration
SINQAmorSim::ConfigurationParser::parse_string_uri(const std::string &uri,
                                                   const bool use_defaults) {
  KafkaConfiguration configuration;
  if (use_defaults) {
    configuration.broker = get_broker_topic(uri,"localhost:9092");
    configuration.topic = get_topic(uri, "empty-topic");
  } else {
    configuration.broker = get_broker_topic(uri);
    configuration.topic = get_topic(uri);
    if(!broker_topic_is_valid(configuration.broker,configuration.topic) ) {
      throw ConfigurationParsingException();
    }
  }
  return configuration;
}

int SINQAmorSim::ConfigurationParser::parse_configuration_file(
    const std::string &input) {
  std::ifstream ifs(input);
  if (!ifs.good()) {
    return ConfigurationError::error_input_file;
  }
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document d;
  d.ParseStream(isw);
  if (d.HasParseError()) {
    return d.GetParseError();
  }

  return parse_configuration_file_impl(d);
}

int SINQAmorSim::ConfigurationParser::parse_configuration_file_impl(
    rapidjson::Document &document) {
  assert(document.IsObject());

  for (auto &m : document.GetObject()) {
    if (m.name.GetString() == std::string("producer_broker")) {
      if (!m.value.IsString()) {
        return ConfigurationError::error_parsing_json;
      }
      config.producer = parse_string_uri(m.value.GetString());
    }
    if (m.name.GetString() == std::string("multiplier")) {
      if (!m.value.IsInt()) {
        return ConfigurationError::error_parsing_json;
      }
      config.multiplier = m.value.GetInt();
    }
    if (m.name.GetString() == std::string("rate")) {
      if (!m.value.IsInt()) {
        return ConfigurationError::error_parsing_json;
      }
      config.rate = m.value.GetInt();
    }
    if (m.name.GetString() == std::string("source")) {
      if (!m.value.IsString()) {
        return ConfigurationError::error_parsing_json;
      }
      config.source = m.value.GetString();
    }
    if (m.name.GetString() == std::string("timestamp_generator")) {
      if (!m.value.IsString()) {
        return ConfigurationError::error_parsing_json;
      }
      config.timestamp_generator = m.value.GetString();
    }
  }
  return ConfigurationError::error_no_configuration_error;
}

int SINQAmorSim::ConfigurationParser::parse_configuration(int argc,
                                                          char **argv) {
  int error = ConfigurationError::error_no_configuration_error;
  Configuration command_line_config;
  if (argc > 1) {
    command_line_config = parse_command_line(argc, argv);
  }

  try {
    if (command_line_config.configuration_file.empty()) {
      error = parse_configuration_file("config.json");
    } else {
      error = parse_configuration_file(command_line_config.configuration_file);
    }
  } catch (const ConfigurationParsingException& e) {
    std::cout << e.what() << "\n";
  }
  
  if (error == ConfigurationError::error_no_configuration_error) {
    override_configuration_with(command_line_config);
    return validate();
  } else {
    return error;
  }
}

void usage(const std::string &executable);

SINQAmorSim::Configuration
SINQAmorSim::ConfigurationParser::parse_command_line(int argc, char **argv) {
  SINQAmorSim::Configuration result;
  result.timestamp_generator = "";
  
  static struct option long_options[] = {
      {"help", no_argument, nullptr, 'h'},
      {"config-file", required_argument, nullptr, 0},
      {"producer-uri", required_argument, nullptr, 0},
      {"status-uri", required_argument, nullptr, 0},
      {"use-signal-handler", required_argument, nullptr, 0},
      {"source", required_argument, nullptr, 0},
      {"multiplier", required_argument, nullptr, 0},
      {"rate", required_argument, nullptr, 0},
      {"timestamp-generator", required_argument, nullptr, 0},      
      {nullptr, 0, nullptr, 0},
  };
  std::string cmd;
  int option_index = 0;
  bool getopt_error = false;
  while (true) {
    int c = getopt_long(argc, argv, "h", long_options, &option_index);
    if (c == -1)
      break;
    if (c == '?') {
      getopt_error = true;
    }
    switch (c) {
    case 'h':
      usage(std::string(argv[0]));
      break;
    case 0:
      auto lname = long_options[option_index].name;
      if (std::string("help") == lname) {
      }
      if (std::string("config-file") == lname) {
        result.configuration_file = optarg;
      }
      if (std::string("producer-uri") == lname) {
        result.producer = parse_string_uri(optarg,true);
      }
      if (std::string("source") == lname) {
        result.source = optarg;
      }
      if (std::string("multiplier") == lname) {
        std::istringstream buffer(optarg);
        buffer >> result.multiplier;
      }
      if (std::string("rate") == lname) {
        std::istringstream buffer(optarg);
        buffer >> result.rate;
      }
      if (std::string("timestamp-generator") == lname) {
        result.timestamp_generator = optarg;
      }
      break;
    }
  }

  if (getopt_error) {
    result.valid = false;
    std::cerr << "ERROR parsing command line options\n";
  }

  return std::move(result);
}

void SINQAmorSim::ConfigurationParser::override_configuration_with(
    const SINQAmorSim::Configuration &other) {

  if (!other.producer.broker.empty()) {
    config.producer.broker = other.producer.broker;
  }
  if (!other.producer.topic.empty()) {
    config.producer.topic = other.producer.topic;
  }
  if (!other.configuration_file.empty()) {
    config.configuration_file = other.configuration_file;
  }
  if (!other.source.empty()) {
    config.source = other.source;
  }
  if (other.multiplier > 0) {
    config.multiplier = other.multiplier;
  }
  if (other.rate > 0) {
    config.rate = other.rate;
  }
  if (!other.timestamp_generator.empty()) {
    config.timestamp_generator = other.timestamp_generator;
  }
}

int SINQAmorSim::ConfigurationParser::validate() {
  if (config.producer.broker.empty() || config.producer.topic.empty() ||
      config.source.empty() || config.multiplier <= 0 || config.rate <= 0) {
    return ConfigurationError::error_configuration_invalid;
  } else {
    return ConfigurationError::error_no_configuration_error;
  }
}

void SINQAmorSim::ConfigurationParser::print() {
  std::cout << "producer:\n"
            << "\tbroker: " << config.producer.broker << "\n"
            << "\ttopic: " << config.producer.topic << "\n"
            << "source: " << config.source << "\n"
            << "multiplier: " << config.multiplier << "\n"
            << "rate: " << config.rate << "\n"
            << "\n";
}

void usage(const std::string &exe) {
  std::cout << "Usage: " << exe << " [OPTIONS]\n";
  std::cout << "\t--config-file: "
            << "\n";
  std::cout << "\t--producer-uri: "
            << "\n";
  std::cout << "\t--source: "
            << "\n";
  std::cout << "\t--multiplier: "
            << "\n";
  std::cout << "\t--rate: "
            << "\n";
}
