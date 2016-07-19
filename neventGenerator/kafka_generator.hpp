#ifndef _KAFKA_GENERATOR_H
#define _KAFKA_GENERATOR_H

#include <string>
#include <sstream>
#include <utility>
#include <cctype>

#include <assert.h>

#include <librdkafka/rdkafkacpp.h>

#include "uparam.hpp"

const int max_message_size = 100000000;

namespace generator {

  enum {transmitter,receiver};
  /*! \struct KafkaGen
   *  Uses Kafka as the streamer
   *
   *  \author Michele Brambilla <mib.mic@gmail.com>
   *  \date Wed Jun 08 15:19:16 2016
   */
  template<int mode_selector>
  struct KafkaGen {
  
    KafkaGen(uparam::Param p) : brokers(p["brokers"]), topic_str(p["topic"]) {
      /*! @param p see uparam::Param for description. Must contain "brokers" and "topic" key-value */
      /*! Connects to the "broker" clients of the Kafka messaging system. Streams data to the "topic" topic.
       */
      conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
      tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

      conf->set("metadata.broker.list", brokers, errstr);
      if (!debug.empty()) {
        if (conf->set("debug", debug, errstr) != RdKafka::Conf::CONF_OK) {
          std::cerr << errstr << std::endl;
          exit(1);
        }
      }

      std::string err_str;
      conf->set("message.max.bytes", "100000000", err_str);
      std::cerr << err_str << std::endl;

      if(topic_str.empty()) {
        std::cerr << "Topic not set" << std::endl;
        exit(1);
      }
      
      producer = RdKafka::Producer::create(conf, errstr);
      if (!producer) {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
        exit(1);
      }
      std::cout << "% Created producer " << producer->name() << std::endl;
      
      topic = RdKafka::Topic::create(producer, topic_str,
                                     tconf, errstr);
      if (!topic) {
        std::cerr << "Failed to create topic: " << errstr << std::endl;
        exit(1);
      }
    }

    template<typename T>
    void send(T* data,const int size, const int flag = 0) {
      /*! @param data data to be sent
       *  @param size number of elements
       *  @param flag optional flag (unused) */
      RdKafka::ErrorCode resp =
        producer->produce(topic, partition,
                          RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                          data, size*sizeof(T),
                          NULL, NULL);
      if (resp != RdKafka::ERR_NO_ERROR)
        std::cerr << "% Produce failed: " << RdKafka::err2str(resp) << std::endl;
      std::cerr << "% Produced message (" << sizeof(data) 
                << " bytes)" 
                << std::endl;
    }

    template<typename T>
    void send(std::string h, T* data, int nev, serialiser::NoSerialiser<T>) {
      RdKafka::ErrorCode resp =
        producer->produce(topic, partition,
                          RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                          (void*)h.c_str(), h.size(),
                          NULL, NULL);
      if (resp != RdKafka::ERR_NO_ERROR)
        std::cerr << "% Produce failed header: " <<
          RdKafka::err2str(resp) << std::endl;
      if( nev > 0 ) {
        resp =
          producer->produce(topic, partition,
                            RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                            (void*)data, nev*sizeof(T),
                            NULL, NULL);
        
        if (resp != RdKafka::ERR_NO_ERROR)
          std::cerr << "% Produce failed data: " <<
            RdKafka::err2str(resp) << std::endl;
      }
      
      std::cerr << "% Produced message (" << h.size()+sizeof(T)*nev 
                << " bytes)" 
                << std::endl;
    }
    
    
    template<typename T>
    void send(std::string h, T* data, int nev, serialiser::FlatBufSerialiser<T>) {
      
      serialiser::FlatBufSerialiser<T> s;
      s(h,data,nev);
      
      RdKafka::ErrorCode resp =
        producer->produce(topic, partition,
                          RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                          (void*)s(), s.size(),
                          NULL, NULL);
      if (resp != RdKafka::ERR_NO_ERROR)
        std::cerr << "% Produce failed: " <<
          RdKafka::err2str(resp) << std::endl;
      
      std::cerr << "% Produced message (" << h.size()+sizeof(T)*nev 
                << " bytes)" 
                << std::endl;
    }
    
    
    
  private:
    std::string brokers;
    std::string topic_str;
    std::string errstr;
    std::string debug;
    
    int32_t partition = RdKafka::Topic::PARTITION_UA;
    int64_t start_offset = RdKafka::Topic::OFFSET_BEGINNING;
    
