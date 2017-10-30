#pragma once

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <thread>

#include <assert.h>

#include <stdlib.h>
#include <time.h>

#include "Configuration.hpp"

#if HAVE_ZMQ
#include "zmq_generator.hpp"
#endif

#include "file_writer.hpp"
#include "kafka_generator.hpp"

#include "control.hpp"
#include "timestamp_generator.hpp"

using nanoseconds = std::chrono::nanoseconds;

/*! \struct Generator
 *
 * The ``Generator`` send an event stream via the network using a templated
 * protocol. The constructor receives a set of key-values and (optionally) a
 * multiplier factor (unuseful so far). A header template is read from
 * "header.in" and regularly modified to account for any change (number of
 * events, hw status,...). To start the streaming use the method ``run``. It
 * keeps sending data at a fixed frequency until a *pause* or *stop* control
 * command is sent. Every 10s returns statistics and check for control
 * parameters.
 *
 * @tparam Streamer policy for stremer protocol (Kafka, 0MQ, ...)
 * @tparam Header policy for creating the header (jSON, ...)
 * @tparam Control policy to start, pause and stop the generator (plain text) -
 *TODO
 *
 *  \author Michele Brambilla <mib.mic@gmail.com>
 *  \date Wed Jun 08 15:19:52 2016 */
template <typename Streamer, typename Control, typename Serialiser>
class Generator {
  using self_t = Generator<Streamer, Control, Serialiser>;

public:
  Generator(SINQAmorSim::Configuration &configuration)
      : config(configuration), control{new Control(configuration)} {

    SINQAmorSim::GeneratorOptions options;
    options.push_back(SINQAmorSim::GeneratorOptions::value_type{
        "source_name", configuration.source_name});
    streamer.reset(new Streamer{configuration.producer.broker,
                                configuration.producer.topic, options});
    if (!streamer) {
      throw std::runtime_error("Error creating the streamer instance");
    }
    if (!control) {
      throw std::runtime_error("Error creating the control instance");
    }
  }

  template <class T> void run(std::vector<T> &stream) {
    std::thread ts(&self_t::run_impl<T>, this, std::ref(stream));
    control->update();
    ts.join();
  }

  template <class T> void listen(std::vector<T> &stream) {
    std::thread tr(&self_t::listen_impl<T>, this, std::ref(stream));
    tr.join();
  }

private:
  std::unique_ptr<Streamer> streamer{nullptr};
  std::unique_ptr<Control> control{nullptr};
  bool initial_status;
  SINQAmorSim::Configuration config;

  template <class T> void run_impl(std::vector<T> &stream) {
    using namespace std::chrono;
    int nev = stream.size();
    uint64_t pulseID = 0;
    int count = 0;
    control->start(initial_status);

    using std::chrono::system_clock;
    auto start = system_clock::now();
    std::time_t to_time = system_clock::to_time_t(start);
    auto timeout = std::localtime(&to_time);

    while (!control->stop()) {
      nanoseconds ns =
          duration_cast<nanoseconds>(system_clock::now().time_since_epoch());
      auto timestamp = ns.count();
      generate_timestamp(stream, config.rate, timestamp,
                         config.timestamp_generator);

      if (control->run()) {
        streamer->send(pulseID, timestamp, stream, stream.size());
        ++count;
      } else {
        streamer->send(pulseID, timestamp, stream, 0);
      }
      ++pulseID;
      if (pulseID % control->rate() == 0) {
        ++timeout->tm_sec;
        std::this_thread::sleep_until(
            system_clock::from_time_t(mktime(timeout)));
        streamer->poll(1);
      }

      auto from_start = system_clock::now() - start;
      if (std::chrono::duration_cast<std::chrono::seconds>(from_start).count() >
          config.report_time) {
        std::cout << "Sent " << streamer->messages() << "/"
                  << control->rate() * config.report_time << " packets @ "
                  << 1e3 * streamer->Mbytes() /
                         std::chrono::duration_cast<std::chrono::milliseconds>(
                             from_start)
                             .count()
                  << "MB/s"
                  << "\t(timestamp : " << timestamp << ")" << std::endl;
        streamer->messages() = 0;
        streamer->Mbytes() = 0;
        count = 0;
        start = system_clock::now();
      }
    }
  }

  template <class T> void listen_impl(std::vector<T> &stream) {

    int pulseID = -1, missed = -1;
    uint64_t pid;
    uint64_t size = 0;
    int count = 0, nev, maxsize = 0, len;
    int recvmore;
    uint32_t rate = 0;

    using std::chrono::system_clock;
    auto start = system_clock::now();

    while (1) {

      auto msg = streamer->recv(stream);
      pid = msg.first;
      if (pid - pulseID != 0) {
        pulseID = pid;
        ++missed;
      } else {
        ++count;
        size += msg.second;
      }
      if (std::chrono::duration_cast<std::chrono::seconds>(system_clock::now() -
                                                           start)
              .count() > config.report_time) {
        std::cout << "Missed " << missed << " packets" << std::endl;
        std::cout << "Received " << count << "packets"
                  << " @ " << size * 1e-1 * 1e-6 << "MB/s" << std::endl;
        count = 0;
        missed = 0;
        size = 0;
        start = system_clock::now();
      }
      pulseID++;
    }
  }
};
