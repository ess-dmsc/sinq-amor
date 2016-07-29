#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <inttypes.h>
#include <type_traits>

#include "hardware.hpp"

#include "psi_sinq_schema.h"

///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Thu Jul 28 14:32:29 2016

namespace serialiser {

  template<typename T>
  struct FlatBufSerialiser {
    typedef std::true_type is_serialised;
    FlatBufSerialiser() { }
    FlatBufSerialiser(const FlatBufSerialiser& other) { }
    FlatBufSerialiser(hws::HWstatus& hws, T* val = NULL, int nev = 0) {
      build_impl(hws,val,nev);
    }


    T* operator()(std::string h, T* val = NULL, int n=0) {
      builder.Clear(); 
      // EventDataBuilder evBuilder(builder);

      // if( builder.GetSize() != 0 )
      //   std::cerr << "FlatBuffer builder is not empty" << std::endl;

      // auto htype = builder.CreateString("sinq-1.0");
      // evBuilder.add_htype(htype);
      // evBuilder.add_pid()
      //      auto pid = builder.CreateVector(packet_id,n);
      // auto event = CreateEvent(builder,
      //                          htype,
      //                          pid
      //                          );
      //      builder.Finish(event);
      _size = builder.GetSize(); // GetSize() is in byte
      //       std::cout << "\t: " << reinterpret_cast<T*>(builder.GetBufferPointer()) << "\n";
      return reinterpret_cast<T*>(builder.GetBufferPointer());
    }

    T* operator()(hws::HWstatus& hws, T* val = NULL, int nev=0) {
      builder.Clear(); 
      if( builder.GetSize() != 0 )
        throw std::runtime_error("FlatBuffer builder is not empty");
      
      EventDataBuilder evBuilder(builder);
      auto htype = builder.CreateString("sinq-1.0");
      evBuilder.add_htype(htype);
      // evBuilder.add_pid(hws.pid);
      // evBuilder.add_st(hws.system_time);
      // evBuilder.add_ts(1234);
      // evBuilder.add_tr(hws.rate);
      // evBuilder.add_nev(nev);
      //      auto value = builder.CreateVector<T>(val,nev);
      //      evBuilder.add_data(value);
      auto result = evBuilder.Finish();
      _size = builder.GetSize(); // GetSize() is in byte
      return reinterpret_cast<T*>(builder.GetBufferPointer());
    }
    
    
    const T* operator()() { return reinterpret_cast<const T*>(builder.GetBufferPointer()); }
    char* get() { return builder.GetBufferPointer(); }

    const int size() { return _size; }
    
    void extract(const char* msg, std::string& header, std::vector<T>& data) { }

    void extract(const char* msg, int& pulse_id, std::vector<T>& data) {
      auto EvData = GetEventData(reinterpret_cast<const void*>(msg));
      pulse_id = EvData->pid();
      if( data.size() < EvData->data()->size() )
        data.resize(EvData->data()->size());
      std::copy(EvData->data()->begin(),EvData->data()->end(),data.begin());
    }

    void extract(const char* msg, 
                 int& pulse_id,
                 std::vector<T>& data,
                 uint32_t timestamp,
                 uint32_t transmit_rate,
                 uint32_t n_events) { 
      extract(msg,pulse_id,data);
      auto EvData = GetEventData(reinterpret_cast<const void*>(msg));
      timestamp = EvData->ts();
      transmit_rate = EvData->tr();
      n_events = EvData->nev();
    }


    void extract_header(const char* msg, char* header) {
      // auto event = GetEvent(reinterpret_cast<const void*>(msg));
      // std::cout << "header: " << "\n\t" << event->header()->c_str() << std::endl;
      // std::copy (event->header()->c_str(), 
      //            event->header()->c_str()+event->header()->Length(), header);
    }

    void extract_data(const char* msg, char* data, int nev = 0) {
      // auto event = GetEvent(reinterpret_cast<const void*>(msg));
      // std::copy (event->data()->begin(), event->data()->end(),
      //            data);
    }


  private:
    flatbuffers::FlatBufferBuilder builder;
    int _size = 0;
    uint8_t *buf;

    void build_impl(hws::HWstatus& hws, T* val, int nev) {
    }
    
  };


///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Fri Jun 17 12:22:01 2016
  template<typename T>
  struct NoSerialiser {
    typedef std::false_type is_serialised;
    
    T* operator()(std::string, T*, int) const {
      return NULL;
    }    
    const int size() { return _size; }

  private:
    const int _size = 0;
    const uint8_t *buf;

  };
  



} // serialiser