    RdKafka::Conf *conf;
    RdKafka::Conf *tconf;
    RdKafka::Producer *producer;
    RdKafka::Topic *topic;
    
  };


  
  std::pair<int,int> msg_consume(RdKafka::Message* message, void* opaque) {
    
    std::pair<int,int> result;
    
    switch (message->err()) {
    case RdKafka::ERR__TIMED_OUT:
      break;
      
    case RdKafka::ERR_NO_ERROR:
      /* Real message */
      std::cout << "Read msg at offset " << message->offset() << std::endl;
      if (message->key()) {
        std::cout << "Key: " << message->key() << "\t";
      }
      std::cout << "Len: " << message->len() << "\n";
      if( static_cast<char*>(message->payload())[0] == '{' ) {
        std::cout << "Payload: " << static_cast<char*>(message->payload())[0] << "\n";
        
        cJSON* root = NULL;
        root = cJSON_Parse(static_cast<char*>(message->payload()));
        if( root == 0 ) {
          //      throw std::runtime_error("can't parse header");
          std::cout << "Error: can't parse header" << std::endl;
          exit(-1);
        }
        
        cJSON* item = cJSON_GetObjectItem(root,"ds");
        result.first = cJSON_GetObjectItem(root,"pid")->valuedouble;
        result.second = cJSON_GetArrayItem(item,1) -> valuedouble;
      }
      
      else
        for(int i=0;i<10;++i)
          std::cout << "Payload: " << static_cast<uint64_t*>(message->payload())[i] << "\n";
      
      
      break;
      
    case RdKafka::ERR__PARTITION_EOF:
      /* Last message */
      //    if (exit_eof) {
      std::cerr << "Consume failed: " << message->errstr() << std::endl;
      //      run = false;
      //    }
      break;
      
    case RdKafka::ERR__UNKNOWN_TOPIC:
    case RdKafka::ERR__UNKNOWN_PARTITION:
      std::cerr << "Consume failed: " << message->errstr() << std::endl;
      //    run = false;
      break;
      
    default:
      /* Errors */
      std::cerr << "Consume failed: " << message->errstr() << std::endl;
      //    run = false;
    }
    
    return result;
  }
  
  
  std::pair<int,int> msg_consume1(RdKafka::Message* message, void* opaque) {
    
    std::pair<int,int> result;
    char *head;
    uint64_t *stream;
    serialiser::FlatBufSerialiser<uint64_t> s;
    
    switch (message->err()) {
    case RdKafka::ERR__TIMED_OUT:
      break;
      
    case RdKafka::ERR_NO_ERROR:
      /* Real message */
      std::cout << "Read msg at offset " << message->offset() << std::endl;
      if (message->key()) {
        std::cout << "Key: " << message->key() << "\t";
      }
      std::cout << "Len: " << message->len() << "\n";

      head = new char[1000];
      stream = new uint64_t[100000];
      s.extract(static_cast<char*>(message->payload()),head,stream);
      std::cout << head << std::endl;


      // if( static_cast<char*>(message->payload())[0] == '{' ) {
      //   std::cout << "Payload: " << static_cast<char*>(message->payload())[0] << "\n";

      //   cJSON* root = NULL;
      //   root = cJSON_Parse(static_cast<char*>(message->payload()));
      //   if( root == 0 ) {
      //     //      throw std::runtime_error("can't parse header");
      //     std::cout << "Error: can't parse header" << std::endl;
      //     exit(-1);
      //   }
      
      //   cJSON* item = cJSON_GetObjectItem(root,"ds");
      //   result.first = cJSON_GetObjectItem(root,"pid")->valuedouble;
      //   result.second = cJSON_GetArrayItem(item,1) -> valuedouble;
      // }
      
      // else
      //   for(int i=0;i<10;++i)
      //     std::cout << "Payload: " << static_cast<uint64_t*>(message->payload())[i] << "\n";
    
    
      break;
    
    case RdKafka::ERR__PARTITION_EOF:
      /* Last message */
      //    if (exit_eof) {
      std::cerr << "Consume failed: " << message->errstr() << std::endl;
      //      run = false;
      //    }
      break;
    
    case RdKafka::ERR__UNKNOWN_TOPIC:
    case RdKafka::ERR__UNKNOWN_PARTITION:
      std::cerr << "Consume failed: " << message->errstr() << std::endl;
      //    run = false;
      break;
    
    default:
      /* Errors */
      std::cerr << "Consume failed: " << message->errstr() << std::endl;
      //    run = false;
    }

    return result;
  }


  class ExampleConsumeCb : public RdKafka::ConsumeCb {
  public:
    std::pair<int,int> info;
    void consume_cb (RdKafka::Message &msg, void *opaque) {
      info = msg_consume(&msg, opaque);
    }
  };

