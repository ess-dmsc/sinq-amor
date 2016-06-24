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

#include "serialiser.hpp"

extern "C" {
#include "cJSON/cJSON.h"
}


  namespace detail {
    
    template<class T, typename Serialiser>
    const int do_send(T* stream, int nev) {
      std::cout << "Serialiser" << std::endl;
      return 0;
    }
    
    template<class T>
    const int do_send(T* stream, int nev) {
      std::cout << "ulong" << std::endl;
      return 0;
    }
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
template<typename Streamer, typename Header, typename Control, typename Serialiser>
struct Generator {
  typedef Generator<Streamer,Header,Control, Serialiser> self_t;

  Generator(uparam::Param& p,
            const int& m = 1) : streamer(p), 
                                multiplier(m), 
                                head(p["header"]),
                                c(p["control"]) {
    /*! @param p see uparam::Param for description. Set of key-value used for initializations. */
    /*! @param m optional multiplier for emulating larger data size */
    /*! Constructor: initialize the streamer, the header and the control. */
    //    get_control();
  }
  
  template<class T>
  void run(T* stream, int nev = 0) {
    std::thread t1(&self_t::run_impl<uint64_t>,this,stream,nev);
    c.read();
    t1.join();
  }
  

private:
  
  int multiplier;
  Streamer streamer;
  Header head;
  Control c,other;
  Serialiser s;

  template<class T>
  void run_impl(T* stream, int nev = 0) {
    
    int pulseID = 0;
    int count = 0;
    double rate = c.rate();
    
    Control other(c);

    using std::chrono::system_clock;
    auto start = system_clock::now();
    auto now = system_clock::now();


    while(!c.stop()) {

      now += std::chrono::microseconds((uint64_t)(1e6/rate));
      head.set(pulseID,time(0),1234567,nev,rate);
      
      if(!Serialiser::is_serialised::value) {
        if(c.run()) {
          streamer.send(&head.get()[0],head.size(),ZMQ_SNDMORE);
          streamer.send(stream,nev,0);
          ++count;
        }
        else {
          streamer.send(&head.get()[0],head.size(),0);
        }        
      }
      else {
        if(c.run()) {
          std::cout << "RUN pid = " << pulseID;
          s(head.get(),stream,nev);
          streamer.send(s(),s.size(),0);
          ++count;
          std::cout << "\tdone" << std::endl;
        }
        else {
          std::cout << "PAUSE pid = " << pulseID;          
          std::cout << "\t(" << s(head.get(),NULL,0);
          std::cout << "," << s.size() << ")";
          streamer.send(s(head.get(),stream,0),s.size(),0);
          std::cout << "\tdone" << std::endl;                    
        }  
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
        other = c;
        count = 0;
        now = start = system_clock::now();
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
    cJSON_GetObjectItem(root, "st")->valuedouble = st;
    cJSON_GetObjectItem(root, "ts")->valuedouble = ts;
    cJSON_GetObjectItem(root, "tr")->valuedouble = tr;

    cJSON* item = cJSON_GetObjectItem(root,"ds");
    cJSON_GetArrayItem(item,1) -> valuedouble = nev;

    return content = std::string(cJSON_Print(root));;
  }

  //  const std::string get() { return content; }
  std::string get() { return content; }

  const int size() { return len; }

  std::string content;
  int len;
};




#endif //GENERATOR_H


