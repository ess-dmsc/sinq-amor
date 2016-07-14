#ifndef _ZMQ_GENERATOR_H
#define _ZMQ_GENERATOR_H

#include <string>
#include <sstream>
#include <exception>
#include <assert.h>
#include <zmq.h>

#include "serialiser.hpp"
#include "uparam.hpp"

extern "C" {
#include "cJSON/cJSON.h"
}

/*! \struct ZmqGen
 *  Uses 0MQ as data the streamer.
*
*  \author Michele Brambilla <mib.mic@gmail.com>
*  \date Wed Jun 08 15:19:27 2016
*/


template<int mode_selector=0> // mode_selector = 0 -> transmitter, = 1 -> receiver 
struct ZmqGen {
  static const int max_header_size=10000;
  static const int max_recv_size=100000000;

  
  ZmqGen(uparam::Param p ) {
    /*! @param p see uparam::Param for description. Must contain "port" key-value */
    /*! Generates the 0MQ context and bind PULL socket to "port". If binding
        fails throws an error */
    context = zmq_ctx_new ();
    int rc;
    if(!mode_selector) {
      socket = zmq_socket (context, ZMQ_PUSH);
      std::cout << "tcp://*:"+p["port"] << std::endl;
      rc = zmq_bind(socket,("tcp://*:"+p["port"]).c_str());
      //    socket.setsockopt(zmq::SNDHWM, 100);
    }
    else {
      socket = zmq_socket (context, ZMQ_PULL);
      std::cout << "tcp://" << p["host"] << ":" << p["port"] << std::endl;
      rc = zmq_connect(socket,("tcp://"+p["host"]+":"+p["port"]).c_str());
    }
    assert (rc == 0);
  }

  template<typename T>
  void send(const T* data, int size, const int flag = 0) {
    std::cout << "->" << data
              << "," << size
              << std::endl;
    /*! @param data data to be sent
     *  @param size number of elements
     *  @param flag optional flag (default = 0) */
    zmq_send(socket,data,size,flag);
    std::cout << std::endl;
  }

  template<typename T>
  void send(std::string h, T* data, int nev, serialiser::NoSerialiser<T>) {
    if(nev > 0) {
      zmq_send(socket,&h[0],h.size(),ZMQ_SNDMORE);
      zmq_send(socket,data,nev*sizeof(T),0);
    }
    else {
      zmq_send(socket,&h[0],h.size(),0);
    }
    std::cout << "->" << h
              << "," << h.size()
              << "," << nev
              << std::endl;    
  }
  
  template<typename T>
  void send(std::string h, T* data, int nev, serialiser::FlatBufSerialiser<T>) {
    serialiser::FlatBufSerialiser<T> s;
    zmq_send(socket,(char*)s(h.c_str(),data,nev),s.size(),0);
    std::cout << "->" << h
              << "," << h.size()
              << "," << nev
              << std::endl;    
    
    std::ofstream of("first.out",std::ofstream::binary);
    of.write((char*)s(h.c_str(),data,nev),s.size()); 
    of.close();
    


    // char* he = new char[10000]; 
    // T* d = new T[1000000];
    // s.extract((char*)s(h.c_str(),data,nev),he,d);
    // std::cout << "unpacked\n"<< he << std::endl;

    // s.extract_data((char*)s(h.c_str(),data,nev),(char*)d,195509);
    // std::cout << "unpacked\n"
    //           << d[0] << std::endl
    //           << d[1] << std::endl
    //           << d[2] << std::endl
    //           << d[3] << std::endl
    //           << d[4] << std::endl;


  }
  

  template<typename T>
  int recv(char* h, T* data, int& nev, serialiser::NoSerialiser<T>) {

    int rcvmore;
    int s = zmq_recv (socket,h,max_header_size,0);
    zmq_getsockopt(socket, ZMQ_RCVMORE, &rcvmore , &optlen);

    cJSON* root = NULL;
    root = cJSON_Parse(h);
    if( root == 0 ) {
      //      throw std::runtime_error("can't parse header");
      std::cout << "Error: can't parse header" << std::endl;
      exit(-1);
    }

    cJSON* item = cJSON_GetObjectItem(root,"ds");
    int pid = cJSON_GetObjectItem(root,"pid")->valuedouble;
    nev = cJSON_GetArrayItem(item,1) -> valuedouble;
    
    if(rcvmore && (nev > 0) ) {
      if ( nev > max_event_size ) {
        max_event_size = nev;
        if (d)
          delete [] d;
        d = new char [nev*sizeof(T)];
      }
      int s = zmq_recv (socket,d,nev*sizeof(T),0);
      data = (T*)(d);
    }
    else
      if(rcvmore || (nev > 0) )
        std::cout << "Error receiving data" << std::endl;
    return pid;
  }



  template<typename T>
  int recv(char* h, T* data, int& nev, serialiser::FlatBufSerialiser<T>) {
    if (!raw) {
      raw = new char [max_recv_size];
    }
    int n = zmq_recv (socket,raw,max_header_size,0);

    serialiser::FlatBufSerialiser<T> s;


    s.extract(raw,h,data);

    // cJSON* root = NULL;
    // root = cJSON_Parse(h);
    // if( root == 0 ) {
    //   //      throw std::runtime_error("can't parse header");
    //   std::cout << "Error: can't parse header" << std::endl;
    //   exit(-1);
    // }
    // cJSON* item = cJSON_GetObjectItem(root,"ds");
    // int pid = cJSON_GetObjectItem(root,"pid")->valuedouble;
    // nev = cJSON_GetArrayItem(item,1) -> valuedouble;

    // std::cout << "nev: " << nev << std::endl;
    // if(nev > 0 ) {
    //   if ( (!d) || nev > max_event_size ) {
    //     max_event_size = nev;
    //     if (d)
    //       delete [] d;
    //     d = new char [nev*sizeof(T)];
    //   }
      
    //   s.extract_data(raw,d,nev);
    //   data = reinterpret_cast<T*>(d);
    // }
    // return pid;
    return 1;
  }





private:
  void* context; /// pointer to 0MQ context 
  void* socket;  /// pointer to 0MQ socket 
  char* d = NULL;
  char* raw = NULL;
  int max_event_size = 0;
  size_t optlen=sizeof(int);
};




struct ZmqRecv {
  
  ZmqRecv(uparam::Param p ) {
    /*! @param p see uparam::Param for description. Must contain "port" key-value */
    /*! Generates the 0MQ context and bind PULL socket to "port". If binding
        fails throws an error */
    context = zmq_ctx_new ();
    socket = zmq_socket (context, ZMQ_PULL);
    std::cout << "tcp://localhost:"+p["port"] << std::endl;
    int rc = zmq_connect(socket,("tcp://localhost:"+p["port"]).c_str());
    assert (rc == 0);
  }

  template<typename T>
  int recv(T* data, int size, int &rcvmore) {
    int s = zmq_recv (socket,data,size*sizeof(T),0);
    zmq_getsockopt(socket, ZMQ_RCVMORE, &rcvmore , &optlen);
    //    std::cout << data << std::endl;
    return s;
  }

private:
  void* context; /// pointer to 0MQ context 
  void* socket;  /// pointer to 0MQ socket 
  size_t optlen=sizeof(int);
};

#endif //ZMQ_GENERATOR_H
