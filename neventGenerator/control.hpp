#include <atomic>
#include <iostream>
#include <stdlib.h>

//#include "factory.hpp"

namespace control {

const uint32_t default_rate = 10;

struct Control {
  int update() { return stop_; };
  virtual int start(int) { return stop_; };
  const uint32_t &&rate() { return std::move(rate_.load()); };
  bool run() const { return status_ == run_; }
  bool pause() const { return status_ == pause_; }
  bool stop() const { return status_ == stop_; }

protected:
  enum { run_, pause_, stop_ };
  std::atomic<int> status_;
  std::atomic<uint32_t> rate_{default_rate};
};

// using ControlFactory = factory::ObjectFactory<Control, std::string>;

struct NoControl : public Control {
  NoControl() { status_.store(run_); }
  NoControl(const NoControl &) { status_.store(run_); }
  NoControl(const uparam::Param &) { status_.store(run_); }

  int &&update() { return std::move(status_.load()); }
};

struct CommandlineControl : public Control {
  CommandlineControl() { status_.store(run_); }
  CommandlineControl(const CommandlineControl &other) = default;
  //     : status_(other.status_), rate_(other.rate_) {}
  CommandlineControl(uparam::Param &p) {
    status_.store(run_);
    rate_.store(std::stoi(p["rate"]));
  }

  CommandlineControl &operator=(CommandlineControl &other) {
    status_.store(other.status_);
    rate_.store(other.rate_);
    return *this;
  }

  int update() { return update_impl(); }

  int start(int value) {
    if (value >= 0 && value < 3) {
      status_.store(value);
    }
    return run();
  }

private:
  int &&update_impl() {
    std::cout << "run_status : " << status_ << "\t"
              << "transmission_rate : " << std::to_string(rate_) << "\n";
    std::string value;
    while (status_ != stop_) {
      std::cin >> value;
      if (std::string(value) == "run" || std::string(value) == "ru") {
        status_.store(run_);
      }
      if (std::string(value) == "pause" || std::string(value) == "pa") {
        status_.store(pause_);
      }
      if (std::string(value) == "stop" || std::string(value) == "st") {
        status_.store(stop_);
      }
      if (std::string(value) == "rate" || std::string(value) == "ra") {
        std::cout << "Insert the new transmission rate:" << std::endl;
        std::cin >> value;
        rate_.store(std::stoi(value));
        std::cout << value << std::endl;
      }
      std::cout << "status : " << status_ << std::endl;
    }
    std::cout << "status : " << status_ << "\t"
              << "tr : " << std::to_string(rate_) << "\n";
    return std::move(status_.load());
  }
};

} // namespace control
