#include <nlohmann/json.hpp>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include <string>

template <typename T> class JsonMaybe {
public:
  JsonMaybe() {}
  JsonMaybe(T inner) : inner_(inner), found_(true) {}
  explicit operator bool() const { return found_; }
  T inner() const { return inner_; }

private:
  T inner_;
  bool found_ = false;
};

template <typename T>
JsonMaybe<T> find(std::string Key, nlohmann::json const &Json);

template <typename T>
JsonMaybe<T> find(std::string Key, nlohmann::json const &Json) {
  auto It = Json.find(Key);
  if (It != Json.end()) {
    return JsonMaybe<T>(It.value().get<T>());
  }
  return JsonMaybe<T>();
}
