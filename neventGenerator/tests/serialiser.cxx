#include "../serialiser.hpp"

#include <gtest/gtest.h>

const int data_size = 1024;

TEST(flatbuffer_serialiser, serialise_ess_format) {
  SINQAmorSim::FlatBufferSerialiser serialiser;
  std::vector<SINQAmorSim::ESSformat::value_type> data;
  data.resize(data_size);
  auto buffer_size = serialiser.serialise(1, 1, data);
  EXPECT_TRUE(buffer_size > 0);
  EXPECT_TRUE(EventMessageBufferHasIdentifier(serialiser.get()));
  EXPECT_TRUE(serialiser.verify());
}

TEST(flatbuffer_serialiser, serialise_and_unserialise_ess_format) {
  SINQAmorSim::FlatBufferSerialiser serialiser;
  std::vector<SINQAmorSim::ESSformat::value_type> input, output;
  for (int i = 0; i < data_size; ++i) {
    input.push_back(i);
  }
  auto buffer_size = serialiser.serialise(11, 37, input);

  uint64_t packet_id, timestamp;
  serialiser.extract(reinterpret_cast<const char *>(serialiser.get()), output,
                     packet_id, timestamp);
  EXPECT_TRUE(input == output);
  EXPECT_EQ(packet_id, 11);
  EXPECT_EQ(timestamp, 37);

  input[10] = -1;
  EXPECT_FALSE(input == output);
  EXPECT_NE(packet_id, 13);
  EXPECT_NE(timestamp, 91);
}

TEST(flatbuffer_serialiser, serialise_empty_array_ess_format) {
  SINQAmorSim::FlatBufferSerialiser serialiser;
  auto buffer_size =
      serialiser.serialise<SINQAmorSim::ESSformat::value_type>(1, 1);
  EXPECT_TRUE(buffer_size > 0);
  EXPECT_TRUE(EventMessageBufferHasIdentifier(serialiser.get()));
}

// TEST(flatbuffer_serialiser,serialise_empty_array_psi_format) {
//   SINQAmorSim::FlatBufferSerialiser serialiser;
//   auto buffer =
//   serialiser.serialise<SINQAmorSim::PSIformat::value_type>(1,1);
//   EXPECT_TRUE(buffer.size() > 0);
//   EXPECT_TRUE(EventMessageBufferHasIdentifier(&buffer[0]));
// }
