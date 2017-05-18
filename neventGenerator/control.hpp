#include <iostream>
#include <stdlib.h>

#include "factory.hpp"

namespace control {

const uint32_t default_rate = 10;

struct Control {
  int update() {
    return stop_;
  };
  virtual int start(int) {
    return stop_;
  };
  const uint32_t &rate() {
    return rate_;
  };
  bool run() const { return status_ == run_; }
  bool pause() const { return status_ == pause_; }
  bool stop() const { return status_ == stop_; }

protected:
  enum {
    run_,
    pause_,
    stop_
  };
  int status_;
  uint32_t rate_ = default_rate;
};

using ControlFactory = factory::ObjectFactory<Control, std::string>;

struct NoControl : public Control {
  NoControl() { status_ = run_; }
  NoControl(const NoControl &) { status_ = run_; }
  NoControl(const uparam::Param &) { status_ = run_; }

  int update() { return status_; }
};

struct CommandlineControl : public Control {
  CommandlineControl() { status_ = run_; }
  CommandlineControl(const CommandlineControl &other) {
    status_ = other.status_;
    rate_ = other.rate_;
  }
  CommandlineControl(uparam::Param &p) {
    status_ = run_;
    rate_ = std::stoi(p["rate"]);
  }

  CommandlineControl &operator=(CommandlineControl &other) {
    status_ = other.status_;
    rate_ = other.rate_;
    return *this;
  }

  int update() { return update_impl(); }

  int start(int value) {
    if (value >= 0 && value < 3) {
      status_ = value;
    }
    return run();
  }

private:
  int update_impl() {
    std::string value;
    while (status_ != stop_) {
      std::cin >> value;
      if (std::string(value) == "run" || std::string(value) == "ru")
        status_ = run_;
      if (std::string(value) == "pause" || std::string(value) == "pa")
        status_ = pause_;
      if (std::string(value) == "stop" || std::string(value) == "st")
        status_ = stop_;
      if (std::string(value) == "rate" || std::string(value) == "ra") {
        std::cout << "Insert the new transmission rate:" << std::endl;
        std::cin >> value;
        rate_ = std::stoi(value);
      }
      std::cout << "status : " << status_ << std::endl;
    }
    return status_;
  }
};

} // control
