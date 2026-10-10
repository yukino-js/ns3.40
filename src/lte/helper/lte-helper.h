
#ifndef LTE_HELPER_H
#define LTE_HELPER_H

#include "mac-stats-calculator.h"
#include "phy-rx-stats-calculator.h"
#include "phy-stats-calculator.h"
#include "phy-tx-stats-calculator.h"
#include "radio-bearer-stats-calculator.h"
#include "radio-bearer-stats-connector.h"

#include <ns3/component-carrier-enb.h>
#include <ns3/config.h>
#include <ns3/epc-tft.h>
#include <ns3/eps-bearer.h>
#include <ns3/mobility-model.h>
#include <ns3/names.h>
#include <ns3/net-device-container.h>
#include <ns3/net-device.h>
#include <ns3/node-container.h>
#include <ns3/node.h>
#include <ns3/simulator.h>

#include <map>

namespace ns3 {

class LteUePhy;
class LteEnbPhy;
class SpectrumChannel;
class EpcHelper;
class PropagationLossModel;
class SpectrumPropagationLossModel;

class LteHelper : public Object {
public:
  LteHelper();
  ~LteHelper() override;

  static TypeId GetTypeId();
  void DoDispose() override;

  void SetEpcHelper(Ptr<EpcHelper> h);

  void SetPathlossModelType(TypeId type);

  void SetPathlossModelAttribute(std::string n, const AttributeValue &v);

  void SetSchedulerType(std::string type);

  std::string GetSchedulerType() const;

  void SetSchedulerAttribute(std::string n, const AttributeValue &v);

  void SetFfrAlgorithmType(std::string type);

  std::string GetFfrAlgorithmType() const;

  void SetFfrAlgorithmAttribute(std::string n, const AttributeValue &v);

  void SetHandoverAlgorithmType(std::string type);

  std::string GetHandoverAlgorithmType() const;

  void SetHandoverAlgorithmAttribute(std::string n, const AttributeValue &v);

  void SetEnbDeviceAttribute(std::string n, const AttributeValue &v);

  void SetEnbAntennaModelType(std::string type);

  void SetEnbAntennaModelAttribute(std::string n, const AttributeValue &v);

  void SetUeDeviceAttribute(std::string n, const AttributeValue &v);

  void SetUeAntennaModelType(std::string type);

  void SetUeAntennaModelAttribute(std::string n, const AttributeValue &v);

  void SetSpectrumChannelType(std::string type);

  void SetSpectrumChannelAttribute(std::string n, const AttributeValue &v);

  void SetEnbComponentCarrierManagerType(std::string type);

  std::string GetEnbComponentCarrierManagerType() const;

  void SetEnbComponentCarrierManagerAttribute(std::string n,
                                              const AttributeValue &v);

  void SetUeComponentCarrierManagerType(std::string type);

  std::string GetUeComponentCarrierManagerType() const;

  void SetUeComponentCarrierManagerAttribute(std::string n,
                                             const AttributeValue &v);

  NetDeviceContainer InstallEnbDevice(NodeContainer c);

  NetDeviceContainer InstallUeDevice(NodeContainer c);

  void Attach(NetDeviceContainer ueDevices);

  void Attach(Ptr<NetDevice> ueDevice);

  void Attach(NetDeviceContainer ueDevices, Ptr<NetDevice> enbDevice);

  void Attach(Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice,
              uint8_t componentCarrierId = 0);

  void AttachToClosestEnb(NetDeviceContainer ueDevices,
                          NetDeviceContainer enbDevices);

  void AttachToClosestEnb(Ptr<NetDevice> ueDevice,
                          NetDeviceContainer enbDevices);

  uint8_t ActivateDedicatedEpsBearer(NetDeviceContainer ueDevices,
                                     EpsBearer bearer, Ptr<EpcTft> tft);

  uint8_t ActivateDedicatedEpsBearer(Ptr<NetDevice> ueDevice, EpsBearer bearer,
                                     Ptr<EpcTft> tft);

  void DeActivateDedicatedEpsBearer(Ptr<NetDevice> ueDevice,
                                    Ptr<NetDevice> enbDevice, uint8_t bearerId);
  void AddX2Interface(NodeContainer enbNodes);

  void AddX2Interface(Ptr<Node> enbNode1, Ptr<Node> enbNode2);

