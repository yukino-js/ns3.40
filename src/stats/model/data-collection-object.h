
#ifndef DATA_COLLECTION_OBJECT_H
#define DATA_COLLECTION_OBJECT_H

#include "ns3/object.h"

#include <string>

namespace ns3 {

class DataCollectionObject : public Object {
public:
  static TypeId GetTypeId();

  DataCollectionObject();
  ~DataCollectionObject() override;

  void Enable();
  void Disable();

  virtual bool IsEnabled() const;

  std::string GetName() const;

  void SetName(std::string name);

protected:
  bool m_enabled;

  std::string m_name;
};

} // namespace ns3

#endif
