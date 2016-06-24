#SHELL = /bin/tcsh

MAKE = make

CC = gcc
CXX = g++

NEXUS_PATH=/afs/psi.ch/project/sinq/sl6-64

CCFLAGS = 
CXXFLAGS = -std=c++11 -pthread -I. -I${KAFKA_INCLUDE} -I${NEXUS_PATH}/include -I${FLATBUFFERS_INCLUDE}

CXXFLAGS += -D_GLIBCXX_USE_NANOSLEEP

LDFLAGS = -L${KAFKA_LIBRARY_PATH} -L${NEXUS_PATH}/lib
LIBS = -lrdkafka -lrdkafka++ -lNeXus -lNeXusCPP -L${FLATBUFFERS_LIBRARY_PATH} ${FLATBUFFERS_LIBS}

.SUFFIXES: 
.SUFFIXES: .cpp .o 

targets = AMORgenerator
objects = AMORgenerator.o

all : $(targets)

AMORgenerator: AMORgenerator.o
	$(MAKE) -C cJSON/
	$(CXX) $(CXXFLAGS) -o $@ $^ -I. -IcJSON $(LDFLAGS) -LcJSON $(LIBS) -lcjson -lzmq -lsodium

doc:
	cd docs; doxygen Doxyfile; cd ..

%.o : %.cpp generator.hpp zmq_generator.hpp kafka_generator.hpp nexus_reader.hpp mcstas_reader.hpp uparam.hpp control.hpp serialiser.hpp file_writer.hpp
	$(CXX) -c $(CXXFLAGS) $<

.PHONY : clean
clean:
	rm -f $(targets) $(objects)
