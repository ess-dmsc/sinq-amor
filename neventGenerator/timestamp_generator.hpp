#pragma once

#include<chrono>

template <class T>
void generate_timestamp(std::vector<T> &output, const uint32_t &rate,
                        const std::chrono::milliseconds &ms,
                        const std::string &generation_type) {
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(ms);
  if (generation_type == "const_timestamp") {
    std::fill(output.begin(), output.end(), ns.count());
    return;
  }
  if (generation_type == "random_timestamp") {
    std::uniform_int_distribution<T> distribution{
        0, static_cast<T>(std::llround(floor(1e-9 / rate)))};
    std::default_random_engine engine;
    std::generate(output.begin(), output.end(),
                  std::bind(distribution, engine));
    return;
  }
  if (generation_type == "none") {
    return;
  }
  throw std::runtime_error("Unknown timestamp generation type");
}
