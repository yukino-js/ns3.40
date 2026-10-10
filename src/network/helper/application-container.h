
#ifndef APPLICATION_CONTAINER_H
#define APPLICATION_CONTAINER_H

#include "ns3/application.h"
#include "ns3/random-variable-stream.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class ApplicationContainer {
public:
  ApplicationContainer();

  ApplicationContainer(Ptr<Application> application);

  ApplicationContainer(std::string name);

  typedef std::vector<Ptr<Application>>::const_iterator Iterator;

  Iterator Begin() const;

  Iterator End() const;

  uint32_t GetN() const;

  Ptr<Application> Get(uint32_t i) const;

  void Add(ApplicationContainer other);

  void Add(Ptr<Application> application);

  void Add(std::string name);

  void Start(Time start) const;

  void StartWithJitter(Time start, Ptr<RandomVariableStream> rv) const;

  void Stop(Time stop) const;

private:
  std::vector<Ptr<Application>> m_applications;
};

} // namespace ns3

#endif
