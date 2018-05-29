#pragma once

#include "json.h"

std::pair<int, int> parse_header(const std::string &Header) {
  std::pair<int, int> result;
  nlohmann::json j = nlohmann::json::parse(Header);

  auto Value = find<int>("value", Header);
  if (Value) {
    result.first = Value.inner();
  }
  auto DS = find<int>("ds", Header);
  if (DS) {
    result.second = DS.inner();
  }
  return result;
}
