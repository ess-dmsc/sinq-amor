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

using milliseconds = std::chrono::milliseconds;
using nanoseconds = std::chrono::nanoseconds;

/*! \class Generator
 *  \author Michele Brambilla <mib.mic@gmail.com>
 *  \date Wed Jun 08 15:19:52 2016 */
template <typename Streamer, typename Control, typename Serialiser>
class Generator {
  using self_t = Generator<Streamer, Control, Serialiser>;

public:
  Generator(SINQAmorSim::Configuration &configuration)
      : Config(configuration), Streaming{new Control(configuration)} {

    SINQAmorSim::KafkaOptions Options;
    Stream.reset(new Streamer{Config.producer.broker, Config.producer.topic,
                              Config.source_name, Config.options});
    if (!Stream) {
      throw std::runtime_error("Error creating the stream instance");
    }
    if (!Streaming) {
      throw std::runtime_error("Error creating the control instance");
    }
  }

  template <class T> void run(std::vector<T> &EventsData) {
    std::thread ts(&self_t::runImpl<T>, this, std::ref(EventsData));
    Streaming->update();
    ts.join();
  }

  template <class T> void listen(std::vector<T> &EventsData) {
    std::thread tr(&self_t::listenImpl<T>, this, std::ref(EventsData));
    tr.join();
  }

private:
  std::unique_ptr<Streamer> Stream{nullptr};
  std::unique_ptr<Control> Streaming{nullptr};
  SINQAmorSim::Configuration Config;

  template <class T> void runImpl(std::vector<T> &Events) {
    using namespace std::chrono;
    uint64_t PulseID = 0;

    using system_clock = std::chrono::system_clock;
    auto StartTime = system_clock::now();
    std::time_t to_time = system_clock::to_time_t(StartTime);
    auto Timeout = std::localtime(&to_time);

    nanoseconds PulseTime =
        duration_cast<nanoseconds>(system_clock::now().time_since_epoch());
    // Pregenerate serialised buffer: this prevents flatbuffers to be a possible
    // bottleneck
    Stream->serialiseAndStore(PulseID, PulseTime, Events, Events.size());
    while (!Streaming->exit()) {
      if (Streaming->stop()) {
        std::this_thread::sleep_for(milliseconds(100));
        continue;
      }

      if (Streaming->run()) {
        Stream->sendExistingBuffer();
      } else {
        Stream->send(PulseID, PulseTime, Events, 0);
      }
      ++PulseID;
      auto ElapsedTime = system_clock::now() - StartTime;
      if (std::chrono::duration_cast<std::chrono::seconds>(ElapsedTime)
              .count() > Config.report_time) {
        int NumMessages = Stream->poll(-1);
        std::cout << "Sent " << NumMessages 
		  << "/" << Stream->getNumMessages()
		  << " packets @ "
                  // << 1e3 * NumMessages * Stream->bufferSizeMbytes() /
                  //        std::chrono::duration_cast<std::chrono::milliseconds>(
                  //            ElapsedTime)
                  //            .count()
		  // << "/" 
		  << 1e3 * Stream->getMbytes()/
                         std::chrono::duration_cast<std::chrono::milliseconds>(
                             ElapsedTime)
                             .count() << " "
                  << "MB/s"
                  << "\t(timestamp : " << PulseTime.count() << ")" << std::endl;
        Stream->getNumMessages() = 0;
        Stream->getMbytes() = 0;
        StartTime = system_clock::now();
      }
    }
  }

  template <class T> void listenImpl(std::vector<T> &Events) {

    int PulseID = -1, MessagesLost = -1;
    uint64_t PacketID;
    uint64_t ReceivedBytes = 0;
    int MessagesReceived = 0;

    using system_clock = std::chrono::system_clock;
    auto StartTime = system_clock::now();

    while (1) {

      auto Message = Stream->recv(Events);
      PacketID = Message.first;
      if (PacketID - PulseID != 0) {
        PulseID = PacketID;
        ++MessagesLost;
      } else {
        ++MessagesReceived;
        ReceivedBytes += Message.second;
      }
      if (std::chrono::duration_cast<std::chrono::seconds>(system_clock::now() -
                                                           StartTime)
              .count() > Config.report_time) {
        std::cout << "Missed " << MessagesLost << " packets" << std::endl;
        std::cout << "Received " << MessagesReceived << "packets"
                  << " @ " << ReceivedBytes * 1e-1 * 1e-6 << "MB/s"
                  << std::endl;
        MessagesReceived = 0;
        MessagesLost = 0;
        ReceivedBytes = 0;
        StartTime = system_clock::now();
      }
      PulseID++;
    }
  }
};
