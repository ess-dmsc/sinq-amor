#include <iostream>

#include "Configuration.hpp"
#include "generator.hpp"
#include "mcstas_reader.hpp"
#include "nexus_reader.hpp"

using Instrument = SINQAmorSim::Amor;
using Source = SINQAmorSim::NeXusSource<Instrument, SINQAmorSim::PSIformat>;
using Control = control::NoControl;
using Serialiser = SINQAmorSim::FlatBufferSerialiser;

///////////////////////
// In the end we want to use kafka, I will use 0MQ for development purposes
// typedef ZmqRecv Transport;
// typedef generator::ZmqGen<generator::receiver> Transport;
typedef SINQAmorSim::KafkaListener Transport;
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

  Generator<Transport, Control, Serialiser> g(config);

  std::vector<Source::value_type> stream;
  g.listen(stream);

  return 0;
}
