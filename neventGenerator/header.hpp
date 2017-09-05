#pragma once

#include <utility>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/stringbuffer.h>

std::pair<int, int> parse_header(const std::string &s) {
  std::pair<int, int> result;
  rapidjson::Document d;
  d.Parse(s.c_str());
  result.first = d["value"].GetInt();
  result.second = d["ds"][1].GetInt();
  return result;
}
