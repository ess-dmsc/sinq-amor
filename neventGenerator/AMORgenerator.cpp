#include <iostream>

#include "generator.hpp"
#include "mcstas_reader.hpp"
#include "nexus_reader.hpp"

#include "Configuration.hpp"

using StreamFormat = nexus::ESSformat;

using Instrument = nexus::Amor;
using Source = nexus::NeXusSource<Instrument, StreamFormat>;
using Control = control::CommandlineControl;
using Serialiser = serialiser::FlatBufSerialiser<StreamFormat::value_type>;
// typedef serialiser::NoSerialiser<uint64_t> Serialiser;

// typedef generator::ZmqGen<generator::transmitter> generator_t;
using Communication = generator::KafkaGen<generator::transmitter>;
// typedef FileWriterGen generator_t;

///////////////////////////////////////////////
///
/// Main program for using the flexible event generator
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
  Generator<Communication, Control, Serialiser> g(config.producer.broker,
                                                  config.producer.topic);

  int n_events = stream.count() / 2;
  g.run(&(stream.begin()[0]), n_events);

  return 0;
}
