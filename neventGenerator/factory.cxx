#include <iostream>
#include <vector>

#include "Source.hpp"

using namespace std;

class Stooge {
public:
  using Base = Stooge;
  virtual void slap_stick() = 0;
};

class Larry : public Stooge {
public:
  void slap_stick() { cout << "Larry: poke eyes\n"; }
};
class Moe : public Stooge {
public:
  void slap_stick() { cout << "Moe: slap head\n"; }
};
class Curly : public Stooge {
public:
  void slap_stick() { cout << "Curly: suffer abuse\n"; }
};

using Factory = factory::ObjectFactory<Stooge, std::string>;

Factory::value_type *Creator(const Factory::value_id &type) {
  Factory f;
  f.subscribe("curly", factory::createInstance<Curly>);
  f.subscribe("moe", factory::createInstance<Moe>);
  f.subscribe("larry", factory::createInstance<Larry>);
  return f.create(type);
}

using Source = factory::ObjectFactory<source::Source, std::string>;
Source::value_type *GetSource(const Source::value_id &type) {
  Source f;
  f.subscribe("nexus", factory::createInstance<source::Nexus>);
  f.subscribe("mcstas", factory::createInstance<source::Mcstas>);
  return f.create(type);
}

int main() {

  Factory f;
  f.subscribe("curly", factory::createInstance<Curly>);
  f.subscribe("moe", factory::createInstance<Moe>);
  std::vector<Factory::value_type *> x;
  x.push_back(f.create("curly"));
  x.push_back(f.create("moe"));

  for (auto i : x)
    i->slap_stick();

  for (auto i : { "moe", "curly", "curly", "moe", "larry" })
    Creator(i)->slap_stick();
  return 0;
}
