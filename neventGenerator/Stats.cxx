#include "Stats.hpp"

#include <iostream>
#include <numeric>

#include <nlohmann/json.hpp>

Stats::~Stats() { Run = false; }
uint64_t Stats::getCurrentTimestamp() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

void Stats::setNumThreads(const int NumThreads) {
  NumMessages.resize(NumThreads);
  MBytes.resize(NumThreads);
}

void Stats::add(const int Messages, const int MB, const int ThreadId) {
  NumMessages[ThreadId] += Messages;
  MBytes[ThreadId] += MB;
  std::lock_guard<std::mutex> Lock(CountGuard);
  ThreadCount++;
  if (ThreadCount == NumMessages.size()) {
    WaitUntilReady.notify_all();
    ThreadCount = 0;
  }
}
void Stats::report() {

  while (Run) {
    std::chrono::time_point<std::chrono::system_clock> StartTime =
        system_clock::now();
    std::unique_lock<std::mutex> Lock(CountGuard);
    WaitUntilReady.wait(Lock);
    std::chrono::time_point<std::chrono::system_clock> Now =
        system_clock::now();

    int Messages = std::accumulate(NumMessages.begin(), NumMessages.end(), 0);
    int MB = std::accumulate(MBytes.begin(), MBytes.end(), 0);

    std::fill(NumMessages.begin(), NumMessages.end(), 0);
    std::fill(MBytes.begin(), MBytes.end(), 0);

    nlohmann::json Message;
    Message["packets"] = Messages;
    Message["MB"] = MB;
    Message["MB/s"] =
        1e3 * MB /
        std::chrono::duration_cast<std::chrono::milliseconds>(Now - StartTime)
            .count();
    Message["timestamp"] = getCurrentTimestamp();
    Message["num_threads"] = MBytes.size();
    std::cout << Message.dump(4) << "\n";
    // std::cout << "Sent " << Messages << " packets @ "
    //           << 1e3 * MB /
    //                  std::chrono::duration_cast<std::chrono::milliseconds>(
    //                      Now - StartTime)
    //                      .count()
    //           << "MB/s"
    //           << "\t(timestamp : " < < < <
    // ")" << std::endl;
    StartTime = Now;
  }
}
