#pragma once

#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <iterator>
#include <string>
#include <type_traits>

#include "schemas/ev42_events_generated.h"

namespace serialiser {

///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Thu Jul 28 14:32:29 2016
template <typename T> struct FlatBufSerialiser {
  typedef std::true_type is_serialised;
  FlatBufSerialiser() {}

  std::vector<char> &serialise(const int &message_id,
                               const uint64_t &pulse_time, T *value = NULL,
                               int nev = 0) {

    flatbuffers::FlatBufferBuilder builder;
    auto source_name = builder.CreateString("AMOR.event.stream");
    auto time_of_flight = builder.CreateVector(value, nev);
    auto detector_id = builder.CreateVector(value + nev, nev);
    auto event = CreateEventMessage(builder, source_name, message_id,
                                    pulse_time, time_of_flight, detector_id);
    FinishEventMessageBuffer(builder, event);
    buffer.assign(builder.GetBufferPointer(),
                  builder.GetBufferPointer() + builder.GetSize());
    return buffer;
  }

  char *get() { return &buffer[0]; }
  const int size() { return buffer.size(); }

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

private:
  int _size = 0;
  std::vector<char> buffer;
};

///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Fri Jun 17 12:22:01 2016
template <typename T> struct NoSerialiser {
  typedef std::false_type is_serialised;

  NoSerialiser() : buf(nullptr){};
  char *get() { return nullptr; }

  const int size() { return _size; }

private:
  const int _size = 0;
  const uint8_t *buf;
};

} // serialiser
