
#ifndef LTE_TEST_UE_MEASUREMENTS_H
#define LTE_TEST_UE_MEASUREMENTS_H

#include <ns3/lte-rrc-sap.h>
#include <ns3/nstime.h>
#include <ns3/test.h>

#include <list>
#include <set>
#include <vector>

namespace ns3 {

class MobilityModel;

}

using namespace ns3;

class LteUeMeasurementsTestSuite : public TestSuite {
public:
  LteUeMeasurementsTestSuite();
};

class LteUeMeasurementsTestCase : public TestCase {
public:
  LteUeMeasurementsTestCase(std::string name, double d1, double d2,
                            double rsrpDbmUe1, double rsrpDbmUe2,
                            double rsrqDbUe1, double rsrqDbUe2);
  ~LteUeMeasurementsTestCase() override;

  void ReportUeMeasurements(uint16_t rnti, uint16_t cellId, double rsrp,
                            double rsrq, bool servingCell);

  void RecvMeasurementReport(uint64_t imsi, uint16_t cellId, uint16_t rnti,
                             LteRrcSap::MeasurementReport meas);

private:
  void DoRun() override;

  double m_d1;
  double m_d2;
  double m_rsrpDbmUeServingCell;
  double m_rsrpDbmUeNeighborCell;
  double m_rsrqDbUeServingCell;
  double m_rsrqDbUeNeighborCell;
};

class LteUeMeasurementsPiecewiseTestSuite1 : public TestSuite {
public:
  LteUeMeasurementsPiecewiseTestSuite1();
};

class LteUeMeasurementsPiecewiseTestCase1 : public TestCase {
public:
  LteUeMeasurementsPiecewiseTestCase1(std::string name,
                                      LteRrcSap::ReportConfigEutra config,
                                      std::vector<Time> expectedTime,
                                      std::vector<uint8_t> expectedRsrp);

  ~LteUeMeasurementsPiecewiseTestCase1() override;

  void RecvMeasurementReportCallback(std::string context, uint64_t imsi,
                                     uint16_t cellId, uint16_t rnti,
                                     LteRrcSap::MeasurementReport report);

private:
  void DoRun() override;

  void DoTeardown() override;

  void TeleportVeryNear();
  void TeleportNear();
  void TeleportFar();
  void TeleportVeryFar();

  LteRrcSap::ReportConfigEutra m_config;

  std::vector<Time> m_expectedTime;

  std::vector<uint8_t> m_expectedRsrp;

  std::vector<Time>::iterator m_itExpectedTime;

  std::vector<uint8_t>::iterator m_itExpectedRsrp;

  uint8_t m_expectedMeasId;

  Ptr<MobilityModel> m_ueMobility;
};

class LteUeMeasurementsPiecewiseTestSuite2 : public TestSuite {
public:
  LteUeMeasurementsPiecewiseTestSuite2();
};

class LteUeMeasurementsPiecewiseTestCase2 : public TestCase {
public:
  LteUeMeasurementsPiecewiseTestCase2(std::string name,
                                      LteRrcSap::ReportConfigEutra config,
                                      std::vector<Time> expectedTime,
                                      std::vector<uint8_t> expectedRsrp);

  ~LteUeMeasurementsPiecewiseTestCase2() override;

  void RecvMeasurementReportCallback(std::string context, uint64_t imsi,
                                     uint16_t cellId, uint16_t rnti,
                                     LteRrcSap::MeasurementReport report);

private:
  void DoRun() override;

  void DoTeardown() override;

  void TeleportVeryNear();
  void TeleportNear();
  void TeleportFar();
  void TeleportVeryFar();

  LteRrcSap::ReportConfigEutra m_config;

  std::vector<Time> m_expectedTime;

  std::vector<uint8_t> m_expectedRsrp;

  std::vector<Time>::iterator m_itExpectedTime;

  std::vector<uint8_t>::iterator m_itExpectedRsrp;

  uint8_t m_expectedMeasId;

  Ptr<MobilityModel> m_ueMobility;
};

class LteUeMeasurementsPiecewiseTestSuite3 : public TestSuite {
public:
  LteUeMeasurementsPiecewiseTestSuite3();
};

class LteUeMeasurementsPiecewiseTestCase3 : public TestCase {
public:
  LteUeMeasurementsPiecewiseTestCase3(std::string name,
                                      LteRrcSap::ReportConfigEutra config,
                                      std::vector<Time> expectedTime);

  ~LteUeMeasurementsPiecewiseTestCase3() override;

  void RecvMeasurementReportCallback(std::string context, uint64_t imsi,
                                     uint16_t cellId, uint16_t rnti,
                                     LteRrcSap::MeasurementReport report);

private:
  void DoRun() override;

  void DoTeardown() override;

  void TeleportEnbNear();

  LteRrcSap::ReportConfigEutra m_config;

  std::vector<Time> m_expectedTime;

  std::vector<Time>::iterator m_itExpectedTime;

  uint8_t m_expectedMeasId;

  Ptr<MobilityModel> m_enbMobility;
};

class LteUeMeasurementsHandoverTestSuite : public TestSuite {
public:
  LteUeMeasurementsHandoverTestSuite();
};

class LteUeMeasurementsHandoverTestCase : public TestCase {
public:
  LteUeMeasurementsHandoverTestCase(
      std::string name,
      std::list<LteRrcSap::ReportConfigEutra> sourceConfigList,
      std::list<LteRrcSap::ReportConfigEutra> targetConfigList,
      std::vector<Time> expectedTime, std::vector<uint8_t> expectedRsrp,
      Time duration);

  ~LteUeMeasurementsHandoverTestCase() override;

  void RecvMeasurementReportCallback(std::string context, uint64_t imsi,
                                     uint16_t cellId, uint16_t rnti,
                                     LteRrcSap::MeasurementReport report);

private:
  void DoRun() override;

  void DoTeardown() override;

  std::list<LteRrcSap::ReportConfigEutra> m_sourceConfigList;

  std::list<LteRrcSap::ReportConfigEutra> m_targetConfigList;

  std::vector<Time> m_expectedTime;

  std::vector<uint8_t> m_expectedRsrp;

  std::vector<Time>::iterator m_itExpectedTime;

  std::vector<uint8_t>::iterator m_itExpectedRsrp;

  Time m_duration;

  std::set<uint8_t> m_expectedSourceCellMeasId;

  std::set<uint8_t> m_expectedTargetCellMeasId;
};

#endif
