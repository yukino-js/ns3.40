
#ifndef LTE_TEST_DOWNLINK_FR_H
#define LTE_TEST_DOWNLINK_FR_H

#include "ns3/lte-spectrum-value-helper.h"
#include "ns3/node.h"
#include "ns3/spectrum-test.h"
#include "ns3/spectrum-value.h"
#include "ns3/test.h"
#include <ns3/lte-rrc-sap.h>

namespace ns3 {
class MobilityModel;
}

using namespace ns3;

class LteFrequencyReuseTestSuite : public TestSuite {
public:
  LteFrequencyReuseTestSuite();
};

class LteFrTestCase : public TestCase {
public:
  LteFrTestCase(std::string name, uint32_t userNum, uint16_t dlBandwidth,
                uint16_t ulBandwidth, std::vector<bool> availableDlRb,
                std::vector<bool> availableUlRb);
  ~LteFrTestCase() override;

  void DlDataRxStart(Ptr<const SpectrumValue> spectrumValue);
  void UlDataRxStart(Ptr<const SpectrumValue> spectrumValue);

protected:
  void DoRun() override;

  uint32_t m_userNum;
  uint16_t m_dlBandwidth;
  uint16_t m_ulBandwidth;

  std::vector<bool> m_availableDlRb;
  bool m_usedMutedDlRbg;

  std::vector<bool> m_availableUlRb;
  bool m_usedMutedUlRbg;
};

class LteHardFrTestCase : public LteFrTestCase {
public:
  LteHardFrTestCase(std::string name, uint32_t userNum,
                    std::string schedulerType, uint16_t dlBandwidth,
                    uint16_t ulBandwidth, uint8_t dlSubBandOffset,
                    uint16_t dlSubBandwidth, uint8_t ulSubBandOffset,
                    uint16_t ulSubBandwidth, std::vector<bool> availableDlRb,
                    std::vector<bool> availableUlRb);
  ~LteHardFrTestCase() override;

private:
  void DoRun() override;

  std::string m_schedulerType;

  uint8_t m_dlSubBandOffset;
  uint8_t m_dlSubBandwidth;

  uint8_t m_ulSubBandOffset;
  uint8_t m_ulSubBandwidth;
};

class LteStrictFrTestCase : public LteFrTestCase {
public:
  LteStrictFrTestCase(std::string name, uint32_t userNum,
                      std::string schedulerType, uint16_t dlBandwidth,
                      uint16_t ulBandwidth, uint16_t dlCommonSubBandwidth,
                      uint8_t dlEdgeSubBandOffset, uint16_t dlEdgeSubBandwidth,
                      uint16_t ulCommonSubBandwidth,
                      uint8_t ulEdgeSubBandOffset, uint16_t ulEdgeSubBandwidth,
                      std::vector<bool> availableDlRb,
                      std::vector<bool> availableUlRb);
  ~LteStrictFrTestCase() override;

private:
  void DoRun() override;

  std::string m_schedulerType;

  uint16_t m_dlCommonSubBandwidth;
  uint8_t m_dlEdgeSubBandOffset;
  uint16_t m_dlEdgeSubBandwidth;

  uint16_t m_ulCommonSubBandwidth;
  uint8_t m_ulEdgeSubBandOffset;
  uint16_t m_ulEdgeSubBandwidth;
};

class LteFrAreaTestCase : public TestCase {
public:
  LteFrAreaTestCase(std::string name, std::string schedulerType);
  ~LteFrAreaTestCase() override;

  void DlDataRxStart(Ptr<const SpectrumValue> spectrumValue);
  void UlDataRxStart(Ptr<const SpectrumValue> spectrumValue);

  void SimpleTeleportUe(uint32_t x, uint32_t y);
  void TeleportUe(uint32_t x, uint32_t y, double expectedPower,
                  std::vector<bool> expectedDlRb);
  void TeleportUe2(Ptr<Node> ueNode, uint32_t x, uint32_t y,
                   double expectedPower, std::vector<bool> expectedDlRb);

  void SetDlExpectedValues(double expectedPower,
                           std::vector<bool> expectedDlRb);
  void SetUlExpectedValues(double expectedPower,
                           std::vector<bool> expectedDlRb);

protected:
  void DoRun() override;

  std::string m_schedulerType;

  uint16_t m_dlBandwidth;
  uint16_t m_ulBandwidth;

  Time m_teleportTime;
  Ptr<MobilityModel> m_ueMobility;

  double m_expectedDlPower;
  std::vector<bool> m_expectedDlRb;
  bool m_usedWrongDlRbg;
  bool m_usedWrongDlPower;

  double m_expectedUlPower;
  std::vector<bool> m_expectedUlRb;
  bool m_usedWrongUlRbg;
  bool m_usedWrongUlPower;
};

class LteStrictFrAreaTestCase : public LteFrAreaTestCase {
public:
  LteStrictFrAreaTestCase(std::string name, std::string schedulerType);
  ~LteStrictFrAreaTestCase() override;

private:
  void DoRun() override;
};

class LteSoftFrAreaTestCase : public LteFrAreaTestCase {
public:
  LteSoftFrAreaTestCase(std::string name, std::string schedulerType);
  ~LteSoftFrAreaTestCase() override;

private:
  void DoRun() override;
};

class LteSoftFfrAreaTestCase : public LteFrAreaTestCase {
public:
  LteSoftFfrAreaTestCase(std::string name, std::string schedulerType);
  ~LteSoftFfrAreaTestCase() override;

private:
  void DoRun() override;
};

class LteEnhancedFfrAreaTestCase : public LteFrAreaTestCase {
public:
  LteEnhancedFfrAreaTestCase(std::string name, std::string schedulerType);
  ~LteEnhancedFfrAreaTestCase() override;

private:
  void DoRun() override;
};

class LteDistributedFfrAreaTestCase : public LteFrAreaTestCase {
public:
  LteDistributedFfrAreaTestCase(std::string name, std::string schedulerType);
  ~LteDistributedFfrAreaTestCase() override;

private:
  void DoRun() override;
};

#endif
