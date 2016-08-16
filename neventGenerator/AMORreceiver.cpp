#include <iostream>
#include <unistd.h> // getopt

#include "uparam.hpp"
#include "nexus_reader.hpp"
#include "mcstas_reader.hpp"
#include "generator.hpp"

typedef nexus::Amor Instrument;
typedef nexus::NeXusSource<Instrument> Source;

typedef control::NoControl Control;


typedef serialiser::FlatBufSerialiser<uint64_t> Serialiser;
//typedef serialiser::NoSerialiser<uint64_t> Serialiser;

///////////////////////
// In the end we want to use kafka, I will use 0MQ for development purposes
//typedef ZmqRecv Transport;
//typedef generator::ZmqGen<generator::receiver> Transport;
typedef generator::KafkaListener<generator::receiver> Transport;
//typedef FileWriterGen Transport

typedef uparam::Param Param;


Param parse(int, char **);
int main(int argc, char **argv) {

  Param input = parse(argc,argv);
  
  Generator<Transport,Control,Serialiser> g(input);

  std::vector<uint64_t> stream;
  g.listen(stream);

  return 0;
}




/*!
 * Default arguments and command line parser
 *  \author Michele Brambilla <mib.mic@gmail.com>
 *  \date Fri Jun 17 12:20:25 2016
 */
Param parse(int argc, char **argv) {
  Param input;
  // default values
  input.read("config.in",uparam::RapidJSON());

  opterr = 0;
  int opt;
  while ((opt = getopt (argc, argv, "a:b:c:f:p:s:t:e:o:")) != -1) {
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
    case 'o':
      input["host"] = std::string(optarg);
      break;
    case '?':
      // TODO help
      exit(0);
    }
  }

  return std::move(input);
}