  void HandoverRequest(Time hoTime, Ptr<NetDevice> ueDev,
                       Ptr<NetDevice> sourceEnbDev,
                       Ptr<NetDevice> targetEnbDev);

  void HandoverRequest(Time hoTime, Ptr<NetDevice> ueDev,
                       Ptr<NetDevice> sourceEnbDev, uint16_t targetCellId);

  void ActivateDataRadioBearer(NetDeviceContainer ueDevices, EpsBearer bearer);

  void ActivateDataRadioBearer(Ptr<NetDevice> ueDevice, EpsBearer bearer);

  void SetFadingModel(std::string type);

  void SetFadingModelAttribute(std::string n, const AttributeValue &v);

  void EnableLogComponents();

  void EnableTraces();

  void EnablePhyTraces();

  void EnableDlPhyTraces();

  void EnableUlPhyTraces();

  void EnableDlTxPhyTraces();

  void EnableUlTxPhyTraces();

  void EnableDlRxPhyTraces();

  void EnableUlRxPhyTraces();

  void EnableMacTraces();

  void EnableDlMacTraces();

  void EnableUlMacTraces();

  void EnableRlcTraces();

  Ptr<RadioBearerStatsCalculator> GetRlcStats();

  void EnablePdcpTraces();

  Ptr<RadioBearerStatsCalculator> GetPdcpStats();

  int64_t AssignStreams(NetDeviceContainer c, int64_t stream);

  Ptr<SpectrumChannel> GetUplinkSpectrumChannel() const;

  Ptr<SpectrumChannel> GetDownlinkSpectrumChannel() const;

protected:
  void DoInitialize() override;

private:
  void DoComponentCarrierConfigure(uint32_t ulEarfcn, uint32_t dlEarfcn,
                                   uint16_t ulbw, uint16_t dlbw);
  Ptr<NetDevice> InstallSingleEnbDevice(Ptr<Node> n);

  Ptr<NetDevice> InstallSingleUeDevice(Ptr<Node> n);

  void DoHandoverRequest(Ptr<NetDevice> ueDev, Ptr<NetDevice> sourceEnbDev,
                         uint16_t targetCellId);

  void DoDeActivateDedicatedEpsBearer(Ptr<NetDevice> ueDevice,
                                      Ptr<NetDevice> enbDevice,
                                      uint8_t bearerId);

  void ChannelModelInitialization();

  Ptr<SpectrumChannel> m_downlinkChannel;
  Ptr<SpectrumChannel> m_uplinkChannel;
  Ptr<Object> m_downlinkPathlossModel;
  Ptr<Object> m_uplinkPathlossModel;

  ObjectFactory m_schedulerFactory;
  ObjectFactory m_ffrAlgorithmFactory;
  ObjectFactory m_handoverAlgorithmFactory;
  ObjectFactory m_enbComponentCarrierManagerFactory;
  ObjectFactory m_ueComponentCarrierManagerFactory;
  ObjectFactory m_enbNetDeviceFactory;
  ObjectFactory m_enbAntennaModelFactory;
  ObjectFactory m_ueNetDeviceFactory;
  ObjectFactory m_ueAntennaModelFactory;
  ObjectFactory m_pathlossModelFactory;
  ObjectFactory m_channelFactory;

  std::string m_fadingModelType;
  ObjectFactory m_fadingModelFactory;
  Ptr<SpectrumPropagationLossModel> m_fadingModel;
  bool m_fadingStreamsAssigned;

  Ptr<PhyStatsCalculator> m_phyStats;
  Ptr<PhyTxStatsCalculator> m_phyTxStats;
  Ptr<PhyRxStatsCalculator> m_phyRxStats;
  Ptr<MacStatsCalculator> m_macStats;
  Ptr<RadioBearerStatsCalculator> m_rlcStats;
  Ptr<RadioBearerStatsCalculator> m_pdcpStats;
  RadioBearerStatsConnector m_radioBearerStatsConnector;

  Ptr<EpcHelper> m_epcHelper;

  uint64_t m_imsiCounter;
  uint16_t m_cellIdCounter;

  bool m_useIdealRrc;
  bool m_isAnrEnabled;
  bool m_usePdschForCqiGeneration;

  bool m_useCa;

  std::map<uint8_t, ComponentCarrier> m_componentCarrierPhyParams;

  uint16_t m_noOfCcs;
};

} // namespace ns3

#endif
