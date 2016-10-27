#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <vector>


namespace hws {

  // std::string to_string(HWstatus::short_type val) { return std::to_string(static_cast<unsigned long long>(val)) };
  // std::string to_string(HWstatus::value_type val) { return std::to_string(static_cast<unsigned long long>(val)) };

  using namespace std::chrono;

  struct HWstatus {
    typedef uint32_t value_type;
    typedef uint16_t short_type;
    
    HWstatus(value_type& pulse_id, 
             value_type& transmit_rate) : htype("sinq-1.0"), 
                                          ds({32,1,1,1,1,4,12,12}), 
                                          hws({0,0,0}), 
                                          system_time(time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count()*1e-3), 
                                          pid(pulse_id),
                                          rate(transmit_rate)
    { };

    void update() {
      system_time=time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count()*1e-3;
    }
    

    std::string to_string(const value_type& nev, const value_type& timestamp) {

      std::string ds_string;
      // ds_string = std::string("[{") 
      //   + "\"ts\":"  + std::to_string(ds[0]) + ','
      //   + "\"bsy\":" + std::to_string(ds[1]) + ','
      //   + "\"cnt\":" + std::to_string(ds[2]) + ','
      //   + "\"rok\":" + std::to_string(ds[3]) + ','
      //   + "\"gat\":" + std::to_string(ds[4]) + ','
      //   + "\"evt\":" + std::to_string(ds[5]) + ','
      //   + "\"id1\":" + std::to_string(ds[6]) + ','
      //   + "\"id2\":" + std::to_string(ds[7]) + ','
      //   + '}'                + ','
      //   + std::to_string(nev)
      //   + "]";

      std::string s;
      s =  std::string("{") ;
      //   + "\"htype\":\"" + htype                       + "\","
      //   + "\"pid\":\""   + std::to_string(pid)         + "\","
      //   + "\"st\":\""    + std::to_string(system_time) + "\","
      //   + "\"ts\":\""    + std::to_string(timestamp)   + "\","
      //   + "\"tr\":\""    + std::to_string(rate)        + "\","
      //   + "\"ds\":\""    + ds_string                   + "\","
      //   + "\"hws\":\":"                                
      //   + "{"                                          
      //   + "\"error\":"   + std::to_string(hws[0])      + "\","
      //   + "\"full\":"    + std::to_string(hws[1])      + "\","
      //   + "\"zmqerr\":"  + std::to_string(hws[2])      + "\""
      //   + "\"lost\":"    + "[0,0,0,0,0,0,0,0,0,0]"
      //   + "}"                                          + "\""
      //   + "}";
      return s;
    }
    
    std::string htype;
    std::vector<short_type> ds;
    std::vector<short_type> hws;
    double system_time;
    value_type& pid;
    value_type& rate;
  };
    

} // hws
  
