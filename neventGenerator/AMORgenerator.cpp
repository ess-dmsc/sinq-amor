#include <iostream>
#include <unistd.h> // getopt

#include "generator.hpp"
#include "mcstas_reader.hpp"
#include "nexus_reader.hpp"
#include "uparam.hpp"

#include "parser.hpp"
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

using Param = uparam::Param;

Param parse(int, char **);

///////////////////////////////////////////////
///////////////////////////////////////////////
///
/// Main program for using the flexible event generator
///
///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Wed Jun 08 15:14:10 2016
int main(int argc, char **argv) {
  
  SINQAmorSim::ConfigurationParser parser;
  auto err = parser.parse_configuration(argc,argv);
  
  Param input = parse(argc, argv);
  input.print();
  Source stream(input, uparam::to_num<int>(input["multiplier"]));

  Generator<Communication, Control, Serialiser> g(input);

  int n_events = stream.count() / 2;
  g.run(&(stream.begin()[0]), n_events);

  return 0;
}

void helper(Param input) {
  std::cout
      << "AMORgenerator"
      << "\n is a neutron event generator based on a flexible library. It "
      << "supports multiple transport (kafka, 0MQ, file I/O), sources (NeXus "
         "files,"
      << "MCstas simulation output) and serialisation (no serialisation,"
      << "FlatBuffers). When executed the generator sites on the status "
         "defined in the"
      << "control file and can be driven from the command line (available "
         "commands are"
      << "'run', 'pause', 'stop')\n"
      << std::endl;

  std::cout << "-f"
            << "\t"
            << "NeXus file source [default = " << input["filename"] << "\n"
            << "-i"
            << "\t"
            << "configuration file"
            << "\n"
            << "-m"
            << "\t"
            << "data multiplier [default = " << input["multiplier"] << "\n"
            << "-r"
            << "\t"
            << "set generator status to 'run'"
            << "\n"
            << "-h"
            << "\t"
            << "this help"
            << "\n"
            << std::endl;
  exit(0);
}

/*!
 * Default arguments and command line parser
 *  \author Michele Brambilla <mib.mic@gmail.com>
 *  \date Fri Jun 17 12:20:25 2016
 */
Param parse(int argc, char **argv) {

  std::string configuration_file("config.in");
  Param input;
  input.read(configuration_file, uparam::RapidJSON());
  input["status"] = "pause";

  parser::Parser::Param p;
  parser::Parser parser;
  opterr = 0;
  int opt;
  while ((opt = getopt(argc, argv, "b:f:m:rh")) != -1) {
    switch (opt) {
    case 'b':
      parser.init(std::string(optarg));
      p = parser.get();
      break;
    case 'f':
      input["filename"] = std::string(optarg);
      break;
    case 'm':
      input["multiplier"] = std::string(optarg);
      break;
    case 'r':
      input["status"] = "run";
      break;
    case 'h':
      helper(input);
    }
  }
  if (p["host"] != "")
    input["brokers"] = p["host"];
  if (p["port"] != "")
    input["port"] = p["port"];
  if (p["topic"] != "")
    input["topic"] = p["topic"];

  return std::move(input);
}
