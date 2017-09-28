#include <iostream>

#include "generator.hpp"
#include "mcstas_reader.hpp"
#include "nexus_reader.hpp"

using StreamFormat = SINQAmorSim::ESSformat;

using Instrument = SINQAmorSim::Amor;
using Source = SINQAmorSim::NeXusSource<Instrument, StreamFormat>;
using Control = control::CommandlineControl;
using Serialiser = SINQAmorSim::FlatBufferSerialiser;
using TimeStamp = ConstTimestamp;

using Communication = SINQAmorSim::KafkaTransmitter<Serialiser>;

///////////////////////////////////////////////
///
/// Main program for using the event generator
///
///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Wed Jun 08 15:14:10 2016
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

  Source stream(config.source, config.multiplier);
  Generator<Communication, Control, Serialiser> g(config);
  
  int n_events = stream.count() / 2;
  g.run<StreamFormat::value_type,TimeStamp>(&(stream.begin()[0]), n_events);
  
  return 0;
}
