#ifndef TRAFFIC_CONTROL_HELPER_H
#define TRAFFIC_CONTROL_HELPER_H

#include "queue-disc-container.h"

#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"
#include "ns3/queue.h"

#include <map>
#include <string>
#include <vector>

namespace ns3 {

class QueueDiscFactory {
public:
  QueueDiscFactory(ObjectFactory factory);

  virtual ~QueueDiscFactory() {}

  QueueDiscFactory() = delete;

  void AddInternalQueue(ObjectFactory factory);

  void AddPacketFilter(ObjectFactory factory);

  uint16_t AddQueueDiscClass(ObjectFactory factory);

  void SetChildQueueDisc(uint16_t classId, uint16_t handle);

  Ptr<QueueDisc> CreateQueueDisc(const std::vector<Ptr<QueueDisc>> &queueDiscs);

private:
  ObjectFactory m_queueDiscFactory;
  std::vector<ObjectFactory> m_internalQueuesFactory;
  std::vector<ObjectFactory> m_packetFiltersFactory;
  std::vector<ObjectFactory> m_queueDiscClassesFactory;
  std::map<uint16_t, uint16_t> m_classIdChildHandleMap;
};

class TrafficControlHelper {
public:
  TrafficControlHelper();

  virtual ~TrafficControlHelper() {}

  static TrafficControlHelper Default(std::size_t nTxQueues = 1);

  template <typename... Args>
  uint16_t SetRootQueueDisc(const std::string &type, Args &&...args);

  template <typename... Args>
  void AddInternalQueues(uint16_t handle, uint16_t count, std::string type,
                         Args &&...args);

  template <typename... Args>
  void AddPacketFilter(uint16_t handle, const std::string &type,
                       Args &&...args);

  typedef std::vector<uint16_t> ClassIdList;

  template <typename... Args>
  ClassIdList AddQueueDiscClasses(uint16_t handle, uint16_t count,
                                  const std::string &type, Args &&...args);

  template <typename... Args>
  uint16_t AddChildQueueDisc(uint16_t handle, uint16_t classId,
                             const std::string &type, Args &&...args);

  typedef std::vector<uint16_t> HandleList;

  template <typename... Args>
  HandleList AddChildQueueDiscs(uint16_t handle, const ClassIdList &classes,
                                const std::string &type, Args &&...args);

  template <typename... Args>
  void SetQueueLimits(std::string type, Args &&...args);

  QueueDiscContainer Install(NetDeviceContainer c);

  QueueDiscContainer Install(Ptr<NetDevice> d);

  void Uninstall(NetDeviceContainer c);

  void Uninstall(Ptr<NetDevice> d);

private:
  uint16_t DoSetRootQueueDisc(ObjectFactory factory);

  void DoAddInternalQueues(uint16_t handle, uint16_t count,
                           ObjectFactory factory);

  void DoAddPacketFilter(uint16_t handle, ObjectFactory factory);

  ClassIdList DoAddQueueDiscClasses(uint16_t handle, uint16_t count,
                                    ObjectFactory factory);

  uint16_t DoAddChildQueueDisc(uint16_t handle, uint16_t classId,
                               ObjectFactory factory);

  HandleList DoAddChildQueueDiscs(uint16_t handle, const ClassIdList &classes,
                                  ObjectFactory factory);

  std::vector<QueueDiscFactory> m_queueDiscFactory;
  std::vector<Ptr<QueueDisc>> m_queueDiscs;
  ObjectFactory m_queueLimitsFactory;
};

} // namespace ns3

namespace ns3 {

template <typename... Args>
uint16_t TrafficControlHelper::SetRootQueueDisc(const std::string &type,
                                                Args &&...args) {
  return DoSetRootQueueDisc(ObjectFactory(type, args...));
}

template <typename... Args>
void TrafficControlHelper::AddInternalQueues(uint16_t handle, uint16_t count,
                                             std::string type, Args &&...args) {
  QueueBase::AppendItemTypeIfNotPresent(type, "QueueDiscItem");
  DoAddInternalQueues(handle, count, ObjectFactory(type, args...));
}

template <typename... Args>
void TrafficControlHelper::AddPacketFilter(uint16_t handle,
                                           const std::string &type,
                                           Args &&...args) {
  DoAddPacketFilter(handle, ObjectFactory(type, args...));
}

template <typename... Args>
TrafficControlHelper::ClassIdList TrafficControlHelper::AddQueueDiscClasses(
    uint16_t handle, uint16_t count, const std::string &type, Args &&...args) {
  return DoAddQueueDiscClasses(handle, count, ObjectFactory(type, args...));
}

template <typename... Args>
uint16_t TrafficControlHelper::AddChildQueueDisc(uint16_t handle,
                                                 uint16_t classId,
                                                 const std::string &type,
                                                 Args &&...args) {
  return DoAddChildQueueDisc(handle, classId, ObjectFactory(type, args...));
}

template <typename... Args>
TrafficControlHelper::HandleList TrafficControlHelper::AddChildQueueDiscs(
    uint16_t handle, const ClassIdList &classes, const std::string &type,
    Args &&...args) {
  return DoAddChildQueueDiscs(handle, classes, ObjectFactory(type, args...));
}

template <typename... Args>
void TrafficControlHelper::SetQueueLimits(std::string type, Args &&...args) {
  m_queueLimitsFactory.SetTypeId(type);
  m_queueLimitsFactory.Set(args...);
}

} // namespace ns3

#endif
