
#ifndef COMPONENT_CARRIER_H
#define COMPONENT_CARRIER_H

#include <ns3/object.h>

namespace ns3 {

class ComponentCarrier : public Object {
public:
  static TypeId GetTypeId();

  ComponentCarrier();

  ~ComponentCarrier() override;
  void DoDispose() override;

  uint16_t GetUlBandwidth() const;

  virtual void SetUlBandwidth(uint16_t bw);

  uint16_t GetDlBandwidth() const;

  virtual void SetDlBandwidth(uint16_t bw);

  uint32_t GetDlEarfcn() const;

  void SetDlEarfcn(uint32_t earfcn);

  uint32_t GetUlEarfcn() const;

  void SetUlEarfcn(uint32_t earfcn);

  uint32_t GetCsgId() const;

  void SetCsgId(uint32_t csgId);

  bool GetCsgIndication() const;

  void SetCsgIndication(bool csgIndication);

  void SetAsPrimary(bool primaryCarrier);

  bool IsPrimary() const;

protected:
  uint32_t m_csgId{0};
  bool m_csgIndication{false};

  bool m_primaryCarrier{false};

  uint16_t m_dlBandwidth{0};
  uint16_t m_ulBandwidth{0};

  uint32_t m_dlEarfcn{0};
  uint32_t m_ulEarfcn{0};
};

class ComponentCarrierBaseStation : public ComponentCarrier {
public:
  static TypeId GetTypeId();

  ComponentCarrierBaseStation();

  ~ComponentCarrierBaseStation() override;

  uint16_t GetCellId() const;

  void SetCellId(uint16_t cellId);

protected:
  uint16_t m_cellId{0};
};

} // namespace ns3

#endif
