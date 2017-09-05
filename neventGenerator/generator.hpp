#ifndef _GENERATOR_H
#define _GENERATOR_H

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

#if HAVE_ZMQ
#include "zmq_generator.hpp"
#endif

#include "file_writer.hpp"
#include "kafka_generator.hpp"

#include "control.hpp"

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
struct Generator {
  typedef Generator<Streamer, Control, Serialiser> self_t;

  Generator(const std::string &broker, const std::string &topic)
      : streamer(broker, topic), control{new Control()} {}

  template <class T> void run(T *stream, int nev = 0) {
    std::thread ts(&self_t::run_impl<T>, this, stream, nev);
    control->update();
    ts.join();
  }

  template <class T> void listen(std::vector<T> stream, int nev = 0) {
    std::thread tr(&self_t::listen_impl<T>, this, stream);
    tr.join();
    listen_impl<T>(stream);
  }

private:
  Streamer streamer;
  std::unique_ptr<Control> control;
  Serialiser serialiser;
  bool initial_status;
  std::default_random_engine engine;

  template <class T>
  void generate_timestamp(T *first, T *last, const uint32_t pulse_lag) {
    std::uniform_int_distribution<T> distr(0, pulse_lag);
    auto rng = std::bind(distr, engine);
    std::generate(first, last, rng);
  }

  template <class T> void run_impl(T *stream, int nev = 0) {
    using namespace std::chrono;
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
      if (control->run()) {
        streamer.send(pulseID, timestamp, stream, nev, serialiser);
        ++count;
      } else {
        streamer.send(pulseID, timestamp, stream, 0, serialiser);
      }
      ++pulseID;
      if (pulseID % control->rate() == 0) {
        ++timeout->tm_sec;
        std::this_thread::sleep_until(
            system_clock::from_time_t(mktime(timeout)));
        generate_timestamp(stream, stream + nev, floor(1e9 / control->rate()));
      }

      if (std::chrono::duration_cast<std::chrono::seconds>(system_clock::now() -
                                                           start)
              .count() > 10) {
        std::cout << "Sent " << count << " packets @ "
                  << count * nev * sizeof(T) / (10 * 1e6) << "MB/s"
                  << "\t(timestamp : " << timestamp << ")" << std::endl;
        count = 0;
        start = system_clock::now();
      }
    }
  }

  template <class T> void listen_impl(std::vector<T> stream) {

    int pulseID = -1, missed = -1;
    uint64_t pid;
    uint64_t size = 0;
    int count = 0, nev, maxsize = 0, len;
    int recvmore;
    uint32_t rate = 0;

    using std::chrono::system_clock;
    auto start = system_clock::now();

    while (1) {

      auto msg = streamer.recv(stream, Serialiser());
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
              .count() > 10) {
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

#endif // GENERATOR_H
