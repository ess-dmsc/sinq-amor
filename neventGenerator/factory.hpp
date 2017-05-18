#pragma once

#include <map>

namespace factory {

// Default error handler
template <typename IdentifierType, class ControlType>
class DefaultFactoryError {

protected:
  class Exception : public std::exception {
  public:
    Exception(const IdentifierType &id) : unknown_(id) {}
    virtual const char *what() { return "Unknown type passed to a Factory"; }
    const IdentifierType get() { return unknown_; }

  private:
    IdentifierType unknown_;
  };

public:
  static ControlType *OnUnknownType(const IdentifierType &id) {
    throw Exception(id);
  }
};

// Factory
template <typename AbstractProduct, typename IdentifierType,
          typename ProductCreator = AbstractProduct *(*)(),
          template <typename, class> class FactoryErrorPolicy =
              DefaultFactoryError>
class ObjectFactory {

public:
  AbstractProduct *create(const IdentifierType &id) {
    typename Associations::const_iterator i = this->associations_.find(id);
    if (this->associations_.end() != i) {
      return (i->second)();
    }
    return FactoryErrorPolicy<IdentifierType, AbstractProduct>::OnUnknownType(
        id);
  }

  template <class T> AbstractProduct *create() { return new T; }

  bool subscribe(const IdentifierType &id, ProductCreator creator) {
    return this->associations_.insert(typename Associations::value_type(
                                          id, creator)).second;
  }
  bool unsubscribe(const IdentifierType &id) {
    return associations_.erase(id) == 1;
  }

private:
  using Associations = std::map<IdentifierType, ProductCreator>;
  Associations associations_;
};

// ProductCreator function
template <typename Derived> typename Derived::BaseType *createInstance() {
  return new Derived;
}
};
