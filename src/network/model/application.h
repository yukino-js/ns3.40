
#ifndef APPLICATION_H
#define APPLICATION_H

#include "node.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

namespace ns3 {

class Node;

class Application : public Object {
public:
  static TypeId GetTypeId();
  Application();
  ~Application() override;

  void SetStartTime(Time start);

  void SetStopTime(Time stop);

  Ptr<Node> GetNode() const;

  void SetNode(Ptr<Node> node);

  typedef void (*DelayAddressCallback)(const Time &delay, const Address &from);

  typedef void (*StateTransitionCallback)(const std::string &oldState,
                                          const std::string &newState);

private:
  virtual void StartApplication();

  virtual void StopApplication();

protected:
  void DoDispose() override;
  void DoInitialize() override;

  Ptr<Node> m_node;
  Time m_startTime;
  Time m_stopTime;
  EventId m_startEvent;
  EventId m_stopEvent;
};

} // namespace ns3

#endif
