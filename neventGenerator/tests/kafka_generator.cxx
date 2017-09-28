#include <../kafka_generator.hpp>

#include <gtest/gtest.h>

using DummyTransmitter = SINQAmorSim::KafkaTransmitter<SINQAmorSim::NoSerialiser>;

TEST(kafka_generator, missing_broker_and_or_topic) {
  EXPECT_ANY_THROW(DummyTransmitter("",""));
  EXPECT_ANY_THROW(DummyTransmitter("localhost:9092",""));
  EXPECT_ANY_THROW(DummyTransmitter("","topic-name"));
  EXPECT_NO_THROW(DummyTransmitter("localhost:9092","topic-name"));
}

//TEST(kafka_generator, options) { }

// TEST(kafka_generator, send_message) {
//   auto source = DummyTransmitter("localhost:9092","topic-name");

//   std::vector<uint64_t> data{32};
//   EXPECT_EQ(source.send(data),RdKafka::ERR__TIMED_OUT);
// }
