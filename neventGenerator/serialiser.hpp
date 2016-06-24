#include <iostream>
#include <fstream>
#include <string>
#include <inttypes.h>
#include <type_traits>

#include "sample_flatbuf_generated.h"

///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Fri Jun 17 12:21:55 2016
namespace serialiser {

  template<typename T>
  struct FlatBufSerialiser {
    typedef std::true_type is_serialised;
    
    T* operator()(std::string h, T* val = NULL, int n=0) {
      builder.Clear(); 
      _size = builder.GetSize();
      std::cout << "\nafter builder.Clear() size = " << _size << "\t";
      auto header = builder.CreateString(h);
      auto data = builder.CreateVector(val,n);
      auto event = CreateEvent(builder,header,data);
      builder.Finish(event);
      _size = builder.GetSize(); // GetSize() is in byte
      std::cout << "after builder.Finish() size = " << _size << std::endl;
      
      //       std::cout << "\t: " << reinterpret_cast<T*>(builder.GetBufferPointer()) << "\n";
      
      return reinterpret_cast<T*>(builder.GetBufferPointer());
    }
    
    const T* operator()() {
      return reinterpret_cast<const T*>(builder.GetBufferPointer());
    }

    const int size() { // return builder.GetSize();
      return _size;
    }
    
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
    
    T* operator()(std::string, T*, int) const {
      return NULL;
    }    
    const int size() { return _size; }


  private:
    const int _size = 0;
    const uint8_t *buf;

  };
  



} // serialiser
