
#ifndef WIFI_MAC_HELPER_H
#define WIFI_MAC_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/wifi-standards.h"

namespace ns3 {

class WifiMac;
class WifiNetDevice;

class WifiMacHelper {
public:
  WifiMacHelper();
  virtual ~WifiMacHelper();

  template <typename... Args> void SetType(std::string type, Args &&...args);

  template <typename... Args>
  void SetAssocManager(std::string type, Args &&...args);

  template <typename... Args>
  void SetMacQueueScheduler(std::string type, Args &&...args);

  template <typename... Args>
  void SetProtectionManager(std::string type, Args &&...args);

  template <typename... Args>
  void SetAckManager(std::string type, Args &&...args);

  template <typename... Args>
  void SetMultiUserScheduler(std::string type, Args &&...args);

  template <typename... Args>
  void SetEmlsrManager(std::string type, Args &&...args);

  virtual Ptr<WifiMac> Create(Ptr<WifiNetDevice> device,
                              WifiStandard standard) const;

protected:
  ObjectFactory m_mac;
  ObjectFactory m_assocManager;
  ObjectFactory m_queueScheduler;
  ObjectFactory m_protectionManager;
  ObjectFactory m_ackManager;
  ObjectFactory m_muScheduler;
  ObjectFactory m_emlsrManager;
};

} // namespace ns3

namespace ns3 {

template <typename... Args>
void WifiMacHelper::SetType(std::string type, Args &&...args) {
  m_mac.SetTypeId(type);
  m_mac.Set(args...);
}

template <typename... Args>
void WifiMacHelper::SetAssocManager(std::string type, Args &&...args) {
  m_assocManager.SetTypeId(type);
  m_assocManager.Set(args...);
}

template <typename... Args>
void WifiMacHelper::SetMacQueueScheduler(std::string type, Args &&...args) {
  m_queueScheduler.SetTypeId(type);
  m_queueScheduler.Set(args...);
}

template <typename... Args>
void WifiMacHelper::SetProtectionManager(std::string type, Args &&...args) {
  m_protectionManager.SetTypeId(type);
  m_protectionManager.Set(args...);
}

template <typename... Args>
void WifiMacHelper::SetAckManager(std::string type, Args &&...args) {
  m_ackManager.SetTypeId(type);
  m_ackManager.Set(args...);
}

template <typename... Args>
void WifiMacHelper::SetMultiUserScheduler(std::string type, Args &&...args) {
  m_muScheduler.SetTypeId(type);
  m_muScheduler.Set(args...);
}

template <typename... Args>
void WifiMacHelper::SetEmlsrManager(std::string type, Args &&...args) {
  m_emlsrManager.SetTypeId(type);
  m_emlsrManager.Set(args...);
}

} // namespace ns3

#endif
