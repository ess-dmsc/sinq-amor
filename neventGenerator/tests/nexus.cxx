#include "../nexus_reader.hpp"
#include <gtest/gtest.h>

extern std::string source_dir;

TEST(NexusReader,file_not_found) {  
  EXPECT_ANY_THROW(NeXus::File("dummy.hdf"));
}

TEST(NexusReader,open_file) {
  auto nexusfile = source_dir + "/../files/amor2015n001774.hdf";
  EXPECT_NO_THROW(NeXus::File(nexusfile.c_str()));
}

TEST(NexusReader,long_test) {
  auto nexusfile = source_dir + "/../files/amor2015n001774.hdf";
  // auto file = NeXus::File(nexusfile.c_str());

  // std::vector<NeXus::AttrInfo> attr_infos = file.getAttrInfos();
  // file.openGroup("entry", "NXentry");

  // std::cout  << "Number of global attributes: " << attr_infos.size() << "\n";
  // for (std::vector<NeXus::AttrInfo>::iterator it = attr_infos.begin();
  //      it != attr_infos.end(); it++) {
  //   if (it->name != "file_time" && it->name != "HDF_version" && it->name !=  "HDF5_Version" && it->name != "XML_version") {
  //     std::cout  << "   " << it->name << " = ";
  //     // if (it->type == NeXus::CHAR) {
  //     //   std::cout << file.getStrAttr(*it);
  //     // }
  //     std::cout << "\n";
  //   }
  // }


}
