#include <iostream>

#include "Configuration.hpp"
#include "generator.hpp"
#include "mcstas_reader.hpp"
#include "nexus_reader.hpp"

typedef nexus::Amor Instrument;
typedef nexus::NeXusSource<Instrument, nexus::PSIformat> Source;

typedef control::NoControl Control;

typedef serialiser::FlatBufSerialiser<uint64_t> Serialiser;
// typedef serialiser::NoSerialiser<uint64_t> Serialiser;

///////////////////////
// In the end we want to use kafka, I will use 0MQ for development purposes
// typedef ZmqRecv Transport;
// typedef generator::ZmqGen<generator::receiver> Transport;
typedef generator::KafkaListener<generator::receiver> Transport;
// typedef FileWriterGen Transport

int main(int argc, char **argv) {

  SINQAmorSim::ConfigurationParser parser;
  auto err = parser.parse_configuration(argc, argv);
  if (!err) {
    parser.print();
  } else {
    std::cout << SINQAmorSim::Err2Str(err) << "\n";
    return -1;
  }
  auto &config = parser.config;

  Generator<Transport, Control, Serialiser> g(config.producer.broker,
                                              config.producer.topic);

  std::vector<Source::value_type> stream;
  g.listen(stream);

  return 0;
}
