#ifndef _GENERATOR_H
#define _GENERATOR_H

#include <iostream>
#include <fstream>
#include <map>
#include <thread>
#include <chrono>
#include <ctime>

#include <assert.h>


#include <stdlib.h>
#include <time.h>

#include "zmq_generator.hpp"
#include "kafka_generator.hpp"
#include "file_writer.hpp"

#include "control.hpp"


extern "C" {
#include "cJSON/cJSON.h"
}



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
 * @tparam Control policy to start, pause and stop the generator (plain text) - TODO
 *
 *  \author Michele Brambilla <mib.mic@gmail.com>
 *  \date Wed Jun 08 15:19:52 2016 */
template<typename Streamer, typename Control, typename Serialiser>
struct Generator {
  typedef Generator<Streamer,Control, Serialiser> self_t;

  Generator(uparam::Param& p) : streamer(p), 
                                c(p["control"]) {
    /*! @param p see uparam::Param for description. Set of key-value used for initializations. */
    /*! Constructor: initialize the streamer, the header and the control. */
    //    get_control();
  }
  
  template<class T>
  void run(T* stream, int nev = 0) {
    std::thread ts(&self_t::run_impl<T>,this,stream,nev);
    c.read();
    ts.join();
  }

  
  template<class T>
  void listen(std::vector<T> stream, int nev = 0) {

    std::thread tr(&self_t::listen_impl<T>,this,stream);
    tr.join();

    listen_impl<T>(stream);
  }

private:
  
  Streamer streamer;
  Control c;
  
  template<class T>
  void run_impl(T* stream, int nev = 0) {
    
    int pulseID = 0;
    int count = 0;
    int rate = c.rate();

    hws::HWstatus hws(pulseID,rate);

    using std::chrono::system_clock;
    auto start = system_clock::now();
    auto now = system_clock::now();

    while(!c.stop()) {

      now += std::chrono::microseconds((uint64_t)(1e6/rate));

      if(c.run()) {
        streamer.send(hws,stream,nev,Serialiser());
        ++count;
      }
      else {
        streamer.send(hws,stream,0,Serialiser());
      }        
      
      ++pulseID;
      
      std::this_thread::sleep_until (now);
      if(std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 10) {
        std::cout << "Sent "       << count 
                  << " packets @ " << count*nev*sizeof(T)/(10*1e6)
                  << "MB/s" 
                  << std::endl;
        c.update();
        rate = c.rate();
        count = 0;
        now = start = system_clock::now();
      }
    }
    
  }



  template<class T>
  void listen_impl(std::vector<T> stream) {
    
    int pulseID = -1, missed = -1, pid;
    int count = 0,nev, maxsize = 0,len;
    int recvmore,rate=0;

    using std::chrono::system_clock;
    auto start = system_clock::now();

    hws::HWstatus hws(pid,rate);

    while(1) {
        
      pid = streamer.recv(hws,stream,Serialiser());
      std::cout << "pid = "      << pid 
                <<"\tpulseID = " << pulseID
                << std::endl;
      if(pid - pulseID != 1) {
        std::cout << "packet lost" << std::endl;
        pulseID = pid;
        missed++;
        std::cout << "\tpid = "      << pid 
                  <<"\tpulseID = " << pulseID
                  << std::endl;
      }
      else {
        if(nev > 0)
          ++count;
      }
      std::cout <<"\tpulseID = " << (++pulseID)
                << std::endl;

      if(std::chrono::duration_cast<std::chrono::seconds>(system_clock::now() - 
                                                          start).count() > 10 ) {
        std::cout << "Missed " << missed << " packets"
                  << " @ " << count/10 << "packets/s"
                  << std::endl;
        count = 0;
        missed = 0;
        start = system_clock::now();
      }

    }
  }


 
};



struct HeaderJson {

  HeaderJson(std::string s) {
    std::ifstream in(s);
    std::string dummy;
    while(in.good()) {
      in >> dummy;
      content += dummy;
    }
    in.close();
    len = content.size();
    std::cout << (content+="\n");
    
  } 

  const std::string& set(const int pid, 
                         const int st,
                         const int ts,
                         const int nev,
                         const int tr) {
    
    
    cJSON* root = cJSON_Parse(content.c_str());

    //////////////////////
    // !!! cJSON does not modify valueint vars: use valuedouble
    cJSON_GetObjectItem(root,"pid")->valuedouble = pid;
    cJSON_GetObjectItem(root,"st")->valuedouble = st;
    cJSON_GetObjectItem(root,"ts")->valuedouble = ts;
    cJSON_GetObjectItem(root,"tr")->valuedouble = tr;

    cJSON* item = cJSON_GetObjectItem(root,"ds");
    cJSON_GetArrayItem(item,1) -> valuedouble = nev;

    return content = std::string(cJSON_Print(root));;
  }

  //  const std::string get() { return content; }
  std::string get() { return content; }

  const int size() { return content.size(); }

  std::string content;
  int len;
};




#endif //GENERATOR_H


