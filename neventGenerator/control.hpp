

#include <iostream>
#include <fstream>
#include <map>
#include <thread>
#include <assert.h>

#include <stdlib.h>
#include <time.h>


namespace control {

  const int default_rate = 10;

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
    int rate() const   { return default_rate; }

    void operator [](const std::string&) const { }    
  };


  struct FileControl {

    FileControl(const FileControl& other) : control(other.control), s(other.s) { }
    FileControl(uparam::Param in) : s(in["control"]) { control.read(s,uparam::PlainText()); }
    FileControl(const std::string in) : s(in) { control.read(s,uparam::PlainText()); }

    void update() { control.read(s,uparam::PlainText()); }
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
    CommandlineControl(const CommandlineControl& other) : status(other.status), _rate(other._rate), s(other.s) { }
    CommandlineControl(uparam::Param&) : status(_pau) { }
    CommandlineControl(std::string) : status(_pau) { }

    CommandlineControl& operator=(CommandlineControl& other) {
      status=other.status;
      _rate=other._rate;
      s=other.s;
      return *this;
    }

    void update() { }
    void read() { 
      read_impl();
      //      std::thread t(&CommandlineControl::read_impl,this);
      //      t.join();
    }

    bool run(const bool do_run) {
      status = _run;
      return status == _run; 
    }
    bool run() const { return status == _run; }
    bool pause() const { return status == _pau; }
    bool stop() const { return status == _sto; }

    int& rate()    { return _rate; }
    void operator [](const std::string&) { }


  private:
    int status;
    int _rate;
    std::string s;
    enum  { _run, _pau, _sto };
    
    void read_impl() {
      char value[10];

      while(status != _sto) {
        std::cin >> value;
        std::cout << "\t" << value << std::endl;
        if ( std::string(value) == "run"   || std::string(value) == "ru" )
          status = _run;
        if ( std::string(value) == "pause" || std::string(value) == "pa" )
          status = _pau;
        if ( std::string(value) == "stop"  || std::string(value) == "st" )
          status = _sto;
        if( std::string(value) == "rate"   || std::string(value) == "ra" ) {
          std::cout << "Insert the new transmission rate:" << std::endl;
          std::cin >> value;
          _rate = atoi(value);
        }
        std::cout << "\t" << status << std::endl;
      }
    }

  };



} // control

