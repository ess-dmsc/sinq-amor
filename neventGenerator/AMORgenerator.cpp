#include <iostream>
#include <unistd.h> // getopt

#include "uparam.hpp"
#include "nexus_reader.hpp"
#include "mcstas_reader.hpp"
#include "generator.hpp"

typedef nexus::Amor Instrument;
typedef nexus::NeXusSource<Instrument> Source;
typedef control::CommandlineControl Control;


typedef serialiser::FlatBufSerialiser<uint64_t> Serialiser;
//typedef serialiser::NoSerialiser<uint64_t> Serialiser;

///////////////////////
// In the end we want to use kafka, I will use 0MQ for development purposes
typedef generator::ZmqGen<generator::transmitter> generator_t;
//typedef  generator::KafkaGen<generator::transmitter> generator_t;
//typedef FileWriterGen generator_t;


uparam::Param parse(int, char **);

///////////////////////////////////////////////
///////////////////////////////////////////////
///
/// Main program for using the flexible event generator
///
///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Wed Jun 08 15:14:10 2016
int main(int argc, char **argv) {

  uparam::Param input = parse(argc,argv);
  // default values
  
  Source stream(input);  
  Generator<generator_t,HeaderJson,Control,Serialiser> g(input);

  g.run(&(stream.begin()[0]),stream.count());

  return 0;
}




/*!
 * Default arguments and command line parser
 *  \author Michele Brambilla <mib.mic@gmail.com>
 *  \date Fri Jun 17 12:20:25 2016
 */
uparam::Param parse(int argc, char **argv) {
  uparam::Param input;
  input["port"] = "1235";
  input["control"] = "control.in";
  input["topic"] = "test_0";
  input["brokers"] = "localhost";
  input["filename"] = "sample/amor2015n001774.hdf";
  input["header"] = "header.amor";

  opterr = 0;
  int opt;
  while ((opt = getopt (argc, argv, "a:b:c:f:p:s:t:e:")) != -1) {
    switch (opt) {
    case 'a': //area
      input["2D"] = std::string(optarg);
      break;
    case 'b':
      input["brokers"] = std::string(optarg);
      break;
    case 'c':
      input["control"] = std::string(optarg);
      break;
    case 'f':
      input["filename"] = std::string(optarg);
      break;
    case 'p':
      input["port"] = std::string(optarg);
      break;
    case 's': // single dimension detector
      input["1D"] = std::string(optarg);
      break;
    case 't':
      input["topic"] = std::string(optarg);
      break;
    case 'e':
      input["header"] = std::string(optarg);
      break;
    case '?':
      // TODO help
      exit(0);
    }
  }

  return std::move(input);
}
