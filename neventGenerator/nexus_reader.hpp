#ifndef _NEXUS_READER_H
#define _NEXUS_READER_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

#include <cstring>
#include <ctime>
#include <stdlib.h>

#include <nexus/napi.h>

#include "uparam.hpp"

namespace nexus {

  ///  \author Michele Brambilla <mib.mic@gmail.com>
  ///  \date Wed Jun 08 16:49:17 2016
  template<typename Instrument>
  struct NeXusSource {
    typedef NeXusSource self_t;
    typedef uint64_t value_type;

    std::vector<value_type>::iterator begin() { return data.begin(); }
    std::vector<value_type>::iterator end() { return data.end(); }
    std::vector<value_type>::const_iterator begin() const { return data.begin(); }
    std::vector<value_type>::const_iterator end() const { return data.end(); }
  
    NeXusSource(uparam::Param p, const int multiplier=1) {
      if(NXopen(p["filename"].c_str(),NXACC_READ,&handle) != NX_OK){
        throw std::runtime_error("Failed to open NeXus file "+p["filename"]);
      }
      read();
      if( multiplier > 1) {
        int nelem=data.size();
        for(int m=1;m<multiplier;++m)
          data.insert(data.end(),data.begin(),data.begin()+nelem);
      }
    }

    int count() const { return data.size(); }

  private:
    NXhandle handle;
    Instrument instrum;
    std::vector<value_type> data;

    void read() {
      instrum(handle, data);
      NXclose(&handle); 
    }

  };


  ///  \author Michele Brambilla <mib.mic@gmail.com>
  ///  \date Thu Jun 09 16:43:08 2016
  struct Rita2 {
    static const int n_monitor = 2;
    std::vector<std::string>::iterator begin() { return path.begin(); }
    std::vector<std::string>::iterator end() { return path.end(); }
    std::vector<std::string>::const_iterator begin() const { return path.begin(); }
    std::vector<std::string>::const_iterator end() const { return path.end(); }

    Rita2() {
      path.push_back("/entry1/RITA-2/detector/counts");
      path.push_back("random");
    }

    template<typename T>
    void operator()(const NXhandle& handle, std::vector<T>& stream) {
      int rank, type;//, size;
    
      if(NXopenpath(handle,path[0].c_str()) != NX_OK){
        throw std::runtime_error("Error reading NeXus data");
      }
      NXgetinfo(handle,&rank,dim,&type);

      // std::cout << "rank: " << rank << "\n"
      //           << "type: " << type << "\n"
      //           << "dim: " << dim[0] << "\t" << dim[1] << "\t" << dim[2] << "\n";
      size = dim[0];
      for(int i = 1; i < rank; i++) size *= dim[i];
      dim[rank] = dim[rank-1];
    
      // std::cout << "size: " << size << "\n"    ;

      data = new int32_t [size];
    
      NXgetdata(handle,data);

      toEventFmt<T>(stream);
    }
  
    std::vector<std::string> path;

  private:
    int32_t* data = NULL;
    int32_t dim[3+1];
    int size;

    template<typename T>
    void toEventFmt(std::vector<T>& signal) {
      int offset, nCount;
      union {
        uint64_t value;
        struct {
          uint32_t low;
          uint32_t high;
        };
      } x;

      srand(time(NULL));
      for(int i = 0; i < dim[0]; ++i){
        for(int j = 0; j < dim[1]; ++j) {
          offset = dim[2]*(j+dim[1]*i);
          for(int k = 0; k < dim[2]; ++k) {
            nCount = data[offset+k];
            x.low  = 1 << 31 | 1 << 30 | 1 << 29 | 1 << 28 | 2 << 24 | i << 12 | j;
            for(int l = 0;l< nCount; ++l) {
              x.high = rand();
              signal.push_back(x.value);
            }
          }
        }
      }
    }
  
  };







  ///  \author Michele Brambilla <mib.mic@gmail.com>
  ///  \date Thu Jun 09 16:43:08 2016
  struct Amor {
    static const int n_monitor = 2;
    std::vector<std::string>::iterator begin() { return path.begin(); }
    std::vector<std::string>::iterator end() { return path.end(); }
    std::vector<std::string>::const_iterator begin() const { return path.begin(); }
    std::vector<std::string>::const_iterator end() const { return path.end(); }

    Amor() {
      path.push_back("/entry1/AMOR/area_detector/data");
      path.push_back("/entry1/AMOR/area_detector/time_binning");
    }

    template<typename T>
    void operator()(const NXhandle& handle, std::vector<T>& stream) {
      int rank, type;//, size;
    
      if(NXopenpath(handle,path[0].c_str()) != NX_OK){
        throw std::runtime_error("Error reading NeXus data");
      }
      NXgetinfo(handle,&rank,dim,&type);
      // std::cout << "rank: " << rank << "\n"
      //           << "type: " << type << "\n"
      //           << "dim: " << dim[0] << "\t" << dim[1] << "\t" << dim[2] << "\n";
      size = dim[0];
      for(int i = 1; i < rank; i++) size *= dim[i];
      dim[rank] = dim[rank-1];
      data = new int32_t [size];
      NXgetdata(handle,data);
      // std::cout << "size: " << size << "\n"    ;

      tof = new float [dim[2]];
      if(NXopenpath(handle,path[1].c_str()) != NX_OK){
        throw std::runtime_error("Error reading NeXus data");
      }
      NXgetdata(handle,tof);

      // std::cout << "size: " << size << "\n"    ;

      NXclose((void**)&handle);

      toEventFmt<T>(stream);
    }
  
    std::vector<std::string> path;

  private:
    int32_t* data = NULL;
    float * tof = NULL;
    int32_t dim[3+1];
    uint32_t size;

    
    template<typename T>
    void toEventFmt(std::vector<T>& signal) {
      unsigned long nEvents = std::accumulate(data,data+size,0);
      int offset, nCount;
      int detID = 0;

      union {
        uint64_t value;
        struct {
          uint32_t low;
          uint32_t high;
        };
      } x;


      for(int i = 0; i < dim[0]; ++i){
        for(int j = 0; j < dim[1]; ++j) {
          detID++;
          offset = dim[2]*(j+dim[1]*i);
          for(int k = 0; k < dim[2]; ++k) {
            nCount = data[offset+k];
            x.high = std::round(tof[k]/10.);
            x.low  = 1 << 31 | 1 << 30 | 1 << 29 | 1 << 28 | 2 << 24 | detID;
            for(int l = 0;l< nCount; ++l) {
              signal.push_back(x.value);
            }
          }
        }
      }
    }
    
  };





} // namespace

#endif //NEXUS_READER_H
