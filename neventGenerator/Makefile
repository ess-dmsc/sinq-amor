#SHELL = /bin/tcsh

MAKE = make

CC = /urs/bin/clang
CXX = /usr/bin/clang++

NEXUS_PATH=/afs/psi.ch/project/sinq/sl6-64
NEXUS_LIBS=-lNeXus -lNeXusCPP

FLATBUFFERS_DIR=/afs/psi.ch/project/esspsi/flatbuffers
FLATBUFFERS_INCLUDE=${FLATBUFFERS_DIR}/include
FLATBUFFERS_LIBRARY_PATH=${FLATBUFFERS_DIR}/build
FLATBUFFERS_LIB= -lflatbuffers

KAFKA_DIR=/afs/psi.ch/project/esspsi/librdkafka
KAFKA_INCLUDE=${KAFKA_DIR}/include
KAFKA_LIBRARY_PATH=${KAFKA_DIR}/lib
KAFKA_LIBS= -lrdkafka -lrdkafka++

CCFLAGS = 
CXXFLAGS = -std=c++11 -stdlib=libc++ -pthread -I. -I${KAFKA_INCLUDE} -I${NEXUS_PATH}/include -I${FLATBUFFERS_INCLUDE} -ggdb

CXXFLAGS += -D_GLIBCXX_USE_NANOSLEEP

LDFLAGS = -L${KAFKA_LIBRARY_PATH} -L${NEXUS_PATH}/lib -L/usr/lib64
LIBS = ${NEXUS_LIBS} ${KAFKA_LIBS} ${FLATBUFFERS_LIBS}

.SUFFIXES: 
.SUFFIXES: .cpp .o 

targets = AMORgenerator AMORreceiver
objects = AMORgenerator.o AMORreceiver.o

all : $(targets)

AMORgenerator: AMORgenerator.o
	$(MAKE) -C cJSON/
	$(CXX) $(CXXFLAGS) -o $@ $^ -I. -IcJSON $(LDFLAGS) -LcJSON $(LIBS) -lcjson -lzmq -lsodium

AMORreceiver: AMORreceiver.o
	$(MAKE) -C cJSON/
	$(CXX) $(CXXFLAGS) -o $@ $^ -I. -IcJSON $(LDFLAGS) -LcJSON $(LIBS) -lcjson -lzmq -lsodium

doc:
	cd docs; doxygen Doxyfile; cd ..

%.o : %.cpp generator.hpp zmq_generator.hpp kafka_generator.hpp nexus_reader.hpp mcstas_reader.hpp uparam.hpp control.hpp serialiser.hpp file_writer.hpp
	$(CXX) -c $(CXXFLAGS) $<

.PHONY : clean
clean:
	rm -f $(targets) $(objects)
