
#ifndef SERVICE_FLOW_H
#define SERVICE_FLOW_H

#include "cs-parameters.h"
#include "wimax-connection.h"
#include "wimax-mac-header.h"
#include "wimax-phy.h"

#include <cstdint>

namespace ns3 {

class ServiceFlowRecord;
class WimaxConnection;
class WimaxMacQueue;

class ServiceFlow {
public:
  enum Direction { SF_DIRECTION_DOWN, SF_DIRECTION_UP };

  enum Type { SF_TYPE_PROVISIONED, SF_TYPE_ADMITTED, SF_TYPE_ACTIVE };

  enum SchedulingType {
    SF_TYPE_NONE = 0,
    SF_TYPE_UNDEF = 1,
    SF_TYPE_BE = 2,
    SF_TYPE_NRTPS = 3,
    SF_TYPE_RTPS = 4,
    SF_TYPE_UGS = 6,
    SF_TYPE_ALL = 255
  };

  enum CsSpecification {
    ATM = 99,
    IPV4 = 100,
    IPV6 = 101,
    ETHERNET = 102,
    VLAN = 103,
    IPV4_OVER_ETHERNET = 104,
    IPV6_OVER_ETHERNET = 105,
    IPV4_OVER_VLAN = 106,
    IPV6_OVER_VLAN = 107
  };

  enum ModulationType {
    MODULATION_TYPE_BPSK_12,
    MODULATION_TYPE_QPSK_12,
    MODULATION_TYPE_QPSK_34,
    MODULATION_TYPE_QAM16_12,
    MODULATION_TYPE_QAM16_34,
    MODULATION_TYPE_QAM64_23,
    MODULATION_TYPE_QAM64_34
  };

  Tlv ToTlv() const;
  ServiceFlow(Tlv tlv);
  bool CheckClassifierMatch(Ipv4Address srcAddress, Ipv4Address dstAddress,
                            uint16_t srcPort, uint16_t dstPort,
                            uint8_t proto) const;
  ServiceFlow();
  ServiceFlow(Direction direction);
  ServiceFlow(const ServiceFlow &sf);
  ServiceFlow(uint32_t sfid, Direction direction,
              Ptr<WimaxConnection> connection);
  ~ServiceFlow();
  ServiceFlow &operator=(const ServiceFlow &o);

  void InitValues();
  void SetDirection(Direction direction);
  Direction GetDirection() const;
  void CopyParametersFrom(ServiceFlow sf);

  void SetType(Type type);
  Type GetType() const;
  void SetConnection(Ptr<WimaxConnection> connection);
  Ptr<WimaxConnection> GetConnection() const;

  void SetIsEnabled(bool isEnabled);
  bool GetIsEnabled() const;

  void SetRecord(ServiceFlowRecord *record);
  ServiceFlowRecord *GetRecord() const;

  Ptr<WimaxMacQueue> GetQueue() const;
  ServiceFlow::SchedulingType GetSchedulingType() const;
  bool HasPackets() const;
  bool HasPackets(MacHeaderType::HeaderType packetType) const;

  void CleanUpQueue();

  void PrintQoSParameters() const;

  char *GetSchedulingTypeStr() const;

  uint32_t GetSfid() const;
  uint16_t GetCid() const;
  std::string GetServiceClassName() const;
  uint8_t GetQosParamSetType() const;
  uint8_t GetTrafficPriority() const;
  uint32_t GetMaxSustainedTrafficRate() const;
  uint32_t GetMaxTrafficBurst() const;
  uint32_t GetMinReservedTrafficRate() const;
  uint32_t GetMinTolerableTrafficRate() const;
  ServiceFlow::SchedulingType GetServiceSchedulingType() const;
  uint32_t GetRequestTransmissionPolicy() const;
  uint32_t GetToleratedJitter() const;
  uint32_t GetMaximumLatency() const;
  uint8_t GetFixedversusVariableSduIndicator() const;
  uint8_t GetSduSize() const;
  uint16_t GetTargetSAID() const;
  uint8_t GetArqEnable() const;
  uint16_t GetArqWindowSize() const;
  uint16_t GetArqRetryTimeoutTx() const;
  uint16_t GetArqRetryTimeoutRx() const;
  uint16_t GetArqBlockLifeTime() const;
  uint16_t GetArqSyncLoss() const;
  uint8_t GetArqDeliverInOrder() const;
  uint16_t GetArqPurgeTimeout() const;
  uint16_t GetArqBlockSize() const;
  CsSpecification GetCsSpecification() const;
  CsParameters GetConvergenceSublayerParam() const;
  uint16_t GetUnsolicitedGrantInterval() const;
  uint16_t GetUnsolicitedPollingInterval() const;
  bool GetIsMulticast() const;
  WimaxPhy::ModulationType GetModulation() const;

