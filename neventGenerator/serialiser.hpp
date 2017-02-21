#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <inttypes.h>
#include <type_traits>

#include "hardware.hpp"
#include "amo0_psi_sinq_schema_generated.h"

namespace serialiser {

///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Thu Jul 28 14:32:29 2016
  template<typename T>
  struct FlatBufSerialiser {
    typedef std::true_type is_serialised;
    FlatBufSerialiser() { }
    FlatBufSerialiser(const FlatBufSerialiser& other) { }

    FlatBufSerialiser(hws::HWstatus& hwstat, T* val = NULL, int nev = 0, const T timestamp=0) {
      int16_t* stat = new int16_t[10];
      auto htype = builder.CreateString("AMOR.event.stream");
      auto data = builder.CreateVector(val,nev);
      auto event = BrightnESS::EventGenerator::FlatBufs::AMOR::CreateEvent(builder,
									   htype,
									   timestamp,
									   hwstat.system_time,
									   hwstat.pid,
									   data);  
      builder.Finish(event);
      _size = builder.GetSize();
    }

    char* get() { return reinterpret_cast<char*>(builder.GetBufferPointer()); }
    const int size() { return _size; }

    void extract(const char* msg,
                 std::vector<T>& data,
                 hws::HWstatus& hwstat) {
      auto event = BrightnESS::EventGenerator::FlatBufs::AMOR::GetEvent(reinterpret_cast<const void*>(msg));

      data.resize(event->data()->size());
      std::copy(event->data()->begin(),event->data()->end(),data.begin());
      hwstat.pid = event->pid();
      return;
    }

  private:
    flatbuffers::FlatBufferBuilder builder;
    int _size = 0;
    uint8_t *buf;

  };


///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Thu Aug 04 09:34:56 2016
  template<typename T>
  struct jSONSerialiser {
    typedef std::true_type is_serialised;
    
    jSONSerialiser(hws::HWstatus&, T* = NULL, int =0, const T =0) { }
    char* get() { return reinterpret_cast<char*>(NULL); }
    
    const int size() { return _size; }

  private:
    const int _size = 0;
    const uint8_t *buf;

  };



///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Fri Jun 17 12:22:01 2016
  template<typename T>
  struct NoSerialiser {
    typedef std::false_type is_serialised;
    
    NoSerialiser(hws::HWstatus&, T* = NULL, int =0, const T =0) { }
    char* get() { return reinterpret_cast<char*>(NULL); }
    
    const int size() { return _size; }

  private:
    const int _size = 0;
    const uint8_t *buf;

  };

  



} // serialiser
