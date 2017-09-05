#include <map>
#include <regex>
#include <string>

namespace parser {

// class Parser {
// public:
//   using Param = std::map<std::string, std::string>;

//   Parser() {};

//   Param &init(const std::string &input) {
//     p["protocol"] = _get_protocol(input);
//     p["host"] = _get_address(input);
//     p["port"] = _get_port(input);
//     p["topic"] = _get_topic(input);
//     return p;
//   }

//   const Param &get() { return p; }

// private:
//   Param p;
//   std::smatch m;

//   std::string get_protocol(const std::string &s, const std::string &d = "")
// {
//     if (std::regex_search(s, m, std::regex("^[A-Za-z]+"))) {
//       return std::move(std::string(m[0]));
//     }
//     return std::move(d);
//   }
//   std::string _get_address(const std::string &s, const std::string &d = "") {
//     if (std::regex_search(s, m, std::regex("//[A-Za-z\\d.]+")))
//       return std::move(std::string(m[0]).substr(2));
//     return std::move(d);
//   }
//   std::string _get_port(const std::string &s, const std::string &d = "") {
//     if (std::regex_search(s, m, std::regex(":\\d+/"))) {
//       std::string result(m[0]);
//       return std::move(result.substr(1, result.length() - 2));
//     }
//     return std::move(d);
//   }
//   std::string _get_topic(const std::string &s, const std::string &d = "") {
//     std::smatch m;
//     if (std::regex_search(s, m, std::regex("[A-Za-z0-9-_:.]*$")))
//       return std::move(std::string(m[0]));
//     return std::move(d);
//   }
// };

class Parser {
public:
  using Param = std::map<std::string, std::string>;

  Parser(){};

  Param &init(const std::string &input) {
    p["protocol"] = _get_protocol(input);
    p["host"] = _get_address(input);
    p["port"] = _get_port(input);
    p["topic"] = _get_topic(input);
    return p;
  }

  const Param &get() { return p; }

private:
  Param p;
  std::smatch m;

  std::string _get_protocol(const std::string &s, const std::string &d = "") {
    size_t first = s.find("//");
    return std::move(s.substr(0, first));
  }
  std::string _get_address(const std::string &s, const std::string &d = "") {
    size_t first = s.find("//") + 2;
    size_t last = s.find(":");
    return std::move(s.substr(first, last - first));
  }
  std::string _get_port(const std::string &s, const std::string &d = "") {
    size_t first = s.find(":") + 1;
    size_t last = s.find_last_of("/");
    return std::move(s.substr(first, last - first));
  }
  std::string _get_topic(const std::string &s, const std::string &d = "") {
    size_t last = s.find_last_of("/");
    return std::move(s.substr(last + 1));
  }
};
}
