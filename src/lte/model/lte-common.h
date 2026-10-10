#ifndef LTE_COMMON_H
#define LTE_COMMON_H

#include "ns3/uinteger.h"

#include <cmath>

#define UL_PUSCH_TTIS_DELAY 4

#define HARQ_PERIOD 7

namespace ns3 {

constexpr uint32_t MIN_NO_CC = 1;

constexpr uint32_t MAX_NO_CC = 5;

struct LteFlowId_t {
  uint16_t m_rnti;
  uint8_t m_lcId;

public:
  LteFlowId_t();
  LteFlowId_t(const uint16_t a, const uint8_t b);

  friend bool operator==(const LteFlowId_t &a, const LteFlowId_t &b);
  friend bool operator<(const LteFlowId_t &a, const LteFlowId_t &b);
};

struct ImsiLcidPair_t {
  uint64_t m_imsi;
  uint8_t m_lcId;

public:
  ImsiLcidPair_t();
  ImsiLcidPair_t(const uint64_t a, const uint8_t b);

  friend bool operator==(const ImsiLcidPair_t &a, const ImsiLcidPair_t &b);
  friend bool operator<(const ImsiLcidPair_t &a, const ImsiLcidPair_t &b);
};

struct LteUeConfig_t {
  uint16_t m_rnti;
  bool m_reconfigureFlag;
  uint8_t m_transmissionMode;
  uint16_t m_srsConfigurationIndex;

public:
  LteUeConfig_t();

  friend bool operator==(const LteUeConfig_t &a, const LteUeConfig_t &b);
  friend bool operator<(const LteUeConfig_t &a, const LteUeConfig_t &b);
};

class LteFfConverter {
public:
  static uint16_t double2fpS11dot3(double val);
  static double fpS11dot3toDouble(uint16_t val);
  static double getMinFpS11dot3Value();
};

class BufferSizeLevelBsr {
public:
  static uint32_t BsrId2BufferSize(uint8_t val);
  static uint8_t BufferSize2BsrId(uint32_t val);

  static int m_bufferSizeLevelBsr[64];
};

class TransmissionModesLayers {
public:
  static uint8_t TxMode2LayerNum(uint8_t txMode);
};

struct PhyTransmissionStatParameters {
  int64_t m_timestamp;
  uint16_t m_cellId;
  uint64_t m_imsi;
  uint16_t m_rnti;
  uint8_t m_txMode;
  uint8_t m_layer;
  uint8_t m_mcs;
  uint16_t m_size;
  uint8_t m_rv;
  uint8_t m_ndi;
  uint8_t m_ccId;

  typedef void (*TracedCallback)(const PhyTransmissionStatParameters params);
};

struct PhyReceptionStatParameters {
  int64_t m_timestamp;
  uint16_t m_cellId;
  uint64_t m_imsi;
  uint16_t m_rnti;
  uint8_t m_txMode;
  uint8_t m_layer;
  uint8_t m_mcs;
  uint16_t m_size;
  uint8_t m_rv;
  uint8_t m_ndi;
  uint8_t m_correctness;
  uint8_t m_ccId;

  typedef void (*TracedCallback)(const PhyReceptionStatParameters params);
};

struct DlSchedulingCallbackInfo {
  uint32_t frameNo;
  uint32_t subframeNo;
  uint16_t rnti;
  uint8_t mcsTb1;
  uint16_t sizeTb1;
  uint8_t mcsTb2;
  uint16_t sizeTb2;
  uint8_t componentCarrierId;
};

class EutranMeasurementMapping {
public:
  static double RsrpRange2Dbm(uint8_t range);

  static uint8_t Dbm2RsrpRange(double dbm);

  static double RsrqRange2Db(uint8_t range);

  static uint8_t Db2RsrqRange(double db);

  static double QuantizeRsrp(double v);

  static double QuantizeRsrq(double v);

  static double IeValue2ActualHysteresis(uint8_t hysteresisIeValue);

  static uint8_t ActualHysteresis2IeValue(double hysteresisDb);

  static double IeValue2ActualA3Offset(int8_t a3OffsetIeValue);

  static int8_t ActualA3Offset2IeValue(double a3OffsetDb);

  static double IeValue2ActualQRxLevMin(int8_t qRxLevMinIeValue);

  static double IeValue2ActualQQualMin(int8_t qQualMinIeValue);
};

}; // namespace ns3

#endif
