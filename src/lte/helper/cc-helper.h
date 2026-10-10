
#ifndef CC_HELPER_H
#define CC_HELPER_H

#include <ns3/component-carrier.h>
#include <ns3/config.h>
#include <ns3/names.h>
#include <ns3/net-device-container.h>
#include <ns3/net-device.h>
#include <ns3/node-container.h>
#include <ns3/node.h>
#include <ns3/simulator.h>

#include <map>

namespace ns3 {

class CcHelper : public Object {
public:
  CcHelper();
  ~CcHelper() override;

  static TypeId GetTypeId();
  void DoDispose() override;

  ComponentCarrier DoCreateSingleCc(uint16_t ulBandwidth, uint16_t dlBandwidth,
                                    uint32_t ulEarfcn, uint32_t dlEarfcn,
                                    bool isPrimary);

  void SetCcAttribute(std::string n, const AttributeValue &v);

  std::map<uint8_t, ComponentCarrier> EquallySpacedCcs();

  void SetNumberOfComponentCarriers(uint16_t nCc);
  void SetUlEarfcn(uint32_t ulEarfcn);
  void SetDlEarfcn(uint32_t dlEarfcn);
  void SetDlBandwidth(uint16_t dlBandwidth);
  void SetUlBandwidth(uint16_t ulBandwidth);
  uint16_t GetNumberOfComponentCarriers() const;
  uint32_t GetUlEarfcn() const;
  uint32_t GetDlEarfcn() const;
  uint16_t GetDlBandwidth() const;
  uint16_t GetUlBandwidth() const;

protected:
  void DoInitialize() override;

private:
  ComponentCarrier CreateSingleCc(uint16_t ulBandwidth, uint16_t dlBandwidth,
                                  uint32_t ulEarfcn, uint32_t dlEarfcn,
                                  bool isPrimary) const;

  ObjectFactory m_ccFactory;

  uint32_t m_ulEarfcn;
  uint32_t m_dlEarfcn;
  uint16_t m_dlBandwidth;
  uint16_t m_ulBandwidth;
  uint16_t m_numberOfComponentCarriers;
};

} // namespace ns3

#endif
