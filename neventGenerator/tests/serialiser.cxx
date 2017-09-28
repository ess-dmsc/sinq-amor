#include "../serialiser.hpp"

#include <gtest/gtest.h>

const int data_size = 1024;

TEST(flatbuffer_serialiser,serialise_ess_format) {
  SINQAmorSim::FlatBufferSerialiser serialiser;
  std::vector<SINQAmorSim::ESSformat::value_type> data;
  data.resize(data_size);
  auto buffer = serialiser.serialise(1,1,data);
  EXPECT_TRUE(buffer.size() > 0);
  EXPECT_TRUE(EventMessageBufferHasIdentifier(&buffer[0]));
  EXPECT_TRUE(serialiser.verify());
}

TEST(flatbuffer_serialiser,serialise_and_unserialise_ess_format) {
  SINQAmorSim::FlatBufferSerialiser serialiser;
  std::vector<SINQAmorSim::ESSformat::value_type> input,output;
  for(int i=0;i<data_size;++i) {
    input.push_back(i);
  }
  auto buffer = serialiser.serialise(11,37,input);
  EXPECT_TRUE(buffer.size() > 0);
  EXPECT_TRUE(EventMessageBufferHasIdentifier(&buffer[0]));
  EXPECT_TRUE(serialiser.verify(buffer));

  uint64_t packet_id,timestamp;
  serialiser.extract(buffer,output,packet_id,timestamp);
  EXPECT_EQ(input,output);
  EXPECT_EQ(packet_id,11);
  EXPECT_EQ(timestamp,37);

  input[10] = -1;
  EXPECT_NE(input,output);
  EXPECT_NE(packet_id,13);
  EXPECT_NE(timestamp,91);
}

TEST(flatbuffer_serialiser,serialise_empty_array_ess_format) {
  SINQAmorSim::FlatBufferSerialiser serialiser;
  auto buffer = serialiser.serialise<SINQAmorSim::ESSformat::value_type>(1,1);
  EXPECT_TRUE(buffer.size() > 0);
  EXPECT_TRUE(EventMessageBufferHasIdentifier(&buffer[0]));
}


// TEST(flatbuffer_serialiser,serialise_empty_array_psi_format) {
//   SINQAmorSim::FlatBufferSerialiser serialiser;
//   auto buffer = serialiser.serialise<SINQAmorSim::PSIformat::value_type>(1,1);
//   EXPECT_TRUE(buffer.size() > 0);
//   EXPECT_TRUE(EventMessageBufferHasIdentifier(&buffer[0]));
// }
