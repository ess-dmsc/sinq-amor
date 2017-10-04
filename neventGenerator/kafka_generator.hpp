#pragma once

#include <cctype>
#include <chrono>
#include <sstream>
#include <string>
#include <utility>

#include <librdkafka/rdkafkacpp.h>

#include "header.hpp"
#include "serialiser.hpp"

namespace SINQAmorSim {

using GeneratorOptions = std::vector<std::pair<std::string, std::string>>;

uint64_t timestamp_now() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

struct KafkaGeneratorInfo {
  double Mbytes{0};
  double messages{0};
};

class DeliveryReport : public RdKafka::DeliveryReportCb {
public:
  void dr_cb(RdKafka::Message &message) {
    if (message.errstr() == "Success") {
      info.messages++;
      info.Mbytes += message.len() * 1e-6;
    } else {
      std::cout << message.errstr() << std::endl;
    }
  }

  double &messages() { return info.messages; }
  double &Mbytes() { return info.Mbytes; }

private:
  KafkaGeneratorInfo info;
};

////////////////
// Producer

template <class Serialiser> class KafkaTransmitter {

public:
  KafkaTransmitter(const std::string &brokers, const std::string &topic_str,
                   const GeneratorOptions &options = {})
      : topic_name{topic_str} {
    if (brokers.empty() || topic_name.empty()) {
      throw std::runtime_error("Broker and/or topic not set");
    }

    std::unique_ptr<RdKafka::Conf> conf{
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)};
    if (!conf) {
      throw std::runtime_error("Unable to create RdKafka::Conf");
    }

    std::string errstr;
    std::string debug;
    conf->set("metadata.broker.list", brokers, errstr);
    if (!debug.empty()) {
      if (conf->set("debug", debug, errstr) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error(errstr);
      }
    }
    conf->set("message.max.bytes", "131001000", errstr);
    if (!errstr.empty()) {
      std::cerr << errstr << std::endl;
    }
    conf->set("api.version.request", "true", errstr);
    if (!errstr.empty()) {
      std::cerr << errstr << std::endl;
    }
    conf->set("dr_cb", &dr_cb, errstr);

    producer.reset(RdKafka::Producer::create(conf.get(), errstr));
    if (!producer) {
      throw std::runtime_error("Failed to create producer: " + errstr);
    }

    RdKafka::Metadata *md;
    auto err = producer->metadata(1, nullptr, &md, 1000);
    if (err == RdKafka::ERR__TIMED_OUT) {
      throw std::runtime_error("Failed to retrieve metadata: " + errstr);
    }
    metadata.reset(md);
  }

  template <typename T> size_t send(std::vector<T> &data, const int nev = -1) {
    RdKafka::ErrorCode resp = producer->produce(
        topic_name, RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY, &data[0], data.size() * sizeof(T),
        nullptr, 0, timestamp_now(), nullptr);
    return int(producer->poll(10));
  }

  template <typename T>
  size_t send(const uint64_t &, const uint64_t &, std::vector<T> &,
              const int = 1) {
    return 0;
  }

  int poll(const int &seconds = -1) { return producer->poll(seconds); }

  double &messages() { return dr_cb.messages(); }
  double &Mbytes() { return dr_cb.Mbytes(); }

private:
  std::unique_ptr<RdKafka::Metadata> metadata{nullptr};
  std::unique_ptr<RdKafka::Producer> producer{nullptr};
  std::string topic_name;

  DeliveryReport dr_cb;
  Serialiser serialiser;
};

template <typename T> class TD;

template <>
template <typename T>
size_t KafkaTransmitter<FlatBufferSerialiser>::send(const uint64_t &pid,
                                                    const uint64_t &timestamp,
                                                    std::vector<T> &data,
                                                    const int nev) {
  if (nev) {
    auto buffer = serialiser.serialise(pid, timestamp, data);
    RdKafka::ErrorCode resp = producer->produce(
        topic_name, RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY,
        reinterpret_cast<void *>(serialiser.get()), serialiser.size(), nullptr,
        0, timestamp_now(), nullptr);
  }
  return 0;
}

////////////////
// Consumer

template <class Serialiser> struct KafkaListener {
  static const int max_header_size = 10000;

  KafkaListener(const std::string &broker_, const std::string &topic_)
      : brokers(broker_), topic_str(topic_) {
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
  int recv(std::string &h, std::vector<T> &data, SINQAmorSim::NoSerialiser) {
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
           SINQAmorSim::FlatBufferSerialiser) {
    void *value;
    std::pair<int, int> result;
    RdKafka::Message *msg = nullptr;
    do {
      msg = consumer->consume(topic, partition, 1000);
      std::cout << RdKafka::err2str(msg->err()) << std::endl;
    } while (msg->err() != RdKafka::ERR_NO_ERROR);
    result =
        consume_serialised<T>(msg, value, SINQAmorSim::FlatBufferSerialiser());
    h = std::string(static_cast<char *>(msg->payload()));
  }

  template <typename T>
  std::pair<uint64_t, uint64_t> recv(std::vector<T> &data,
                                     SINQAmorSim::NoSerialiser) {
    return std::pair<uint64_t, uint64_t>(0, 0);
  }

  template <typename T>
  std::pair<uint64_t, uint64_t> recv(std::vector<T> &data,
                                     SINQAmorSim::FlatBufferSerialiser) {

    std::pair<int, int> result;
    RdKafka::Message *msg = nullptr;
    auto rcv_stat = RdKafka::ERR_NO_ERROR;
    SINQAmorSim::FlatBufferSerialiser s;

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

  template <typename T>
  std::pair<uint64_t, uint64_t> recv(std::vector<T> &data) {

    std::pair<int, int> result;
    RdKafka::Message *msg = nullptr;
    auto rcv_stat = RdKafka::ERR_NO_ERROR;
    Serialiser s;

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
                                         SINQAmorSim::FlatBufferSerialiser) {
    std::pair<int, int> result;
    SINQAmorSim::FlatBufferSerialiser s;
    std::vector<T> data;
    s.extract(reinterpret_cast<const char *>(message->payload()), result.first,
              data);
    std::copy(data.begin(), data.end(), reinterpret_cast<T *>(opaque));

    return result;
  }
};

} // namespace SINQAmorSim
