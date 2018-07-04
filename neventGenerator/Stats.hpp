#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

struct Stats {

  using system_clock = std::chrono::system_clock;

public:
  ~Stats();

  void setNumThreads(const int);
  void add(const int, const int, const int);
  void report();

private:
  uint64_t getCurrentTimestamp();

  std::vector<int> NumMessages;
  std::vector<int> MBytes;
  std::mutex CountGuard;
  std::condition_variable WaitUntilReady;
  std::atomic<int> ThreadCount{0};
  bool Ready{false};
  bool Run{true};
};