  class SerialisedConsumeCb : public RdKafka::ConsumeCb {
  public:
    std::pair<int,int> info;
    void consume_cb (RdKafka::Message &msg, void *opaque) {
      info = msg_consume(&msg, opaque);
    }
  };
  


  struct KafkaListener {
    static const int max_header_size=10000;
    
    KafkaListener(uparam::Param p) : brokers(p["brokers"]), topic_str(p["topic"]) {
      /*! @param p see uparam::Param for description. Must contain "brokers" and "topic" key-value */
      /*! Connects to the "broker" clients of the Kafka messaging system. Streams data to the "topic" topic.
       */
      conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
      tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
      
      conf->set("metadata.broker.list", brokers, errstr);
      if (!debug.empty()) {
        if (conf->set("debug", debug, errstr) != RdKafka::Conf::CONF_OK) {
          std::cerr << errstr << std::endl;
          exit(1);
        }
      }
      
      std::string err_str;
      conf->set("fetch.message.max.bytes", "100000000", err_str);
      std::cerr << err_str << std::endl;
      
      if(topic_str.empty()) {
        //TODO
      }
      
      consumer = RdKafka::Consumer::create(conf, errstr);
      if (!consumer) {
        std::cerr << "Failed to create consumer: " << errstr << std::endl;
        exit(1);
      }
      std::cout << "% Created consumer " << consumer->name() << std::endl;
      
      topic = RdKafka::Topic::create(consumer, topic_str,
                                     tconf, errstr);
      if (!topic) {
        std::cerr << "Failed to create topic: " << errstr << std::endl;
        exit(1);
      }
      
      /*
       * Start consumer for topic+partition at start offset
       */
      RdKafka::ErrorCode resp = consumer->start(topic, partition, start_offset);
      if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Failed to start consumer: " <<
          RdKafka::err2str(resp) << std::endl;
        exit(1);
      }
      
    }


    template<typename T>
    int recv(std::string& h, std::vector<T>& data, serialiser::NoSerialiser<T>) {
      
      ExampleConsumeCb ex_consume_cb;
      int use_ccb = 1;
      /*
       * Consume messages
       */
      //  while (true) {
      consumer->poll(0);
      if (use_ccb) {
        consumer->consume_callback(topic, partition, 1000,
                                   &ex_consume_cb,&use_ccb);
      } else {
        RdKafka::Message *msg = consumer->consume(topic, partition, 1000);
        msg_consume(msg, NULL);
        delete msg;
      }
      //  }

      // RdKafka::Message *msg = consumer->consume(topic, partition, 1000);
      // std::cout << "Read msg at offset " << msg->offset() << std::endl;
      // if (msg->key()) {
      //   std::cout << "Key: " << *msg->key() << "\t";
      // }
  
      // std::cout << "Len: " << static_cast<int>(msg->len()) << std::endl;
      // consumer->poll(0);
  
      std::cout << "pid = " << ex_consume_cb.info.first << "\t" << "nev = " << ex_consume_cb.info.second << std::endl;
      return ex_consume_cb.info.first;
  
    }



    template<typename T>
    int  recv(std::string& h, std::vector<T>& data, serialiser::FlatBufSerialiser<T>) {

      SerialisedConsumeCb ex_consume_cb;
      int use_ccb = 1;
      /*
       * Consume messages
       */
      //  while (true) {
      consumer->poll(0);
      if (use_ccb) {
        consumer->consume_callback(topic, partition, 1000,
                                   &ex_consume_cb,&use_ccb);
      } else {
        RdKafka::Message *msg = consumer->consume(topic, partition, 1000);
        msg_consume(msg, NULL);
        delete msg;
      }
      //  }
  
      std::cout << "pid = " << ex_consume_cb.info.first << "\t" << "nev = " << ex_consume_cb.info.second << std::endl;
      return ex_consume_cb.info.first;


      return ex_consume_cb.info.first;
    }



  private:

    std::string brokers;
    std::string topic_str;
    std::string errstr;
    std::string debug;
  
    int32_t partition = 0;//RdKafka::Topic::PARTITION_UA;
    int64_t start_offset = RdKafka::Topic::OFFSET_BEGINNING;
  
    RdKafka::Conf *conf;
    RdKafka::Conf *tconf;
    RdKafka::Consumer *consumer;
    RdKafka::Topic *topic;
  

  
  
  };


} // namespace generator






#endif //KAFKA_GENERATOR_H