  void SetSfid(uint32_t sfid);
  void SetServiceClassName(std::string name);
  void SetQosParamSetType(uint8_t type);
  void SetTrafficPriority(uint8_t priority);
  void SetMaxSustainedTrafficRate(uint32_t maxSustainedRate);
  void SetMaxTrafficBurst(uint32_t maxTrafficBurst);
  void SetMinReservedTrafficRate(uint32_t minResvRate);
  void SetMinTolerableTrafficRate(uint32_t minJitter);
  void SetServiceSchedulingType(ServiceFlow::SchedulingType schedType);
  void SetRequestTransmissionPolicy(uint32_t policy);
  void SetToleratedJitter(uint32_t jitter);
  void SetMaximumLatency(uint32_t MaximumLatency);
  void SetFixedversusVariableSduIndicator(uint8_t sduIndicator);
  void SetSduSize(uint8_t sduSize);
  void SetTargetSAID(uint16_t targetSaid);
  void SetArqEnable(uint8_t arqEnable);
  void SetArqWindowSize(uint16_t arqWindowSize);
  void SetArqRetryTimeoutTx(uint16_t timeout);
  void SetArqRetryTimeoutRx(uint16_t timeout);
  void SetArqBlockLifeTime(uint16_t lifeTime);
  void SetArqSyncLoss(uint16_t syncLoss);
  void SetArqDeliverInOrder(uint8_t inOrder);
  void SetArqPurgeTimeout(uint16_t timeout);
  void SetArqBlockSize(uint16_t size);
  void SetCsSpecification(CsSpecification spec);
  void SetConvergenceSublayerParam(CsParameters csparam);

  void SetUnsolicitedGrantInterval(uint16_t unsolicitedGrantInterval);
  void SetUnsolicitedPollingInterval(uint16_t unsolicitedPollingInterval);
  void SetIsMulticast(bool isMulticast);
  void SetModulation(WimaxPhy::ModulationType modulationType);

private:
  uint32_t m_sfid;
  std::string m_serviceClassName;
  uint8_t m_qosParamSetType;
  uint8_t m_trafficPriority;
  uint32_t m_maxSustainedTrafficRate;
  uint32_t m_maxTrafficBurst;
  uint32_t m_minReservedTrafficRate;
  uint32_t m_minTolerableTrafficRate;
  ServiceFlow::SchedulingType m_schedulingType;
  uint32_t m_requestTransmissionPolicy;
  uint32_t m_toleratedJitter;
  uint32_t m_maximumLatency;
  uint8_t m_fixedversusVariableSduIndicator;
  uint8_t m_sduSize;
  uint16_t m_targetSAID;
  uint8_t m_arqEnable;
  uint16_t m_arqWindowSize;
  uint16_t m_arqRetryTimeoutTx;
  uint16_t m_arqRetryTimeoutRx;
  uint16_t m_arqBlockLifeTime;
  uint16_t m_arqSyncLoss;
  uint8_t m_arqDeliverInOrder;
  uint16_t m_arqPurgeTimeout;
  uint16_t m_arqBlockSize;
  CsSpecification m_csSpecification;
  CsParameters m_convergenceSublayerParam;
  uint16_t m_unsolicitedGrantInterval;
  uint16_t m_unsolicitedPollingInterval;
  Direction m_direction;
  Type m_type;
  Ptr<WimaxConnection> m_connection;
  bool m_isEnabled;
  bool m_isMulticast;
  WimaxPhy::ModulationType m_modulationType;
  ServiceFlowRecord *m_record;
};

} // namespace ns3

#endif
