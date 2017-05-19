#pragma once

#include <cctype>
#include <sstream>
#include <string>
#include <utility>

#include <assert.h>

extern "C" {
#include <librdkafka/rdkafka.h>
}
#include <librdkafka/rdkafkacpp.h>

#include "header.hpp"
#include "serialiser.hpp"
#include "uparam.hpp"

namespace generator {

enum { transmitter, receiver };
/*! \struct KafkaGen
 *  Uses Kafka as the streamer
 *
 *  \author Michele Brambilla <mib.mic@gmail.com>
 *  \date Wed Jun 08 15:19:16 2016
 */
template <int mode_selector> struct KafkaGen {

  KafkaGen(uparam::Param p) : brokers(p["brokers"]), topic_str(p["topic"]) {
    conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    conf->set("metadata.broker.list", brokers, errstr);
    if (!debug.empty()) {
      if (conf->set("debug", debug, errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
      }
    }
    conf->set("message.max.bytes", "23100100", errstr);
    std::cerr << errstr << std::endl;
    conf->set("api.version.request", "true", errstr);
    std::cerr << errstr << std::endl;

    if (topic_str.empty()) {
      std::cerr << "Topic not set" << std::endl;
      exit(1);
    }
    producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
      std::cerr << "Failed to create producer: " << errstr << std::endl;
      exit(1);
    }
    topic = RdKafka::Topic::create(producer, topic_str, tconf, errstr);
    if (!topic) {
      std::cerr << "Failed to create topic: " << errstr << std::endl;
      exit(1);
    }
  }

  template <typename T> void send(T *data, const int size, const int flag = 0) {
    RdKafka::ErrorCode resp = producer->produce(
        topic, partition, RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
        data, size * sizeof(T), NULL, NULL);
    if (resp != RdKafka::ERR_NO_ERROR)
      std::cerr << "% Produce failed: " << RdKafka::err2str(resp) << std::endl;
    std::cerr << "% Produced message (" << sizeof(data) << " bytes)"
              << std::endl;
  }

  template <typename T>
  void send(std::string h, T *data, int nev,
            serialiser::NoSerialiser<T> serialiser) {
    RdKafka::ErrorCode resp = producer->produce(
        topic, partition, RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
        (void *)h.c_str(), h.size(), NULL, NULL);
    if (resp != RdKafka::ERR_NO_ERROR)
      std::cerr << "% Produce failed header: " << RdKafka::err2str(resp)
                << std::endl;
    if (nev > 0) {
      resp = producer->produce(
          topic, partition, RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
          (void *)data, nev * sizeof(T), NULL, NULL);

      if (resp != RdKafka::ERR_NO_ERROR)
        std::cerr << "% Produce failed data: " << RdKafka::err2str(resp)
                  << std::endl;
    }

    std::cerr << "% Produced message (" << h.size() + sizeof(T) * nev
              << " bytes)" << std::endl;
  }

  template <typename T>
  void send(std::string h, T *data, int nev,
            serialiser::FlatBufSerialiser<T> serialiser) {
    // serialiser.serialise(h,data,nev);
    // RdKafka::ErrorCode resp =
    //   producer->produce(topic, partition,
    //                     RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
    //                     //                          (void*)s(), s.size(),
    //                     (void*)data, nev,
    //                     NULL, NULL);
    // if (resp != RdKafka::ERR_NO_ERROR)
    //   std::cerr << "% Produce failed: " <<
    //     RdKafka::err2str(resp) << std::endl;
    // std::cerr << "% Produced message (" << h.size()+sizeof(T)*nev
    //           << " bytes)"
    //           << std::endl;
  }

  template <typename T>
  void send(const uint64_t &pid, const uint64_t &timestamp, T *data, int nev,
            serialiser::FlatBufSerialiser<T> serialiser) {
    serialiser.serialise(pid, timestamp, data, nev);

    RdKafka::ErrorCode resp = producer->produce(
        topic, partition, RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
        (void *)serialiser.get(), serialiser.size(), NULL, NULL);

    // RdKafka::ErrorCode resp = producer->produce(
    //     topic->name(), partition, RdKafka::Producer::RK_MSG_COPY,
    //     (void *)serialiser.get(), serialiser.size(), NULL, 0, timestamp,
    //     NULL);
    if (resp != RdKafka::ERR_NO_ERROR)
      throw std::runtime_error("% Produce failed: " + RdKafka::err2str(resp));
  }

  template <typename T>
  void send(const uint64_t &pid, const uint64_t &timestamp, T *data, int nev,
            serialiser::NoSerialiser<T>) {
    std::string header = std::string("{pid:") + std::to_string(pid) + ",ts:" +
                         std::to_string(timestamp) + " }";
    send(header, data, nev, serialiser::NoSerialiser<T>());
  }

private:
  std::string brokers;
  std::string topic_str;
  std::string errstr;
  std::string debug;

  int32_t partition = 0; // RdKafka::Topic::PARTITION_UA;
  int64_t start_offset = RdKafka::Topic::OFFSET_BEGINNING;

  RdKafka::Conf *conf;
  RdKafka::Conf *tconf;
  RdKafka::Producer *producer;
  RdKafka::Topic *topic;
};

template <int mode_selector> struct KafkaListener {
  static const int max_header_size = 10000;

  KafkaListener(uparam::Param p)
      : brokers(p["brokers"]), topic_str(p["topic"]) {
    conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
    conf->set("metadata.broker.list", brokers, errstr);
    if (!debug.empty()) {
      if (conf->set("debug", debug, errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
      }
    }
    conf->set("fetch.message.max.bytes", "23100100", errstr);
    conf->set("receive.message.max.bytes", "23100100", errstr);
    std::cerr << errstr << std::endl;

    if (topic_str.empty()) {
      std::cerr << "Topic required." << std::endl;
      exit(1);
    }
    consumer = RdKafka::Consumer::create(conf, errstr);
    if (!consumer) {
      std::cerr << "Failed to create consumer: " << errstr << std::endl;
      exit(1);
    }
    topic = RdKafka::Topic::create(consumer, topic_str, tconf, errstr);
    if (!topic) {
      std::cerr << "Failed to create topic: " << errstr << std::endl;
      exit(1);
    }

    RdKafka::ErrorCode resp = consumer->start(topic, partition, start_offset);
    if (resp != RdKafka::ERR_NO_ERROR) {
      std::cerr << "Failed to start consumer: " << RdKafka::err2str(resp)
                << std::endl;
      exit(1);
    }
  }

  template <typename T>
  int recv(std::string &h, std::vector<T> &data, serialiser::NoSerialiser<T>) {
    void *value;
    std::pair<int, int> result;
    RdKafka::Message *msg = nullptr;
    do {
      msg = consumer->consume(topic, partition, 1000);
    } while (msg->err() != RdKafka::ERR_NO_ERROR);
    result = consume_header(msg, static_cast<void *>(&h[0]));
    h = std::string(static_cast<char *>(msg->payload()));
    if (result.second > 0) {
      msg = consumer->consume(topic, partition, 10000);
      if (msg->err() != RdKafka::ERR_NO_ERROR)
        std::cerr << "expected event data" << std::endl;
      consume_data<T>(msg, value);
    }
    delete msg;
    return result.first;
  }

  template <typename T>
  int recv(std::string &h, std::vector<T> &data,
           serialiser::FlatBufSerialiser<T>) {
    void *value;
    std::pair<int, int> result;
    RdKafka::Message *msg = nullptr;
    do {
      msg = consumer->consume(topic, partition, 1000);
      std::cout << RdKafka::err2str(msg->err()) << std::endl;
    } while (msg->err() != RdKafka::ERR_NO_ERROR);
    result = consume_serialised(msg, value, serialiser::FlatBufSerialiser<T>());
    h = std::string(static_cast<char *>(msg->payload()));
  }

  template <typename T>
  std::pair<uint64_t, uint64_t> recv(std::vector<T> &data,
                                     serialiser::NoSerialiser<T>) {
    return std::pair<uint64_t, uint64_t>(0, 0);
  }

  template <typename T>
  std::pair<uint64_t, uint64_t> recv(std::vector<T> &data,
                                     serialiser::FlatBufSerialiser<T>) {

    std::pair<int, int> result;
    RdKafka::Message *msg = nullptr;
    auto rcv_stat = RdKafka::ERR_NO_ERROR;
    serialiser::FlatBufSerialiser<T> s;

    do {
      msg = consumer->consume(topic, partition, 1000);
      rcv_stat = msg->err();
      if ((rcv_stat != RdKafka::ERR_NO_ERROR) &&
          (rcv_stat != RdKafka::ERR__TIMED_OUT) &&
          (rcv_stat != RdKafka::ERR__PARTITION_EOF)) {
        std::cerr << "message error: " << RdKafka::err2str(msg->err())
                  << std::endl;
      }
    } while (msg->err() != RdKafka::ERR_NO_ERROR);

    uint64_t pid = -1, timestamp = -1;
    s.extract(reinterpret_cast<const char *>(msg->payload()), data, pid,
              timestamp);
    return std::pair<uint64_t, uint64_t>(pid, msg->len());
  }

private:
  std::string brokers;
  std::string topic_str;
  std::string errstr;
  std::string debug;

  int32_t partition = 0; // RdKafka::Topic::PARTITION_UA;
  int64_t start_offset = RdKafka::Topic::OFFSET_BEGINNING;

  RdKafka::Conf *conf;
  RdKafka::Conf *tconf;
  RdKafka::Consumer *consumer;
  RdKafka::Topic *topic;

  std::pair<int, int> consume_header(RdKafka::Message *message, void *opaque) {
    opaque = message->payload();
    std::copy(static_cast<char *>(message->payload()),
              static_cast<char *>(message->payload()) + message->len(),
              static_cast<char *>(opaque));
    return parse_header(std::string(static_cast<char *>(message->payload())));
  }

  template <typename T>
  void consume_data(RdKafka::Message *message, void *opaque) {
    return;
  }

  template <typename T>
  std::pair<int, int> consume_serialised(RdKafka::Message *message,
                                         void *opaque,
                                         serialiser::FlatBufSerialiser<T>) {
    std::pair<int, int> result;
    serialiser::FlatBufSerialiser<T> s;
    std::vector<T> data;
    s.extract(reinterpret_cast<const char *>(message->payload()), result.first,
              data);
    std::copy(data.begin(), data.end(), reinterpret_cast<T *>(opaque));

    return result;
  }
};

} // namespace generator
