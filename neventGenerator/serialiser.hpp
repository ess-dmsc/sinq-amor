#pragma once

#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <iterator>
#include <string>
#include <type_traits>

#include "Errors.hpp"
#include "schemas/ev42_events_generated.h"
#include "utils.hpp"

// WARNING:
// the schema has to match to the serialise template type
// this must change

namespace SINQAmorSim {

///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Thu Jul 28 14:32:29 2016
class FlatBufferSerialiser {
public:
  using is_serialised = std::true_type;

  // WARNING: template parameter has to match schema data type
  template <class T>
  const size_t &serialise(const int &message_id, const uint64_t &pulse_time,
                          const std::vector<T> &message = {}) {
    auto nev = message.size() / 2;
    builder.Clear();
    auto source_name = builder.CreateString("AMOR.event.stream");
    auto time_of_flight = builder.CreateVector(&message[0], nev);
    auto detector_id = builder.CreateVector(&message[nev], nev);
    auto event = CreateEventMessage(builder, source_name, message_id,
                                    pulse_time, time_of_flight, detector_id);
    FinishEventMessageBuffer(builder, event);
    // buffer_.resize(builder.GetSize());
    // buffer_.assign(builder.GetBufferPointer(),
    //                builder.GetBufferPointer() + builder.GetSize());
    return (size_ = builder.GetSize());
  }

  template <class T>
  void extract(const std::vector<char> &message, std::vector<T> &data,
               uint64_t &pid, uint64_t &timestamp) {
    extract_impl<T>(static_cast<const void *>(&message[0]), data, pid,
                    timestamp);
  }

  template <class T>
  void extract(const char *msg, std::vector<T> &data, uint64_t &pid,
               uint64_t &timestamp) {
    extract_impl<T>(static_cast<const void *>(msg), data, pid, timestamp);
  }

  unsigned char *get() { return builder.GetBufferPointer(); }
  const size_t size() { return builder.GetSize(); }

  bool verify() {
    if (builder.GetSize() > 0) {
      flatbuffers::Verifier verifier(builder.GetBufferPointer(),
                                     builder.GetSize());
      return VerifyEventMessageBuffer(verifier);
    }
    return false;
  }
  bool verify(const std::vector<char> &other) {
    auto p = const_cast<const char *>(&other[0]);
    flatbuffers::Verifier verifier(reinterpret_cast<const unsigned char *>(p),
                                   other.size());
    return VerifyEventMessageBuffer(verifier);
  }

private:
  flatbuffers::FlatBufferBuilder builder;

  template <class T>
  void extract_impl(const void *msg, std::vector<T> &data, uint64_t &pid,
                    uint64_t &timestamp) {
    auto event = GetEventMessage(msg);
    data.resize(2 * event->time_of_flight()->size());
    std::copy(event->time_of_flight()->begin(), event->time_of_flight()->end(),
              data.begin());
    std::copy(event->detector_id()->begin(), event->detector_id()->end(),
              data.begin() + event->time_of_flight()->size());
    pid = event->message_id();
    timestamp = event->pulse_time();
  }

  size_t size_{0};
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

} // namespace SINQAmorSim
