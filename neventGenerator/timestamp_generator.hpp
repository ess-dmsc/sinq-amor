#pragma once

class RandomTimestamp { };
class ConstTimestamp { };
class NoTimestamp { };

template <class T>
void generate_timestamp(T* first,
                        T* last,
                        const uint32_t& rate,
                        const uint32_t& timestamp,
                        const NoTimestamp&) {
}

template <class T>
void generate_timestamp(T* first,
                        T* last,
                        const uint32_t& rate,
                        const uint32_t& timestamp,
                        const ConstTimestamp&) {
  std::fill(first, last, timestamp);
}

template <class T>
void generate_timestamp(T* first,
                        T* last,
                        const uint32_t& rate,
                        const uint32_t& timestamp,
                        const RandomTimestamp&) {
  std::uniform_int_distribution<T> distribution{0, floor(1e-9/rate)};
  std::default_random_engine engine;
  
  std::generate(first, last, distribution(engine));
  std::fill(first, last, timestamp);
}

