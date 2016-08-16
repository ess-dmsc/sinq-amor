#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <numeric>
#include <map>
#include <fstream>
#include <cmath>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
///
///  Minimalist framework to read/write parameters.
///
///  \author Dirk Hesse <herr.dirk.hesse@gmail.com>
///  \date Wed May 30 16:39:24 2012

namespace uparam {
  struct PlainText { };
  struct RapidJSON { };

  //  template <typename Source>
  class Param {
  public:
    typedef std::map<std::string, std::string> map_t;
    typedef map_t::iterator iterator;
    typedef map_t::const_iterator const_iterator;
    iterator begin() { return params.begin(); }
    iterator end() { return params.end(); }
    const_iterator begin() const { return params.begin(); }
    const_iterator end() const { return params.end(); }

    
    void read(const std::string& s, const PlainText&){
      read_impl(s,PlainText());
    }

    void read(const std::string& s, const RapidJSON&) {
      read_impl(s,RapidJSON());
    }

    void write(const std::string& s) const {
      std::ofstream of(s.c_str(), std::ios::trunc);
      for (const_iterator i = begin(); i != end(); ++i)
        of << i->first << "  " << i->second << "\n";
      of.close();
    }
    std::string& operator[](const std::string& s){
      return params[s];
    }
    void print(std::ostream& os = std::cout) const {
      for (const_iterator i = begin(); i != end(); ++i)
        os << i->first << "\t:\t" << i->second << "\n";
    }
  private:
    map_t params;
    std::string to_upper(std::string& in){
      std::transform(in.begin(), in.end(), in.begin(), 
                     (int(*)(int))std::toupper);
      return in;
    }
    void read_impl(const std::string& s, const PlainText&){
      std::ifstream inf(s.c_str());
      if (!inf.is_open()) {
        std::cerr << "PARAMETER FILE " << s  << " NOT FOUND!\n";
        throw std::exception();
      }
      while (inf.good()) {
        std::string name = get_str(inf);
        if (name != "")
          params[name] = get_str(inf);
      }
      inf.close();
    }
    void read_impl(const std::string& s, const RapidJSON&){
      FILE* fp = fopen(s.c_str(), "rb");
      char buffer[65536];
      rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
      rapidjson::Document d;
      d.ParseStream(is);
      for( auto i=d.MemberBegin();i!=d.MemberEnd();++i) 
        params[i->name.GetString()] = i->value.GetString();
      fclose(fp);
    }

    std::string get_str(std::ifstream& in) {
      std::string next;
      in >> next;
      char dummy[256];
      while (next[0] == '#' && in.good()) {
        in.getline(dummy, 256);
        in >> next;
      }
      if (next[0] == '#')
        next = "";
      return next;
    }

  };


  template<typename T>
  T to_num(const std::string& s) {
    T result;
    std::stringstream(s) >> result;
    return result;
  }

}

