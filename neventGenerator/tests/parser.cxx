#include "../Configuration.hpp"


TEST(ConfigurationParser, parse_broker_port_topic){
  ConfigurationParser p;
  GTEST_EXPECT(p.parse_string_uri("hello") == 0);
  
  
}
