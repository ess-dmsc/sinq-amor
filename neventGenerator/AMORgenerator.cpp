#include <iostream>
#include <unistd.h> // getopt

#include "uparam.hpp"
#include "nexus_reader.hpp"
#include "mcstas_reader.hpp"
#include "generator.hpp"

#include "parser.hpp"

using StreamFormat = nexus::ESSformat;

typedef nexus::Amor Instrument;

typedef nexus::NeXusSource<Instrument,StreamFormat> Source;
typedef control::CommandlineControl Control;

typedef serialiser::FlatBufSerialiser<StreamFormat::value_type> Serialiser;
//typedef serialiser::NoSerialiser<uint64_t> Serialiser;

///////////////////////
// In the end we want to use kafka, I will use 0MQ for development purposes
//typedef generator::ZmqGen<generator::transmitter> generator_t;
typedef  generator::KafkaGen<generator::transmitter> Communication;
//typedef FileWriterGen generator_t;

typedef uparam::Param Param;

Param parse(int, char **);

///////////////////////////////////////////////
///////////////////////////////////////////////
///
/// Main program for using the flexible event generator
///
///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Wed Jun 08 15:14:10 2016
int main(int argc, char **argv) {

  Param input = parse(argc,argv);
  input.print();
  Source stream(input,uparam::to_num<int>(input["multiplier"]));
   
  Generator<Communication,Control,Serialiser> g(input);

  std::cout << stream.count() << std::endl;
  g.run(&(stream.begin()[0]),stream.count());

  return 0;
}



void helper(Param input) {
  std::cout << "AMORgenerator" << "\n is a neutron event generator based on a flexible library. It "
            << "supports multiple transport (kafka, 0MQ, file I/O), sources (NeXus files,"
            << "MCstas simulation output) and serialisation (no serialisation,"
            << "FlatBuffers). When executed the generator sites on the status defined in the"
            << "control file and can be driven from the command line (available commands are"
            << "'run', 'pause', 'stop')\n"
            << std::endl;

  std::cout << "Usage example:\n"
            << "[0MQ]\t./AMORgenerator -p 1234 -f sample/amor2015n001774.hdf\n"
            << "[kafka]\t./AMORgenerator -b ess01 -t test_0\n"
            << std::endl;
  
  std::cout << "-a" << "\t" << "area detector source file (mcstas)" << "\n"
            << "-c" << "\t" << "control file (when use file)" << "\n"
            << "-f" << "\t" << "NeXus file source [default = " << input["filename"] << "\n"
            << "-e" << "\t" << "header template [default = " << input["header"] << "\n"
            << "-i" << "\t" << "configuration file" << "\n"
            << "-m" << "\t" << "data multiplier [default = " << input["multiplier"] << "\n"
            << "-r" << "\t" << "set generator status to 'run'" << "\n"
            << "-s" << "\t" << "1D detector source file (mcstas)" << "\n"
            << "-h" << "\t" << "this help" << "\n"
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
  input.read(configuration_file,uparam::RapidJSON());
  input["status"] = "pause";

  parser::Parser::Param p;
  {
    parser::Parser parser;
    parser.init(std::string(argv[1]));
    p=parser.get();
  }
  if( p["host"] != "" )  input["brokers"]=p["host"];
  if( p["port"] != "" )  input["port"]=p["port"];
  if( p["topic"] != "" ) input["topic"]=p["topic"];

  opterr = 0;
  int opt;
  while ((opt = getopt (argc, argv, "a:c:f:s:e:m:rh")) != -1) {
    switch (opt) {
    case 'a': //area
      input["2D"] = std::string(optarg);
      break;
    case 'c':
      input["control"] = std::string(optarg);
      break;
    case 'f':
      input["filename"] = std::string(optarg);
      break;
    case 's': // single dimension detector
      input["1D"] = std::string(optarg);
      break;
    case 'e':
      input["header"] = std::string(optarg);
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



  return std::move(input);
}
