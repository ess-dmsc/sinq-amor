#include <atomic>
#include <iostream>
#include <stdlib.h>

#include "Configuration.hpp"

namespace SINQAmorSim {

enum class RunStatus { run, pause, stop };

struct NoControl {
  NoControl() {}
  NoControl(Configuration &) {}

  NoControl(const NoControl &other) = default;

  NoControl &operator=(NoControl &other) = default;

  int update() { return -1; }

  int start(int) { return run(); }
  bool run() const { return true; }
  bool stop() const { return false; }
  bool pause() const { return false; }
};

struct CommandlineControl {
  CommandlineControl(Configuration &configuration) : config{configuration} {
    status.store(int(RunStatus::run));
  }
  CommandlineControl(const CommandlineControl &other) = default;

  CommandlineControl &operator=(CommandlineControl &other) {
    status.store(other.status);
    config.rate = other.config.rate;
    return *this;
  }

  int update() { return update_impl(); }

  int start(int value) {
    if (value >= 0 && value < 3) {
      status.store(value);
    }
    return run();
  }
  bool run() const { return status == int(RunStatus::run); }
  bool stop() const { return status == int(RunStatus::stop); }
  bool pause() const { return status == int(RunStatus::pause); }
  int rate() const { return config.rate; }

private:
  std::atomic<int> status;
  SINQAmorSim::Configuration &config;

  int &&update_impl() {
    std::cout << "run_status : " << status << "\t"
              << "transmission_rate : " << std::to_string(config.rate) << "\n";
    std::string value;
    while (status != int(RunStatus::stop)) {
      std::cin >> value;
      if (std::string(value) == "run" || std::string(value) == "ru") {
        status.store(int(RunStatus::run));
      }
      if (std::string(value) == "pause" || std::string(value) == "pa") {
        status.store(int(RunStatus::pause));
      }
      if (std::string(value) == "stop" || std::string(value) == "st") {
        status.store(int(RunStatus::stop));
      }
      if (std::string(value) == "rate" || std::string(value) == "ra") {
        std::cout << "Insert the new transmission rate:" << std::endl;
        std::cin >> value;
        config.rate = std::stoi(value);
      }
    }
    std::cout << "status : " << status << "\t"
              << "tr : " << std::to_string(config.rate) << "\n";
    return std::move(status.load());
  }
};

} // namespace control
