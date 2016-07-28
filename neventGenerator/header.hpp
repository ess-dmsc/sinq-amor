#pragma once

#include <utility>

extern "C" {
#include "cJSON/cJSON.h"
}

std::pair<int,int> parse_header(const std::string& s) {
  std::pair<int,int> result;
  cJSON* root = NULL;
  root = cJSON_Parse(s.c_str());
  if( root == 0 ) {
    //      throw std::runtime_error("can't parse header");
    std::cout << "Error: can't parse header" << std::endl;
    return std::pair<int,int>(-1,-1);
  }
  
  result.first = cJSON_GetObjectItem(root,"pid")->valuedouble;
  cJSON* item = cJSON_GetObjectItem(root,"ds");
  result.second = cJSON_GetArrayItem(item,1) -> valuedouble;
  return result;
}
