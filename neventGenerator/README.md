kafkaGenerator                  {#mainpage}
==============

A flexible event generator. It uses two main (template) classes: ``Source``
and ``Generator`` to read data from a file, convert to event format and stream
it.

Source
====

The source can be either a NeXus file or a mcstas output. The former can be used
via the ``NeXusSource`` structure, the latter via a ``McStasSource``. Both
requires a *filename* field among the input parameter. Original data are
converted into the stream structure and direct access is provided via the
``data`` method.

Generator
======

The generator can stream data either via 0MQ (``ZmqGen``) or Kafka
(``KafkaGen``). ``ZmqGen`` requires the *port* number of the socket to open
while ``KafkaGen`` the name of the *topic* and the *brokers*
(e.g. "localhost"). Both requires the name of a *control* file that allow to
start, pause or stop the generator.  The generator is started via the ``run``
method, that requires a pointer to the data structure and the number of
elements.


``` cpp

//typedef ZmqGen generator_t;
typedef KafkaGen generator_t;

int main() {

  uparam::Param input;
  input["port"] = "1235";
  input["control"] = "control.in";
  input["topic"] = "test_0";
  input["brokers"] = "localhost";
  
  input["filename"] = "../../neventGenerator/rita22012n006190.hdf";

  NeXusSource<Rita2,int64_t> stream(input);

  Generator<generator_t,HeaderJson> g(input);

  uint64_t* d = new uint64_t[1024];
  
  for(int i =0;i<1024;++i)
    d[i] = 2*i+1;

  // read NeXus or mcstas
  NeXusSource<Rita2,int64_t> stream(input);
  
  uint64_t* d = new uint64_t[1024];
  for(int i =0;i<1024;++i)
    d[i] = 2*i+1;

  Generator<generator_t,HeaderJson> g(input);

  g.run(d,1024);

  return 0;
}
```
