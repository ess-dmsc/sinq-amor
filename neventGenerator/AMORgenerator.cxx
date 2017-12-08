#include <iostream>

#include "generator.hpp"
#include "mcstas_reader.hpp"
#include "nexus_reader.hpp"

using StreamFormat = SINQAmorSim::ESSformat;

using Instrument = SINQAmorSim::Amor;
using Source = SINQAmorSim::NeXusSource<Instrument, StreamFormat>;
using Control = SINQAmorSim::CommandlineControl;

using Serialiser = SINQAmorSim::FlatBufferSerialiser;
using Communication = SINQAmorSim::KafkaTransmitter<Serialiser>;

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

  if(config.bytes > 0 && config.multiplier > 1) {
    std::cerr<< "Warning: conflict between parameters `bytes` and `multiplier`\n\n";
  }
  Source stream(config.source, config.multiplier);
  auto data = stream.get();
  if(config.bytes > 0 && config.multiplier > 1) {
    data.resize(config.bytes);
  }

  Generator<Communication, Control, Serialiser> g(config);

  data.resize(10);
  g.run<StreamFormat::value_type>(data);

  return 0;
}
