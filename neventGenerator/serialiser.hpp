#pragma once

#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <iterator>
#include <string>
#include <type_traits>

#include "utils.hpp"
#include "Errors.hpp"
#include "schemas/ev42_events_generated.h"

// WARNING: 
// the schema has to match to the serialise temprate type
// this must change

namespace SINQAmorSim {

///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Thu Jul 28 14:32:29 2016
class FlatBufferSerialiser {
public:
  typedef std::true_type is_serialised;
  template<class F>
  std::vector<char> &serialise(const int &message_id,
                               const uint64_t &pulse_time,
                               const std::vector<typename F::value_type>& value={}) {
    auto nev = value.size()/2;
    flatbuffers::FlatBufferBuilder builder;
    auto source_name = builder.CreateString("AMOR.event.stream");
    auto time_of_flight = builder.CreateVector(&value[0], nev);
    auto detector_id = builder.CreateVector(&value[nev], nev);
    auto event = CreateEventMessage(builder, source_name, message_id,
                                    pulse_time, time_of_flight, detector_id);
    FinishEventMessageBuffer(builder, event);
    buffer_.assign(builder.GetBufferPointer(),
                  builder.GetBufferPointer() + builder.GetSize());
    return buffer_;
  }

  char *get() { return &buffer_[0]; }
  const int size() { return buffer_.size(); }

  template<class T>
  void extract(const char *msg, std::vector<T> &data, uint64_t &pid,
               uint64_t &timestamp) {
    auto event = GetEventMessage(static_cast<const void *>(msg));
    data.resize(2 * event->time_of_flight()->size());
    std::copy(event->time_of_flight()->begin(), event->time_of_flight()->end(),
              data.begin());
    std::copy(event->detector_id()->begin(), event->detector_id()->end(),
              data.begin() + event->time_of_flight()->size());

    pid = event->message_id();
    timestamp = event->pulse_time();
    return;
  }

  const std::vector<char>& buffer() { return buffer_; }
private:
  int _size = 0;
  std::vector<char> buffer_;
};

///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Fri Jun 17 12:22:01 2016
class NoSerialiser {
public:
  typedef std::false_type is_serialised;

  NoSerialiser() : buf(nullptr){};
  char *get() { return nullptr; }

  const int size() { return _size; }

private:
  const int _size = 0;
  const uint8_t *buf;
};

} // serialiser
