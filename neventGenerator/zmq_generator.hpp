#ifndef _ZMQ_GENERATOR_H
#define _ZMQ_GENERATOR_H

#include <string>
#include <sstream>
#include <assert.h>
#include <zmq.h>

#include "uparam.hpp"

/*! \struct ZmqGen
 *  Uses 0MQ as data the streamer.
*
*  \author Michele Brambilla <mib.mic@gmail.com>
*  \date Wed Jun 08 15:19:27 2016
*/

struct ZmqGen {
  
  ZmqGen(uparam::Param p ) {
    /*! @param p see uparam::Param for description. Must contain "port" key-value */
    /*! Generates the 0MQ context and bind PULL socket to "port". If binding
        fails throws an error */
    context = zmq_ctx_new ();
    socket = zmq_socket (context, ZMQ_PUSH);
    std::cout << "tcp://*:"+p["port"] << std::endl;
    int rc = zmq_bind(socket,("tcp://*:"+p["port"]).c_str());
    assert (rc == 0);
    //    socket.setsockopt(zmq::SNDHWM, 100);
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

private:
  void* context; /// pointer to 0MQ context 
  void* socket;  /// pointer to 0MQ socket 
};

#endif //ZMQ_GENERATOR_H
