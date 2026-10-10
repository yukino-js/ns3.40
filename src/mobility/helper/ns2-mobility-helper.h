#ifndef NS2_MOBILITY_HELPER_H
#define NS2_MOBILITY_HELPER_H

#include "ns3/object.h"
#include "ns3/ptr.h"

#include <stdint.h>
#include <string>

namespace ns3 {

class ConstantVelocityMobilityModel;

class Ns2MobilityHelper {
public:
  Ns2MobilityHelper(std::string filename);

  void Install() const;

  template <typename T> void Install(T begin, T end) const;

private:
  class ObjectStore {
  public:
    virtual ~ObjectStore() {}

    virtual Ptr<Object> Get(uint32_t i) const = 0;
  };

  void ConfigNodesMovements(const ObjectStore &store) const;
  Ptr<ConstantVelocityMobilityModel>
  GetMobilityModel(std::string idString, const ObjectStore &store) const;
  std::string m_filename;
};

} // namespace ns3

namespace ns3 {

template <typename T> void Ns2MobilityHelper::Install(T begin, T end) const {
  class MyObjectStore : public ObjectStore {
  public:
    MyObjectStore(T begin, T end) : m_begin(begin), m_end(end) {}

    Ptr<Object> Get(uint32_t i) const override {
      T iterator = m_begin;
      iterator += i;
      if (iterator >= m_end) {
        return nullptr;
      }
      return *iterator;
    }

  private:
    T m_begin;
    T m_end;
  };

  ConfigNodesMovements(MyObjectStore(begin, end));
}

} // namespace ns3

#endif
