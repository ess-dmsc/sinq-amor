

#include <iostream>
#include <fstream>
#include <map>
#include <thread>
#include <assert.h>

#include <stdlib.h>
#include <time.h>




namespace control {

  struct NoControl {

    NoControl(const NoControl&    ) { }
    NoControl(const uparam::Param&) { }
    NoControl(const std::string&  ) { }

    void read()     const { } ;
    void update()   const { }
    void finalize() const { };


    bool run()   const { return true; }
    bool pause() const { return false; }
    bool stop()  const { return false; }

    int rate() const   { return 140; }
    

    void operator [](const std::string&) const { }
    
  };


  struct FileControl {

    FileControl(const FileControl& other) : 
      control(other.control), s(other.s)
    { }

    FileControl(uparam::Param in) : s(in["control"]) {
      control.read(s);
    }

    FileControl(const std::string in) : s(in) {
      control.read(s);
    }

    void update() { control.read(s); }
    void finalize() { };

    bool run()    { return control["run"] == "run"; }

    bool pause()  { return control["run"] == "pause"; }

    bool stop()   { return control["run"] == "stop"; }

    int rate()    { return atoi(control["rate"].c_str()); }

    std::string& operator [](std::string key) { return control[key]; }

  private:
    uparam::Param control;
    const std::string s;
  };





  struct CommandlineControl {

    CommandlineControl() : status(_pau) { } 
    CommandlineControl(const CommandlineControl& other) : 
      status(other.status), _rate(other._rate), s(other.s)
    { }

    CommandlineControl(uparam::Param // in
                       ) : status(_pau) //s(in["control"])
    { // control.read(s);
    }

    CommandlineControl(std::string // in
                       ) : status(_pau) // s(in)
    { // control.read(s);
    }

    CommandlineControl& operator=(CommandlineControl& other) {
      status=other.status;
      _rate=other._rate;
      s=other.s;
      return *this;
    }

    void update() { }

    bool run() const {
      //    std::cout << "\trun" << (status == _run) << std::endl;
      return status == _run;
    }

    bool pause() const {
      //    std::cout << "\tpause" << (status == _pau) << std::endl;
      return status == _pau;
    }

    bool stop() const {
      //    std::cout << "\tstop" << (status == _sto ) << std::endl;
      return status == _sto;
    }
    int& rate()    { return _rate; }

    // std::string& operator [](std::string // key
    //                          ) {
    //   return std::string("");
    //   //      return control[key];
    // }
    void operator [](const std::string&) { }

    void read() {
      char value[10];

      while(// control["run"] != "stop"
            status != _sto) {

        std::cin >> value;
        std::cout << "\t" << value << std::endl;
        if ( std::string(value) == "run" || std::string(value) == "ru" )
          status = _run;
        if ( std::string(value) == "pause" || std::string(value) == "pa" )
          status = _pau;
        if ( std::string(value) == "stop" || std::string(value) == "st" )
          status = _sto;
        if( std::string(value) == "rate" || std::string(value) == "ra" ) {
          std::cout << "Insert the new transmission rate:" << std::endl;
          std::cin >> value;
          _rate = atoi(value);
        }
        std::cout << "\t" << status << std::endl;
      }
    }

  private:
    int status;
    int _rate = 140;
    //    uparam::Param control;
    std::string s;
    enum  { _run, 
            _pau, 
            _sto };
    

  };



} // control

