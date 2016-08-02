#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <inttypes.h>
#include <type_traits>

#include "hardware.hpp"

#include "psi_sinq_schema.hpp"

///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Thu Jul 28 14:32:29 2016

namespace serialiser {

  template<typename T>
  struct FlatBufSerialiser {
    typedef std::true_type is_serialised;
    FlatBufSerialiser() { }
    FlatBufSerialiser(const FlatBufSerialiser& other) { }

    FlatBufSerialiser(hws::HWstatus& hwstat, T* val = NULL, int nev = 0, const T timestamp=0) {
      int16_t* stat = new int16_t[10];
      auto htype = builder.CreateString("questo dovrebbe essere l'header");
      auto data = builder.CreateVector(val,nev);
      auto hws = builder.CreateVector(&hwstat.hws[0],0);
      auto event = CreateEvent(builder,
                               htype,
                               timestamp,
                               hws,
                               hwstat.system_time,
                               hwstat.pid,
                               data);  
      builder.Finish(event);
      _size = builder.GetSize();
    }

    char* get() { return reinterpret_cast<char*>(builder.GetBufferPointer()); }
    const int size() { return _size; }
    
    // void extract(const char* msg, std::string& header, std::vector<T>& data) { }
    // void extract(const char* msg, int& pulse_id, std::vector<T>& data) {
    //   auto EvData = GetEvent(reinterpret_cast<const void*>(msg));
    //   pulse_id = EvData->pid();
    //   if( data.size() < EvData->data()->size() )
    //     data.resize(EvData->data()->size());
    //   std::copy(EvData->data()->begin(),EvData->data()->end(),data.begin());
    // }

    void extract(const char* msg,
                 std::vector<T>& data,
                 hws::HWstatus& hwstat) {
      auto evt = GetEvent(reinterpret_cast<const void*>(msg));

      data.resize(evt->data()->size());
      std::copy(evt->data()->begin(),evt->data()->end(),data.begin());
      hwstat.pid = evt->pid();
      return;
    }


    // void extract_header(const char* msg, char* header) {
    // }

    // void extract_data(const char* msg, char* data, int nev = 0) {
    // }


  private:
    flatbuffers::FlatBufferBuilder builder;
    int _size = 0;
    uint8_t *buf;

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
