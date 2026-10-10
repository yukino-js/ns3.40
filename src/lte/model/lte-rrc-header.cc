
#include "lte-rrc-header.h"

#include "ns3/log.h"

#include <sstream>
#include <stdio.h>

#define MAX_DRB 11
#define MAX_EARFCN 262143
#define MAX_RAT_CAPABILITIES 8
#define MAX_SI_MESSAGE 32
#define MAX_SIB 32

#define MAX_REPORT_CONFIG_ID 32
#define MAX_OBJECT_ID 32
#define MAX_MEAS_ID 32
#define MAX_CELL_MEAS 32
#define MAX_CELL_REPORT 8

#define MAX_SCELL_REPORT 5
#define MAX_SCELL_CONF 5

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RrcHeader");

RrcAsn1Header::RrcAsn1Header() {}

TypeId RrcAsn1Header::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::RrcAsn1Header").SetParent<Header>().SetGroupName("Lte");
  return tid;
}

TypeId RrcAsn1Header::GetInstanceTypeId() const { return GetTypeId(); }

int RrcAsn1Header::GetMessageType() const { return m_messageType; }

int RrcAsn1Header::BandwidthToEnum(uint16_t bandwidth) const {
  int n;
  switch (bandwidth) {
  case 6:
    n = 0;
    break;
  case 15:
    n = 1;
    break;
  case 25:
    n = 2;
    break;
  case 50:
    n = 3;
    break;
  case 75:
    n = 4;
    break;
  case 100:
    n = 5;
    break;
  default:
    NS_FATAL_ERROR("Wrong bandwidth: " << bandwidth);
  }
  return n;
}

uint16_t RrcAsn1Header::EnumToBandwidth(int n) const {
  uint16_t bw;
  switch (n) {
  case 0:
    bw = 6;
    break;
  case 1:
    bw = 15;
    break;
  case 2:
    bw = 25;
    break;
  case 3:
    bw = 50;
    break;
  case 4:
    bw = 75;
    break;
  case 5:
    bw = 100;
    break;
  default:
    NS_FATAL_ERROR("Wrong enum value for bandwidth: " << n);
  }
  return bw;
}

void RrcAsn1Header::SerializeDrbToAddModList(
    std::list<LteRrcSap::DrbToAddMod> drbToAddModList) const {
  SerializeSequenceOf(drbToAddModList.size(), MAX_DRB, 1);

  auto it = drbToAddModList.begin();
  for (; it != drbToAddModList.end(); it++) {
    std::bitset<5> drbToAddModListOptionalFieldsPresent = std::bitset<5>();
    drbToAddModListOptionalFieldsPresent.set(4, true);
    drbToAddModListOptionalFieldsPresent.set(3, false);
    drbToAddModListOptionalFieldsPresent.set(2, true);
    drbToAddModListOptionalFieldsPresent.set(1, true);
    drbToAddModListOptionalFieldsPresent.set(0, true);
    SerializeSequence(drbToAddModListOptionalFieldsPresent, true);

    SerializeInteger(it->epsBearerIdentity, 0, 15);

    SerializeInteger(it->drbIdentity, 1, 32);

    switch (it->rlcConfig.choice) {
    case LteRrcSap::RlcConfig::UM_BI_DIRECTIONAL:
      SerializeChoice(4, 1, true);

      SerializeSequence(std::bitset<0>(), false);
      SerializeEnum(2, 0);

      SerializeSequence(std::bitset<0>(), false);
      SerializeEnum(2, 0);
      SerializeEnum(32, 0);
      break;

    case LteRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_UL:
      SerializeChoice(4, 2, true);

      SerializeSequence(std::bitset<0>(), false);
      SerializeEnum(2, 0);
      break;

    case LteRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_DL:
      SerializeChoice(4, 3, true);

      SerializeSequence(std::bitset<0>(), false);
      SerializeEnum(2, 0);
      SerializeEnum(32, 0);
      break;

    case LteRrcSap::RlcConfig::AM:
    default:
      SerializeChoice(4, 0, true);

      SerializeSequence(std::bitset<0>(), false);
      SerializeEnum(64, 0);
      SerializeEnum(8, 0);
      SerializeEnum(16, 0);
      SerializeEnum(8, 0);

      SerializeSequence(std::bitset<0>(), false);
      SerializeEnum(32, 0);
      SerializeEnum(64, 0);
      break;
    }

    SerializeInteger(it->logicalChannelIdentity, 3, 10);

    SerializeLogicalChannelConfig(it->logicalChannelConfig);
  }
}

void RrcAsn1Header::SerializeSrbToAddModList(
    std::list<LteRrcSap::SrbToAddMod> srbToAddModList) const {
  SerializeSequenceOf(srbToAddModList.size(), 2, 1);

  auto it = srbToAddModList.begin();
  for (; it != srbToAddModList.end(); it++) {
    std::bitset<2> srbToAddModListOptionalFieldsPresent = std::bitset<2>();
    srbToAddModListOptionalFieldsPresent.set(1, false);
    srbToAddModListOptionalFieldsPresent.set(0, true);
    SerializeSequence(srbToAddModListOptionalFieldsPresent, true);

    SerializeInteger(it->srbIdentity, 1, 2);

    SerializeChoice(2, 0, false);

    SerializeLogicalChannelConfig(it->logicalChannelConfig);
  }
}

void RrcAsn1Header::SerializeLogicalChannelConfig(
    LteRrcSap::LogicalChannelConfig logicalChannelConfig) const {
  SerializeSequence(std::bitset<1>(1), true);

  SerializeSequence(std::bitset<1>(1), false);

  SerializeInteger(logicalChannelConfig.priority, 1, 16);

  int prioritizedBitRate;
  switch (logicalChannelConfig.prioritizedBitRateKbps) {
  case 0:
    prioritizedBitRate = 0;
    break;
  case 8:
    prioritizedBitRate = 1;
    break;
  case 16:
    prioritizedBitRate = 2;
    break;
  case 32:
    prioritizedBitRate = 3;
    break;
  case 64:
    prioritizedBitRate = 4;
    break;
  case 128:
    prioritizedBitRate = 5;
    break;
  case 256:
    prioritizedBitRate = 6;
    break;
  default:
    prioritizedBitRate = 7;
  }
  SerializeEnum(16, prioritizedBitRate);

  int bucketSizeDuration;
  switch (logicalChannelConfig.bucketSizeDurationMs) {
  case 50:
    bucketSizeDuration = 0;
    break;
  case 100:
    bucketSizeDuration = 1;
    break;
  case 150:
    bucketSizeDuration = 2;
    break;
  case 300:
    bucketSizeDuration = 3;
    break;
  case 500:
    bucketSizeDuration = 4;
    break;
  case 1000:
    bucketSizeDuration = 5;
    break;
  default:
    bucketSizeDuration = 5;
  }
  SerializeEnum(8, bucketSizeDuration);

  SerializeInteger(logicalChannelConfig.logicalChannelGroup, 0, 3);
}

void RrcAsn1Header::SerializePhysicalConfigDedicated(
    LteRrcSap::PhysicalConfigDedicated physicalConfigDedicated) const {
  std::bitset<10> optionalFieldsPhysicalConfigDedicated;
  optionalFieldsPhysicalConfigDedicated.set(
      9, physicalConfigDedicated.havePdschConfigDedicated);
  optionalFieldsPhysicalConfigDedicated.set(8, false);
  optionalFieldsPhysicalConfigDedicated.set(7, false);
  optionalFieldsPhysicalConfigDedicated.set(6, false);
  optionalFieldsPhysicalConfigDedicated.set(5, false);
  optionalFieldsPhysicalConfigDedicated.set(4, false);
  optionalFieldsPhysicalConfigDedicated.set(3, false);
  optionalFieldsPhysicalConfigDedicated.set(
      2, physicalConfigDedicated.haveSoundingRsUlConfigDedicated);
  optionalFieldsPhysicalConfigDedicated.set(
      1, physicalConfigDedicated.haveAntennaInfoDedicated);
  optionalFieldsPhysicalConfigDedicated.set(0, false);
  SerializeSequence(optionalFieldsPhysicalConfigDedicated, true);

  if (physicalConfigDedicated.havePdschConfigDedicated) {
    SerializeSequence(std::bitset<0>(), false);

    SerializeEnum(8, physicalConfigDedicated.pdschConfigDedicated.pa);

    SerializeNull();
  }

  if (physicalConfigDedicated.haveSoundingRsUlConfigDedicated) {
    switch (physicalConfigDedicated.soundingRsUlConfigDedicated.type) {
    case LteRrcSap::SoundingRsUlConfigDedicated::RESET:
      SerializeChoice(2, 0, false);
      SerializeNull();
      break;

    case LteRrcSap::SoundingRsUlConfigDedicated::SETUP:
    default:
      SerializeChoice(2, 1, false);

      SerializeSequence(std::bitset<0>(), false);

      SerializeEnum(
          4, physicalConfigDedicated.soundingRsUlConfigDedicated.srsBandwidth);

      SerializeEnum(4, 0);

      SerializeInteger(0, 0, 23);

      SerializeBoolean(false);

      SerializeInteger(
          physicalConfigDedicated.soundingRsUlConfigDedicated.srsConfigIndex, 0,
          1023);

      SerializeInteger(0, 0, 1);

      SerializeEnum(8, 0);

      break;
    }
  }

  if (physicalConfigDedicated.haveAntennaInfoDedicated) {
    SerializeChoice(2, 0, false);

    SerializeSequence(std::bitset<1>(0), false);

    SerializeEnum(8, physicalConfigDedicated.antennaInfo.transmissionMode);

    SerializeChoice(2, 0, false);

    SerializeNull();
  }
}

void RrcAsn1Header::SerializeRadioResourceConfigDedicated(
    LteRrcSap::RadioResourceConfigDedicated radioResourceConfigDedicated)
    const {
  bool isSrbToAddModListPresent =
      !radioResourceConfigDedicated.srbToAddModList.empty();
  bool isDrbToAddModListPresent =
      !radioResourceConfigDedicated.drbToAddModList.empty();
  bool isDrbToReleaseListPresent =
      !radioResourceConfigDedicated.drbToReleaseList.empty();

  std::bitset<6> optionalFieldsPresent = std::bitset<6>();
  optionalFieldsPresent.set(5, isSrbToAddModListPresent);
  optionalFieldsPresent.set(4, isDrbToAddModListPresent);
  optionalFieldsPresent.set(3, isDrbToReleaseListPresent);
  optionalFieldsPresent.set(2, false);
  optionalFieldsPresent.set(1, false);
  optionalFieldsPresent.set(
      0, radioResourceConfigDedicated.havePhysicalConfigDedicated);
  SerializeSequence(optionalFieldsPresent, true);

  if (isSrbToAddModListPresent) {
    SerializeSrbToAddModList(radioResourceConfigDedicated.srbToAddModList);
  }

  if (isDrbToAddModListPresent) {
    SerializeDrbToAddModList(radioResourceConfigDedicated.drbToAddModList);
  }

  if (isDrbToReleaseListPresent) {
    SerializeSequenceOf(radioResourceConfigDedicated.drbToReleaseList.size(),
                        MAX_DRB, 1);
    auto it = radioResourceConfigDedicated.drbToReleaseList.begin();
    for (; it != radioResourceConfigDedicated.drbToReleaseList.end(); it++) {
      SerializeInteger(*it, 1, 32);
    }
  }

  if (radioResourceConfigDedicated.havePhysicalConfigDedicated) {
    SerializePhysicalConfigDedicated(
        radioResourceConfigDedicated.physicalConfigDedicated);
  }
}

void RrcAsn1Header::SerializeSystemInformationBlockType1(
    LteRrcSap::SystemInformationBlockType1 systemInformationBlockType1) const {
  std::bitset<3> sysInfoBlk1Opts;
  sysInfoBlk1Opts.set(2, false);
  sysInfoBlk1Opts.set(1, false);
  sysInfoBlk1Opts.set(0, false);
  SerializeSequence(sysInfoBlk1Opts, false);

  SerializeSequence(std::bitset<1>(1), false);

  SerializeSequenceOf(1, 6, 1);

  SerializeSequence(std::bitset<0>(), false);

  SerializePlmnIdentity(systemInformationBlockType1.cellAccessRelatedInfo
                            .plmnIdentityInfo.plmnIdentity);

  SerializeBitstring(std::bitset<16>(0));
  SerializeBitstring(std::bitset<28>(
      systemInformationBlockType1.cellAccessRelatedInfo.cellIdentity));
  SerializeEnum(2, 0);
  SerializeEnum(2, 0);
  SerializeBoolean(
      systemInformationBlockType1.cellAccessRelatedInfo.csgIndication);
  SerializeBitstring(std::bitset<27>(
      systemInformationBlockType1.cellAccessRelatedInfo.csgIdentity));

  SerializeSequence(std::bitset<1>(0), false);
  SerializeInteger(-50, -70, -22);

  SerializeInteger(1, 1, 64);

  SerializeSequenceOf(1, MAX_SI_MESSAGE, 1);
  SerializeSequence(std::bitset<0>(), false);
  SerializeEnum(7, 0);
  SerializeSequenceOf(0, MAX_SIB - 1, 0);

  SerializeEnum(7, 0);

  SerializeInteger(0, 0, 31);
}

void RrcAsn1Header::SerializeRadioResourceConfigCommon(
    LteRrcSap::RadioResourceConfigCommon radioResourceConfigCommon) const {
  std::bitset<9> rrCfgCmmOpts;
  rrCfgCmmOpts.set(8, true);
  rrCfgCmmOpts.set(7, false);
  rrCfgCmmOpts.set(6, false);
  rrCfgCmmOpts.set(5, false);
  rrCfgCmmOpts.set(4, false);
  rrCfgCmmOpts.set(3, false);
  rrCfgCmmOpts.set(2, false);
  rrCfgCmmOpts.set(1, false);
  rrCfgCmmOpts.set(0, false);

  SerializeSequence(rrCfgCmmOpts, true);

  if (rrCfgCmmOpts[8]) {
    SerializeRachConfigCommon(radioResourceConfigCommon.rachConfigCommon);
  }

  SerializeSequence(std::bitset<1>(0), false);

  SerializeInteger(0, 0, 1023);

  SerializeSequence(std::bitset<0>(), false);

  SerializeSequence(std::bitset<0>(), false);
  SerializeInteger(1, 1, 4);
  SerializeEnum(2, 0);
  SerializeInteger(0, 0, 98);
  SerializeBoolean(false);

  SerializeSequence(std::bitset<0>(), false);
  SerializeBoolean(false);
  SerializeInteger(0, 0, 29);
  SerializeBoolean(false);
  SerializeInteger(4, 0, 7);

  SerializeEnum(2, 0);
}

void RrcAsn1Header::SerializeRadioResourceConfigCommonSib(
    LteRrcSap::RadioResourceConfigCommonSib radioResourceConfigCommonSib)
    const {
  SerializeSequence(std::bitset<0>(0), true);

  SerializeRachConfigCommon(radioResourceConfigCommonSib.rachConfigCommon);

  SerializeSequence(std::bitset<0>(0), false);
  SerializeEnum(4, 0);
  SerializeSequence(std::bitset<0>(0), false);
  SerializeEnum(4, 0);
  SerializeEnum(8, 0);
  SerializeSequence(std::bitset<1>(0), false);
  SerializeInteger(0, 0, 1023);
  SerializeSequence(std::bitset<0>(0), false);
  SerializeInteger(0, -60, 50);
  SerializeInteger(0, 0, 3);
  SerializeSequence(std::bitset<0>(0), false);
  SerializeSequence(std::bitset<0>(0), false);
  SerializeInteger(1, 1, 4);
  SerializeEnum(2, 0);
  SerializeInteger(0, 0, 98);
  SerializeBoolean(false);
  SerializeSequence(std::bitset<0>(0), false);
  SerializeBoolean(false);
  SerializeInteger(0, 0, 29);
  SerializeBoolean(false);
  SerializeInteger(0, 0, 7);
  SerializeSequence(std::bitset<0>(0), false);
  SerializeEnum(3, 0);
  SerializeInteger(0, 0, 98);
  SerializeInteger(0, 0, 7);
  SerializeInteger(0, 0, 2047);
  SerializeChoice(2, 0, false);
  SerializeNull();
  SerializeSequence(std::bitset<0>(0), false);
  SerializeInteger(0, -126, 24);
  SerializeEnum(8, 0);
  SerializeInteger(-110, -127, -96);
  SerializeSequence(std::bitset<0>(0), false);
  SerializeEnum(3, 0);
  SerializeEnum(3, 0);
  SerializeEnum(4, 0);
  SerializeEnum(3, 0);
  SerializeEnum(3, 0);
  SerializeInteger(0, -1, 6);
  SerializeEnum(2, 0);
}

void RrcAsn1Header::SerializeSystemInformationBlockType2(
    LteRrcSap::SystemInformationBlockType2 systemInformationBlockType2) const {
  SerializeSequence(std::bitset<2>(0), true);

  SerializeRadioResourceConfigCommonSib(
      systemInformationBlockType2.radioResourceConfigCommon);

  SerializeSequence(std::bitset<0>(0), true);
  SerializeEnum(8, 0);
  SerializeEnum(8, 0);
  SerializeEnum(7, 0);
  SerializeEnum(8, 0);
  SerializeEnum(7, 0);
  SerializeEnum(8, 0);

  SerializeSequence(std::bitset<2>(3), false);
  SerializeInteger((int)systemInformationBlockType2.freqInfo.ulCarrierFreq, 0,
                   MAX_EARFCN);
  SerializeEnum(
      6, BandwidthToEnum(systemInformationBlockType2.freqInfo.ulBandwidth));

  SerializeInteger(29, 1, 32);
  SerializeEnum(8, 0);
}

void RrcAsn1Header::SerializeMeasResults(
    LteRrcSap::MeasResults measResults) const {
  if (measResults.measResultListEutra.empty()) {
    measResults.haveMeasResultNeighCells = false;
  }

  std::bitset<4> measResultOptional;
  measResultOptional.set(3, measResults.haveMeasResultServFreqList);
  measResultOptional.set(2, false);
  measResultOptional.set(1, false);
  measResultOptional.set(0, measResults.haveMeasResultNeighCells);
  SerializeSequence(measResultOptional, true);

  SerializeInteger(measResults.measId, 1, MAX_MEAS_ID);

  SerializeSequence(std::bitset<0>(0), false);

  SerializeInteger(measResults.measResultPCell.rsrpResult, 0, 97);

  SerializeInteger(measResults.measResultPCell.rsrqResult, 0, 34);

  if (measResults.haveMeasResultNeighCells) {
    SerializeChoice(4, 0, false);

    SerializeSequenceOf(measResults.measResultListEutra.size(), MAX_CELL_REPORT,
                        1);

    for (auto it = measResults.measResultListEutra.begin();
         it != measResults.measResultListEutra.end(); it++) {
      SerializeSequence(std::bitset<1>(it->haveCgiInfo), false);

      SerializeInteger(it->physCellId, 0, 503);

      if (it->haveCgiInfo) {
        SerializeSequence(std::bitset<1>(it->cgiInfo.plmnIdentityList.size()),
                          false);

        SerializeSequence(std::bitset<0>(0), false);
        SerializePlmnIdentity(it->cgiInfo.plmnIdentity);
        SerializeBitstring(std::bitset<28>(it->cgiInfo.cellIdentity));

        SerializeBitstring(std::bitset<16>(it->cgiInfo.trackingAreaCode));

        if (!it->cgiInfo.plmnIdentityList.empty()) {
          SerializeSequenceOf(it->cgiInfo.plmnIdentityList.size(), 5, 1);
          for (auto it2 = it->cgiInfo.plmnIdentityList.begin();
               it2 != it->cgiInfo.plmnIdentityList.end(); it2++) {
            SerializePlmnIdentity(*it2);
          }
        }
      }

      std::bitset<2> measResultFieldsPresent;
      measResultFieldsPresent[1] = it->haveRsrpResult;
      measResultFieldsPresent[0] = it->haveRsrqResult;
      SerializeSequence(measResultFieldsPresent, true);

      if (it->haveRsrpResult) {
        SerializeInteger(it->rsrpResult, 0, 97);
      }

      if (it->haveRsrqResult) {
        SerializeInteger(it->rsrqResult, 0, 34);
      }
    }
  }

  if (measResults.haveMeasResultServFreqList) {
    SerializeSequenceOf(measResults.measResultServFreqList.size(),
                        MAX_SCELL_REPORT, 1);
    for (const auto &it : measResults.measResultServFreqList) {
      std::bitset<2> measResultServFreqPresent;
      measResultServFreqPresent[0] = it.haveMeasResultSCell;
      measResultServFreqPresent[1] = it.haveMeasResultBestNeighCell;
      SerializeSequence(measResultServFreqPresent, true);

      SerializeInteger(it.servFreqId, 0, 7);

      if (it.haveMeasResultSCell) {
        SerializeInteger(it.measResultSCell.rsrpResult, 0, 97);

        SerializeInteger(it.measResultSCell.rsrqResult, 0, 34);
      }

      if (it.haveMeasResultBestNeighCell) {
        SerializeInteger(it.measResultBestNeighCell.physCellId, 0, 503);

        SerializeInteger(it.measResultBestNeighCell.rsrpResult, 0, 97);

        SerializeInteger(it.measResultBestNeighCell.rsrqResult, 0, 34);
      }

      NS_ASSERT(!it.haveMeasResultBestNeighCell);
    }
  }
}

void RrcAsn1Header::SerializePlmnIdentity(uint32_t plmnId) const {
  SerializeSequence(std::bitset<1>(0), false);

  int nDig = (plmnId > 99) ? 3 : 2;

  SerializeSequenceOf(nDig, 3, 2);
  for (int i = nDig - 1; i >= 0; i--) {
    int n = floor(plmnId / pow(10, i));
    SerializeInteger(n, 0, 9);
    plmnId -= n * pow(10, i);
  }

  SerializeEnum(2, 0);
}

void RrcAsn1Header::SerializeRachConfigCommon(
    LteRrcSap::RachConfigCommon rachConfigCommon) const {
  SerializeSequence(std::bitset<0>(0), true);

  SerializeSequence(std::bitset<1>(0), false);

  switch (rachConfigCommon.preambleInfo.numberOfRaPreambles) {
  case 4:
    SerializeEnum(16, 0);
    break;
  case 8:
    SerializeEnum(16, 1);
    break;
  case 12:
    SerializeEnum(16, 2);
    break;
  case 16:
    SerializeEnum(16, 3);
    break;
  case 20:
    SerializeEnum(16, 4);
    break;
  case 24:
    SerializeEnum(16, 5);
    break;
  case 28:
    SerializeEnum(16, 6);
    break;
  case 32:
    SerializeEnum(16, 7);
    break;
  case 36:
    SerializeEnum(16, 8);
    break;
  case 40:
    SerializeEnum(16, 9);
    break;
  case 44:
    SerializeEnum(16, 10);
    break;
  case 48:
    SerializeEnum(16, 11);
    break;
  case 52:
    SerializeEnum(16, 12);
    break;
  case 56:
    SerializeEnum(16, 13);
    break;
  case 60:
    SerializeEnum(16, 14);
    break;
  case 64:
    SerializeEnum(16, 15);
    break;
  default:
    NS_FATAL_ERROR("Wrong numberOfRA-Preambles value");
  }

  SerializeSequence(std::bitset<0>(0), false);
  SerializeEnum(4, 0);
  SerializeEnum(16, 0);
  SerializeSequence(std::bitset<0>(0), false);

  switch (rachConfigCommon.raSupervisionInfo.preambleTransMax) {
  case 3:
    SerializeEnum(11, 0);
    break;
  case 4:
    SerializeEnum(11, 1);
    break;
  case 5:
    SerializeEnum(11, 2);
    break;
  case 6:
    SerializeEnum(11, 3);
    break;
  case 7:
    SerializeEnum(11, 4);
    break;
  case 8:
    SerializeEnum(11, 5);
    break;
  case 10:
    SerializeEnum(11, 6);
    break;
  case 20:
    SerializeEnum(11, 7);
    break;
  case 50:
    SerializeEnum(11, 8);
    break;
  case 100:
    SerializeEnum(11, 9);
    break;
  case 200:
    SerializeEnum(11, 10);
    break;
  default:
    SerializeEnum(11, 0);
  }

  switch (rachConfigCommon.raSupervisionInfo.raResponseWindowSize) {
  case 2:
    SerializeEnum(8, 0);
    break;
  case 3:
    SerializeEnum(8, 1);
    break;
  case 4:
    SerializeEnum(8, 2);
    break;
  case 5:
    SerializeEnum(8, 3);
    break;
  case 6:
    SerializeEnum(8, 4);
    break;
  case 7:
    SerializeEnum(8, 5);
    break;
  case 8:
    SerializeEnum(8, 6);
    break;
  case 10:
    SerializeEnum(8, 7);
    break;
  default:
    SerializeEnum(8, 0);
  }

  SerializeEnum(8, 0);
  SerializeInteger(1, 1, 8);

  switch (rachConfigCommon.txFailParam.connEstFailCount) {
  case 1:
    SerializeEnum(8, 1);
    break;
  case 2:
    SerializeEnum(8, 2);
    break;
  case 3:
    SerializeEnum(8, 3);
    break;
  case 4:
    SerializeEnum(8, 4);
    break;
  default:
    SerializeEnum(8, 1);
  }
}

void RrcAsn1Header::SerializeQoffsetRange(int8_t qOffsetRange) const {
  switch (qOffsetRange) {
  case -24:
    SerializeEnum(31, 0);
    break;
  case -22:
    SerializeEnum(31, 1);
    break;
  case -20:
    SerializeEnum(31, 2);
    break;
  case -18:
    SerializeEnum(31, 3);
    break;
  case -16:
    SerializeEnum(31, 4);
    break;
  case -14:
    SerializeEnum(31, 5);
    break;
  case -12:
    SerializeEnum(31, 6);
    break;
  case -10:
    SerializeEnum(31, 7);
    break;
  case -8:
    SerializeEnum(31, 8);
    break;
  case -6:
    SerializeEnum(31, 9);
    break;
  case -5:
    SerializeEnum(31, 10);
    break;
  case -4:
    SerializeEnum(31, 11);
    break;
  case -3:
    SerializeEnum(31, 12);
    break;
  case -2:
    SerializeEnum(31, 13);
    break;
  case -1:
    SerializeEnum(31, 14);
    break;
  case 0:
    SerializeEnum(31, 15);
    break;
  case 1:
    SerializeEnum(31, 16);
    break;
  case 2:
    SerializeEnum(31, 17);
    break;
  case 3:
    SerializeEnum(31, 18);
    break;
  case 4:
    SerializeEnum(31, 19);
    break;
  case 5:
    SerializeEnum(31, 20);
    break;
  case 6:
    SerializeEnum(31, 21);
    break;
  case 8:
    SerializeEnum(31, 22);
    break;
  case 10:
    SerializeEnum(31, 23);
    break;
  case 12:
    SerializeEnum(31, 24);
    break;
  case 14:
    SerializeEnum(31, 25);
    break;
  case 16:
    SerializeEnum(31, 26);
    break;
  case 18:
    SerializeEnum(31, 27);
    break;
  case 20:
    SerializeEnum(31, 28);
    break;
  case 22:
    SerializeEnum(31, 29);
    break;
  case 24:
    SerializeEnum(31, 30);
    break;
  default:
    SerializeEnum(31, 15);
  }
}

void RrcAsn1Header::SerializeThresholdEutra(
    LteRrcSap::ThresholdEutra thresholdEutra) const {
  switch (thresholdEutra.choice) {
  case LteRrcSap::ThresholdEutra::THRESHOLD_RSRP:
    SerializeChoice(2, 0, false);
    SerializeInteger(thresholdEutra.range, 0, 97);
    break;
  case LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ:
  default:
    SerializeChoice(2, 1, false);
    SerializeInteger(thresholdEutra.range, 0, 34);
  }
}

void RrcAsn1Header::SerializeMeasConfig(
    LteRrcSap::MeasConfig measConfig) const {
  std::bitset<11> measConfigOptional;
  measConfigOptional.set(10, !measConfig.measObjectToRemoveList.empty());
  measConfigOptional.set(9, !measConfig.measObjectToAddModList.empty());
  measConfigOptional.set(8, !measConfig.reportConfigToRemoveList.empty());
  measConfigOptional.set(7, !measConfig.reportConfigToAddModList.empty());
  measConfigOptional.set(6, !measConfig.measIdToRemoveList.empty());
  measConfigOptional.set(5, !measConfig.measIdToAddModList.empty());
  measConfigOptional.set(4, measConfig.haveQuantityConfig);
  measConfigOptional.set(3, measConfig.haveMeasGapConfig);
  measConfigOptional.set(2, measConfig.haveSmeasure);
  measConfigOptional.set(1, false);
  measConfigOptional.set(0, measConfig.haveSpeedStatePars);
  SerializeSequence(measConfigOptional, true);

  if (!measConfig.measObjectToRemoveList.empty()) {
    SerializeSequenceOf(measConfig.measObjectToRemoveList.size(), MAX_OBJECT_ID,
                        1);
    for (auto it = measConfig.measObjectToRemoveList.begin();
         it != measConfig.measObjectToRemoveList.end(); it++) {
      SerializeInteger(*it, 1, MAX_OBJECT_ID);
    }
  }

  if (!measConfig.measObjectToAddModList.empty()) {
    SerializeSequenceOf(measConfig.measObjectToAddModList.size(), MAX_OBJECT_ID,
                        1);
    for (auto it = measConfig.measObjectToAddModList.begin();
         it != measConfig.measObjectToAddModList.end(); it++) {
      SerializeSequence(std::bitset<0>(), false);
      SerializeInteger(it->measObjectId, 1, MAX_OBJECT_ID);
      SerializeChoice(4, 0, true);

      std::bitset<5> measObjOpts;
      measObjOpts.set(4, !it->measObjectEutra.cellsToRemoveList.empty());
      measObjOpts.set(3, !it->measObjectEutra.cellsToAddModList.empty());
      measObjOpts.set(2, !it->measObjectEutra.blackCellsToRemoveList.empty());
      measObjOpts.set(1, !it->measObjectEutra.blackCellsToAddModList.empty());
      measObjOpts.set(0, it->measObjectEutra.haveCellForWhichToReportCGI);
      SerializeSequence(measObjOpts, true);

      SerializeInteger(it->measObjectEutra.carrierFreq, 0, MAX_EARFCN);

      SerializeEnum(6,
                    BandwidthToEnum(it->measObjectEutra.allowedMeasBandwidth));

      SerializeBoolean(it->measObjectEutra.presenceAntennaPort1);
      SerializeBitstring(std::bitset<2>(it->measObjectEutra.neighCellConfig));
      SerializeQoffsetRange(it->measObjectEutra.offsetFreq);

      if (!it->measObjectEutra.cellsToRemoveList.empty()) {
        SerializeSequenceOf(it->measObjectEutra.cellsToRemoveList.size(),
                            MAX_CELL_MEAS, 1);
        for (auto it2 = it->measObjectEutra.cellsToRemoveList.begin();
             it2 != it->measObjectEutra.cellsToRemoveList.end(); it2++) {
          SerializeInteger(*it2, 1, MAX_CELL_MEAS);
        }
      }

      if (!it->measObjectEutra.cellsToAddModList.empty()) {
        SerializeSequenceOf(it->measObjectEutra.cellsToAddModList.size(),
                            MAX_CELL_MEAS, 1);
        for (auto it2 = it->measObjectEutra.cellsToAddModList.begin();
             it2 != it->measObjectEutra.cellsToAddModList.end(); it2++) {
          SerializeSequence(std::bitset<0>(), false);

          SerializeInteger(it2->cellIndex, 1, MAX_CELL_MEAS);

          SerializeInteger(it2->physCellId, 0, 503);

          SerializeQoffsetRange(it2->cellIndividualOffset);
        }
      }

      if (!it->measObjectEutra.blackCellsToRemoveList.empty()) {
        SerializeSequenceOf(it->measObjectEutra.blackCellsToRemoveList.size(),
                            MAX_CELL_MEAS, 1);
        for (auto it2 = it->measObjectEutra.blackCellsToRemoveList.begin();
             it2 != it->measObjectEutra.blackCellsToRemoveList.end(); it2++) {
          SerializeInteger(*it2, 1, MAX_CELL_MEAS);
        }
      }

      if (!it->measObjectEutra.blackCellsToAddModList.empty()) {
        SerializeSequenceOf(it->measObjectEutra.blackCellsToAddModList.size(),
                            MAX_CELL_MEAS, 1);
        for (auto it2 = it->measObjectEutra.blackCellsToAddModList.begin();
             it2 != it->measObjectEutra.blackCellsToAddModList.end(); it2++) {
          SerializeSequence(std::bitset<0>(), false);
          SerializeInteger(it2->cellIndex, 1, MAX_CELL_MEAS);

          std::bitset<1> rangePresent(it2->physCellIdRange.haveRange);
          SerializeSequence(rangePresent, false);
          SerializeInteger(it2->physCellIdRange.start, 0, 503);
          if (it2->physCellIdRange.haveRange) {
            switch (it2->physCellIdRange.range) {
            case 4:
              SerializeEnum(16, 0);
              break;
            case 8:
              SerializeEnum(16, 1);
              break;
            case 12:
              SerializeEnum(16, 2);
              break;
            case 16:
              SerializeEnum(16, 3);
              break;
            case 24:
              SerializeEnum(16, 4);
              break;
            case 32:
              SerializeEnum(16, 5);
              break;
            case 48:
              SerializeEnum(16, 6);
              break;
            case 64:
              SerializeEnum(16, 7);
              break;
            case 84:
              SerializeEnum(16, 8);
              break;
            case 96:
              SerializeEnum(16, 9);
              break;
            case 128:
              SerializeEnum(16, 10);
              break;
            case 168:
              SerializeEnum(16, 11);
              break;
            case 252:
              SerializeEnum(16, 12);
              break;
            case 504:
              SerializeEnum(16, 13);
              break;
            default:
              SerializeEnum(16, 0);
            }
          }
        }
      }

      if (it->measObjectEutra.haveCellForWhichToReportCGI) {
        SerializeInteger(it->measObjectEutra.cellForWhichToReportCGI, 0, 503);
      }
    }
  }

  if (!measConfig.reportConfigToRemoveList.empty()) {
    SerializeSequenceOf(measConfig.reportConfigToRemoveList.size(),
                        MAX_REPORT_CONFIG_ID, 1);
    for (auto it = measConfig.reportConfigToRemoveList.begin();
         it != measConfig.reportConfigToRemoveList.end(); it++) {
      SerializeInteger(*it, 1, MAX_REPORT_CONFIG_ID);
    }
  }

  if (!measConfig.reportConfigToAddModList.empty()) {
    SerializeSequenceOf(measConfig.reportConfigToAddModList.size(),
                        MAX_REPORT_CONFIG_ID, 1);
    for (auto it = measConfig.reportConfigToAddModList.begin();
         it != measConfig.reportConfigToAddModList.end(); it++) {
      SerializeSequence(std::bitset<0>(), false);
      SerializeInteger(it->reportConfigId, 1, MAX_REPORT_CONFIG_ID);
      SerializeChoice(2, 0, false);

      SerializeSequence(std::bitset<0>(), true);
      switch (it->reportConfigEutra.triggerType) {
      case LteRrcSap::ReportConfigEutra::PERIODICAL:
        SerializeChoice(2, 1, false);
        SerializeSequence(std::bitset<0>(), false);
        switch (it->reportConfigEutra.purpose) {
        case LteRrcSap::ReportConfigEutra::REPORT_CGI:
          SerializeEnum(2, 1);
          break;
        case LteRrcSap::ReportConfigEutra::REPORT_STRONGEST_CELLS:
        default:
          SerializeEnum(2, 0);
        }
        break;
      case LteRrcSap::ReportConfigEutra::EVENT:
      default:
        SerializeChoice(2, 0, false);
        SerializeSequence(std::bitset<0>(), false);
        switch (it->reportConfigEutra.eventId) {
        case LteRrcSap::ReportConfigEutra::EVENT_A1:
          SerializeChoice(5, 0, true);
          SerializeSequence(std::bitset<0>(), false);
          SerializeThresholdEutra(it->reportConfigEutra.threshold1);
          break;
        case LteRrcSap::ReportConfigEutra::EVENT_A2:
          SerializeChoice(5, 1, true);
          SerializeSequence(std::bitset<0>(), false);
          SerializeThresholdEutra(it->reportConfigEutra.threshold1);
          break;
        case LteRrcSap::ReportConfigEutra::EVENT_A3:
          SerializeChoice(5, 2, true);
          SerializeSequence(std::bitset<0>(), false);
          SerializeInteger(it->reportConfigEutra.a3Offset, -30, 30);
          SerializeBoolean(it->reportConfigEutra.reportOnLeave);
          break;
        case LteRrcSap::ReportConfigEutra::EVENT_A4:
          SerializeChoice(5, 3, true);
          SerializeSequence(std::bitset<0>(), false);
          SerializeThresholdEutra(it->reportConfigEutra.threshold1);
          break;
        case LteRrcSap::ReportConfigEutra::EVENT_A5:
        default:
          SerializeChoice(5, 4, true);
          SerializeSequence(std::bitset<0>(), false);
          SerializeThresholdEutra(it->reportConfigEutra.threshold1);
          SerializeThresholdEutra(it->reportConfigEutra.threshold2);
        }

        SerializeInteger(it->reportConfigEutra.hysteresis, 0, 30);

        switch (it->reportConfigEutra.timeToTrigger) {
        case 0:
          SerializeEnum(16, 0);
          break;
        case 40:
          SerializeEnum(16, 1);
          break;
        case 64:
          SerializeEnum(16, 2);
          break;
        case 80:
          SerializeEnum(16, 3);
          break;
        case 100:
          SerializeEnum(16, 4);
          break;
        case 128:
          SerializeEnum(16, 5);
          break;
        case 160:
          SerializeEnum(16, 6);
          break;
        case 256:
          SerializeEnum(16, 7);
          break;
        case 320:
          SerializeEnum(16, 8);
          break;
        case 480:
          SerializeEnum(16, 9);
          break;
        case 512:
          SerializeEnum(16, 10);
          break;
        case 640:
          SerializeEnum(16, 11);
          break;
        case 1024:
          SerializeEnum(16, 12);
          break;
        case 1280:
          SerializeEnum(16, 13);
          break;
        case 2560:
          SerializeEnum(16, 14);
          break;
        case 5120:
        default:
          SerializeEnum(16, 15);
        }
      }

      if (it->reportConfigEutra.triggerQuantity ==
          LteRrcSap::ReportConfigEutra::RSRP) {
        SerializeEnum(2, 0);
      } else {
        SerializeEnum(2, 1);
      }

      if (it->reportConfigEutra.reportQuantity ==
          LteRrcSap::ReportConfigEutra::SAME_AS_TRIGGER_QUANTITY) {
        SerializeEnum(2, 0);
      } else {
        SerializeEnum(2, 1);
      }

      SerializeInteger(it->reportConfigEutra.maxReportCells, 1,
                       MAX_CELL_REPORT);

      switch (it->reportConfigEutra.reportInterval) {
      case LteRrcSap::ReportConfigEutra::MS120:
        SerializeEnum(16, 0);
        break;
      case LteRrcSap::ReportConfigEutra::MS240:
        SerializeEnum(16, 1);
        break;
      case LteRrcSap::ReportConfigEutra::MS480:
        SerializeEnum(16, 2);
        break;
      case LteRrcSap::ReportConfigEutra::MS640:
        SerializeEnum(16, 3);
        break;
      case LteRrcSap::ReportConfigEutra::MS1024:
        SerializeEnum(16, 4);
        break;
      case LteRrcSap::ReportConfigEutra::MS2048:
        SerializeEnum(16, 5);
        break;
      case LteRrcSap::ReportConfigEutra::MS5120:
        SerializeEnum(16, 6);
        break;
      case LteRrcSap::ReportConfigEutra::MS10240:
        SerializeEnum(16, 7);
        break;
      case LteRrcSap::ReportConfigEutra::MIN1:
        SerializeEnum(16, 8);
        break;
      case LteRrcSap::ReportConfigEutra::MIN6:
        SerializeEnum(16, 9);
        break;
      case LteRrcSap::ReportConfigEutra::MIN12:
        SerializeEnum(16, 10);
        break;
      case LteRrcSap::ReportConfigEutra::MIN30:
        SerializeEnum(16, 11);
        break;
      case LteRrcSap::ReportConfigEutra::MIN60:
        SerializeEnum(16, 12);
        break;
      case LteRrcSap::ReportConfigEutra::SPARE3:
        SerializeEnum(16, 13);
        break;
      case LteRrcSap::ReportConfigEutra::SPARE2:
        SerializeEnum(16, 14);
        break;
      case LteRrcSap::ReportConfigEutra::SPARE1:
      default:
        SerializeEnum(16, 15);
      }

      switch (it->reportConfigEutra.reportAmount) {
      case 1:
        SerializeEnum(8, 0);
        break;
      case 2:
        SerializeEnum(8, 1);
        break;
      case 4:
        SerializeEnum(8, 2);
        break;
      case 8:
        SerializeEnum(8, 3);
        break;
      case 16:
        SerializeEnum(8, 4);
        break;
      case 32:
        SerializeEnum(8, 5);
        break;
      case 64:
        SerializeEnum(8, 6);
        break;
      default:
        SerializeEnum(8, 7);
      }
    }
  }

  if (!measConfig.measIdToRemoveList.empty()) {
    SerializeSequenceOf(measConfig.measIdToRemoveList.size(), MAX_MEAS_ID, 1);
    for (auto it = measConfig.measIdToRemoveList.begin();
         it != measConfig.measIdToRemoveList.end(); it++) {
      SerializeInteger(*it, 1, MAX_MEAS_ID);
    }
  }

  if (!measConfig.measIdToAddModList.empty()) {
    SerializeSequenceOf(measConfig.measIdToAddModList.size(), MAX_MEAS_ID, 1);
    for (auto it = measConfig.measIdToAddModList.begin();
         it != measConfig.measIdToAddModList.end(); it++) {
      SerializeInteger(it->measId, 1, MAX_MEAS_ID);
      SerializeInteger(it->measObjectId, 1, MAX_OBJECT_ID);
      SerializeInteger(it->reportConfigId, 1, MAX_REPORT_CONFIG_ID);
    }
  }

  if (measConfig.haveQuantityConfig) {
    std::bitset<4> quantityConfigOpts(0);
    quantityConfigOpts.set(3, true);
    SerializeSequence(quantityConfigOpts, true);
    SerializeSequence(std::bitset<0>(), false);

    switch (measConfig.quantityConfig.filterCoefficientRSRP) {
    case 0:
      SerializeEnum(16, 0);
      break;
    case 1:
      SerializeEnum(16, 1);
      break;
    case 2:
      SerializeEnum(16, 2);
      break;
    case 3:
      SerializeEnum(16, 3);
      break;
    case 4:
      SerializeEnum(16, 4);
      break;
    case 5:
      SerializeEnum(16, 5);
      break;
    case 6:
      SerializeEnum(16, 6);
      break;
    case 7:
      SerializeEnum(16, 7);
      break;
    case 8:
      SerializeEnum(16, 8);
      break;
    case 9:
      SerializeEnum(16, 9);
      break;
    case 11:
      SerializeEnum(16, 10);
      break;
    case 13:
      SerializeEnum(16, 11);
      break;
    case 15:
      SerializeEnum(16, 12);
      break;
    case 17:
      SerializeEnum(16, 13);
      break;
    case 19:
      SerializeEnum(16, 14);
      break;
    default:
      SerializeEnum(16, 4);
    }

    switch (measConfig.quantityConfig.filterCoefficientRSRQ) {
    case 0:
      SerializeEnum(16, 0);
      break;
    case 1:
      SerializeEnum(16, 1);
      break;
    case 2:
      SerializeEnum(16, 2);
      break;
    case 3:
      SerializeEnum(16, 3);
      break;
    case 4:
      SerializeEnum(16, 4);
      break;
    case 5:
      SerializeEnum(16, 5);
      break;
    case 6:
      SerializeEnum(16, 6);
      break;
    case 7:
      SerializeEnum(16, 7);
      break;
    case 8:
      SerializeEnum(16, 8);
      break;
    case 9:
      SerializeEnum(16, 9);
      break;
    case 11:
      SerializeEnum(16, 10);
      break;
    case 13:
      SerializeEnum(16, 11);
      break;
    case 15:
      SerializeEnum(16, 12);
      break;
    case 17:
      SerializeEnum(16, 13);
      break;
    case 19:
      SerializeEnum(16, 14);
      break;
    default:
      SerializeEnum(16, 4);
    }
  }

  if (measConfig.haveMeasGapConfig) {
    switch (measConfig.measGapConfig.type) {
    case LteRrcSap::MeasGapConfig::RESET:
      SerializeChoice(2, 0, false);
      SerializeNull();
      break;
    case LteRrcSap::MeasGapConfig::SETUP:
    default:
      SerializeChoice(2, 1, false);
      SerializeSequence(std::bitset<0>(), false);
      switch (measConfig.measGapConfig.gapOffsetChoice) {
      case LteRrcSap::MeasGapConfig::GP0:
        SerializeChoice(2, 0, true);
        SerializeInteger(measConfig.measGapConfig.gapOffsetValue, 0, 39);
        break;
      case LteRrcSap::MeasGapConfig::GP1:
      default:
        SerializeChoice(2, 1, true);
        SerializeInteger(measConfig.measGapConfig.gapOffsetValue, 0, 79);
      }
    }
  }

  if (measConfig.haveSmeasure) {
    SerializeInteger(measConfig.sMeasure, 0, 97);
  }

  if (measConfig.haveSpeedStatePars) {
    switch (measConfig.speedStatePars.type) {
    case LteRrcSap::SpeedStatePars::RESET:
      SerializeChoice(2, 0, false);
      SerializeNull();
      break;
    case LteRrcSap::SpeedStatePars::SETUP:
    default:
      SerializeChoice(2, 1, false);
      SerializeSequence(std::bitset<0>(), false);
      switch (measConfig.speedStatePars.mobilityStateParameters.tEvaluation) {
      case 30:
        SerializeEnum(8, 0);
        break;
      case 60:
        SerializeEnum(8, 1);
        break;
      case 120:
        SerializeEnum(8, 2);
        break;
      case 180:
        SerializeEnum(8, 3);
        break;
      case 240:
        SerializeEnum(8, 4);
        break;
      default:
        SerializeEnum(8, 5);
        break;
      }

      switch (measConfig.speedStatePars.mobilityStateParameters.tHystNormal) {
      case 30:
        SerializeEnum(8, 0);
        break;
      case 60:
        SerializeEnum(8, 1);
        break;
      case 120:
        SerializeEnum(8, 2);
        break;
      case 180:
        SerializeEnum(8, 3);
        break;
      case 240:
        SerializeEnum(8, 4);
        break;
      default:
        SerializeEnum(8, 5);
        break;
      }

      SerializeInteger(
          measConfig.speedStatePars.mobilityStateParameters.nCellChangeMedium,
          1, 16);
      SerializeInteger(
          measConfig.speedStatePars.mobilityStateParameters.nCellChangeHigh, 1,
          16);

      SerializeSequence(std::bitset<0>(), false);
      switch (measConfig.speedStatePars.timeToTriggerSf.sfMedium) {
      case 25:
        SerializeEnum(4, 0);
        break;
      case 50:
        SerializeEnum(4, 1);
        break;
      case 75:
        SerializeEnum(4, 2);
        break;
      case 100:
      default:
        SerializeEnum(4, 3);
      }

      switch (measConfig.speedStatePars.timeToTriggerSf.sfHigh) {
      case 25:
        SerializeEnum(4, 0);
        break;
      case 50:
        SerializeEnum(4, 1);
        break;
      case 75:
        SerializeEnum(4, 2);
        break;
      case 100:
      default:
        SerializeEnum(4, 3);
      }
    }
  }
}

void RrcAsn1Header::SerializeNonCriticalExtensionConfiguration(
    LteRrcSap::NonCriticalExtensionConfiguration nonCriticalExtension) const {
  std::bitset<3> noncriticalExtension_v1020;
  noncriticalExtension_v1020.set(
      2, !nonCriticalExtension.sCellToReleaseList.empty());
  noncriticalExtension_v1020.set(
      1, !nonCriticalExtension.sCellToAddModList.empty());
  noncriticalExtension_v1020.set(0, false);
  SerializeSequence(noncriticalExtension_v1020, false);

  if (!nonCriticalExtension.sCellToReleaseList.empty()) {
    SerializeSequenceOf(nonCriticalExtension.sCellToReleaseList.size(),
                        MAX_OBJECT_ID, 1);
    for (uint8_t sCellIndex : nonCriticalExtension.sCellToReleaseList) {
      SerializeInteger(sCellIndex, 1, 7);
    }
  }

  if (!nonCriticalExtension.sCellToAddModList.empty()) {
    SerializeSequenceOf(nonCriticalExtension.sCellToAddModList.size(),
                        MAX_OBJECT_ID, 1);
    for (auto &it : nonCriticalExtension.sCellToAddModList) {
      std::bitset<4> sCellToAddMod_r10;
      sCellToAddMod_r10.set(3, true);
      sCellToAddMod_r10.set(2, true);
      sCellToAddMod_r10.set(1, true);
      sCellToAddMod_r10.set(0, it.haveRadioResourceConfigDedicatedSCell);
      SerializeSequence(sCellToAddMod_r10, false);
      SerializeInteger(it.sCellIndex, 1, 7);

      std::bitset<2> cellIdentification_r10;
      cellIdentification_r10.set(1, true);
      cellIdentification_r10.set(0, true);
      SerializeSequence(cellIdentification_r10, false);

      SerializeInteger(it.cellIdentification.physCellId, 1, 65536);
      SerializeInteger(it.cellIdentification.dlCarrierFreq, 1, MAX_EARFCN);

      SerializeRadioResourceConfigCommonSCell(
          it.radioResourceConfigCommonSCell);

      if (it.haveRadioResourceConfigDedicatedSCell) {
        SerializeRadioResourceDedicatedSCell(
            it.radioResourceConfigDedicatedSCell);
      }
    }
  }
}

void RrcAsn1Header::SerializeRadioResourceConfigCommonSCell(
    LteRrcSap::RadioResourceConfigCommonSCell rrccsc) const {
  std::bitset<2> radioResourceConfigCommonSCell_r10;
  radioResourceConfigCommonSCell_r10.set(1, rrccsc.haveNonUlConfiguration);
  radioResourceConfigCommonSCell_r10.set(0, rrccsc.haveUlConfiguration);
  SerializeSequence(radioResourceConfigCommonSCell_r10, false);

  if (rrccsc.haveNonUlConfiguration) {
    std::bitset<5> nonUlConfiguration_r10;
    nonUlConfiguration_r10.set(4, true);
    nonUlConfiguration_r10.set(3, true);
    nonUlConfiguration_r10.set(2, false);
    nonUlConfiguration_r10.set(1, true);
    nonUlConfiguration_r10.set(0, false);
    SerializeSequence(nonUlConfiguration_r10, false);

    SerializeInteger(rrccsc.nonUlConfiguration.dlBandwidth, 6, 100);

    std::bitset<1> antennaInfoCommon_r10;
    antennaInfoCommon_r10.set(0, true);
    SerializeSequence(antennaInfoCommon_r10, false);
    SerializeInteger(
        rrccsc.nonUlConfiguration.antennaInfoCommon.antennaPortsCount, 0,
        65536);

    std::bitset<2> pdschConfigCommon_r10;
    pdschConfigCommon_r10.set(1, true);
    pdschConfigCommon_r10.set(0, true);
    SerializeSequence(pdschConfigCommon_r10, false);

    SerializeInteger(
        rrccsc.nonUlConfiguration.pdschConfigCommon.referenceSignalPower, -60,
        50);
    SerializeInteger(rrccsc.nonUlConfiguration.pdschConfigCommon.pb, 0, 3);
  }
  if (rrccsc.haveUlConfiguration) {
    std::bitset<7> UlConfiguration_r10;
    UlConfiguration_r10.set(6, true);
    UlConfiguration_r10.set(5, false);
    UlConfiguration_r10.set(4, true);
    UlConfiguration_r10.set(3, false);
    UlConfiguration_r10.set(2, false);
    UlConfiguration_r10.set(1, true);
    UlConfiguration_r10.set(0, false);
    SerializeSequence(UlConfiguration_r10, true);

    std::bitset<3> FreqInfo_r10;
    FreqInfo_r10.set(2, true);
    FreqInfo_r10.set(1, true);
    FreqInfo_r10.set(0, false);
    SerializeSequence(FreqInfo_r10, false);

    SerializeInteger(rrccsc.ulConfiguration.ulFreqInfo.ulCarrierFreq, 0,
                     MAX_EARFCN);
    SerializeInteger(rrccsc.ulConfiguration.ulFreqInfo.ulBandwidth, 6, 100);

    std::bitset<2> UlPowerControlCommonSCell_r10;
    UlPowerControlCommonSCell_r10.set(1, false);
    UlPowerControlCommonSCell_r10.set(0, true);
    SerializeSequence(UlPowerControlCommonSCell_r10, false);

    SerializeInteger(rrccsc.ulConfiguration.ulPowerControlCommonSCell.alpha, 0,
                     65536);

    std::bitset<1> prachConfigSCell_r10;
    prachConfigSCell_r10.set(0, true);
    SerializeSequence(prachConfigSCell_r10, false);
    SerializeInteger(rrccsc.ulConfiguration.prachConfigSCell.index, 0, 256);
  }
}

void RrcAsn1Header::SerializeRadioResourceDedicatedSCell(
    LteRrcSap::RadioResourceConfigDedicatedSCell rrcdsc) const {
  std::bitset<1> RadioResourceConfigDedicatedSCell_r10;
  RadioResourceConfigDedicatedSCell_r10.set(0, true);
  SerializeSequence(RadioResourceConfigDedicatedSCell_r10, false);

  LteRrcSap::PhysicalConfigDedicatedSCell pcdsc =
      rrcdsc.physicalConfigDedicatedSCell;
  SerializePhysicalConfigDedicatedSCell(pcdsc);
}

void RrcAsn1Header::SerializePhysicalConfigDedicatedSCell(
    LteRrcSap::PhysicalConfigDedicatedSCell pcdsc) const {
  std::bitset<2> pcdscOpt;
  pcdscOpt.set(1, pcdsc.haveNonUlConfiguration);
  pcdscOpt.set(0, pcdsc.haveUlConfiguration);
  SerializeSequence(pcdscOpt, true);

  if (pcdsc.haveNonUlConfiguration) {
    std::bitset<4> nulOpt;
    nulOpt.set(3, pcdsc.haveAntennaInfoDedicated);
    nulOpt.set(2, false);
    nulOpt.set(1, false);
    nulOpt.set(0, pcdsc.havePdschConfigDedicated);
    SerializeSequence(nulOpt, false);

    if (pcdsc.haveAntennaInfoDedicated) {
      SerializeChoice(2, 0, false);

      SerializeSequence(std::bitset<1>(0), false);

      SerializeEnum(8, pcdsc.antennaInfo.transmissionMode);

      SerializeChoice(2, 0, false);

      SerializeNull();
    }
    if (pcdsc.havePdschConfigDedicated) {
      SerializeSequence(std::bitset<0>(), false);

      SerializeEnum(8, pcdsc.pdschConfigDedicated.pa);

      SerializeNull();
    }
  }
  if (pcdsc.haveUlConfiguration) {
    std::bitset<7> ulOpt;
    ulOpt.set(6, pcdsc.haveAntennaInfoUlDedicated);
    ulOpt.set(5, false);
    ulOpt.set(4, false);
    ulOpt.set(3, false);
    ulOpt.set(2, pcdsc.haveSoundingRsUlConfigDedicated);
    ulOpt.set(1, false);
    ulOpt.set(0, false);
    SerializeSequence(ulOpt, false);

    if (pcdsc.haveAntennaInfoUlDedicated) {
      SerializeChoice(2, 0, false);

      SerializeSequence(std::bitset<1>(0), false);

      SerializeEnum(8, pcdsc.antennaInfoUl.transmissionMode);

      SerializeChoice(2, 0, false);

      SerializeNull();
    }
    if (pcdsc.haveSoundingRsUlConfigDedicated) {
      switch (pcdsc.soundingRsUlConfigDedicated.type) {
      case LteRrcSap::SoundingRsUlConfigDedicated::RESET:
        SerializeChoice(2, 0, false);
        SerializeNull();
        break;

      case LteRrcSap::SoundingRsUlConfigDedicated::SETUP:
      default:
        SerializeChoice(2, 1, false);

        SerializeSequence(std::bitset<0>(), false);

        SerializeEnum(4, pcdsc.soundingRsUlConfigDedicated.srsBandwidth);

        SerializeEnum(4, 0);

        SerializeInteger(0, 0, 23);

        SerializeBoolean(false);

        SerializeInteger(pcdsc.soundingRsUlConfigDedicated.srsConfigIndex, 0,
                         1023);

        SerializeInteger(0, 0, 1);

        SerializeEnum(8, 0);

        break;
      }
    }
  }
}

Buffer::Iterator RrcAsn1Header::DeserializeThresholdEutra(
    LteRrcSap::ThresholdEutra *thresholdEutra, Buffer::Iterator bIterator) {
  int thresholdEutraChoice;
  int range;
  bIterator = DeserializeChoice(2, false, &thresholdEutraChoice, bIterator);

  switch (thresholdEutraChoice) {
  case 0:
    thresholdEutra->choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRP;
    bIterator = DeserializeInteger(&range, 0, 97, bIterator);
    thresholdEutra->range = range;
    break;
  case 1:
  default:
    thresholdEutra->choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
    bIterator = DeserializeInteger(&range, 0, 34, bIterator);
    thresholdEutra->range = range;
  }

  return bIterator;
}

Buffer::Iterator
RrcAsn1Header::DeserializeQoffsetRange(int8_t *qOffsetRange,
                                       Buffer::Iterator bIterator) {
  int n;
  bIterator = DeserializeEnum(31, &n, bIterator);
  switch (n) {
  case 0:
    *qOffsetRange = -24;
    break;
  case 1:
    *qOffsetRange = -22;
    break;
  case 2:
    *qOffsetRange = -20;
    break;
  case 3:
    *qOffsetRange = -18;
    break;
  case 4:
    *qOffsetRange = -16;
    break;
  case 5:
    *qOffsetRange = -14;
    break;
  case 6:
    *qOffsetRange = -12;
    break;
  case 7:
    *qOffsetRange = -10;
    break;
  case 8:
    *qOffsetRange = -8;
    break;
  case 9:
    *qOffsetRange = -6;
    break;
  case 10:
    *qOffsetRange = -5;
    break;
  case 11:
    *qOffsetRange = -4;
    break;
  case 12:
    *qOffsetRange = -3;
    break;
  case 13:
    *qOffsetRange = -2;
    break;
  case 14:
    *qOffsetRange = -1;
    break;
  case 15:
    *qOffsetRange = 0;
    break;
  case 16:
    *qOffsetRange = 1;
    break;
  case 17:
    *qOffsetRange = 2;
    break;
  case 18:
    *qOffsetRange = 3;
    break;
  case 19:
    *qOffsetRange = 4;
    break;
  case 20:
    *qOffsetRange = 5;
    break;
  case 21:
    *qOffsetRange = 6;
    break;
  case 22:
    *qOffsetRange = 8;
    break;
  case 23:
    *qOffsetRange = 10;
    break;
  case 24:
    *qOffsetRange = 12;
    break;
  case 25:
    *qOffsetRange = 14;
    break;
  case 26:
    *qOffsetRange = 16;
    break;
  case 27:
    *qOffsetRange = 18;
    break;
  case 28:
    *qOffsetRange = 20;
    break;
  case 29:
    *qOffsetRange = 22;
    break;
  case 30:
  default:
    *qOffsetRange = 24;
  }
  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializeRadioResourceConfigDedicated(
    LteRrcSap::RadioResourceConfigDedicated *radioResourceConfigDedicated,
    Buffer::Iterator bIterator) {
  std::bitset<6> optionalFieldsPresent = std::bitset<6>();
  bIterator = DeserializeSequence(&optionalFieldsPresent, true, bIterator);

  if (optionalFieldsPresent[5]) {
    bIterator = DeserializeSrbToAddModList(
        &(radioResourceConfigDedicated->srbToAddModList), bIterator);
  }

  if (optionalFieldsPresent[4]) {
    bIterator = DeserializeDrbToAddModList(
        &(radioResourceConfigDedicated->drbToAddModList), bIterator);
  }

  if (optionalFieldsPresent[3]) {
    int n;
    int val;
    bIterator = DeserializeSequenceOf(&n, MAX_DRB, 1, bIterator);
    for (int i = 0; i < n; i++) {
      bIterator = DeserializeInteger(&val, 1, 32, bIterator);
      radioResourceConfigDedicated->drbToReleaseList.push_back(val);
    }
  }

  if (optionalFieldsPresent[2]) {
  }

  if (optionalFieldsPresent[1]) {
  }

  radioResourceConfigDedicated->havePhysicalConfigDedicated =
      optionalFieldsPresent[0];
  if (optionalFieldsPresent[0]) {
    bIterator = DeserializePhysicalConfigDedicated(
        &radioResourceConfigDedicated->physicalConfigDedicated, bIterator);
  }

  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializeSrbToAddModList(
    std::list<LteRrcSap::SrbToAddMod> *srbToAddModList,
    Buffer::Iterator bIterator) {
  int numElems;
  bIterator = DeserializeSequenceOf(&numElems, 2, 1, bIterator);

  srbToAddModList->clear();

  for (int i = 0; i < numElems; i++) {
    LteRrcSap::SrbToAddMod srbToAddMod;
    std::bitset<2> optionalFields;
    bIterator = DeserializeSequence(&optionalFields, true, bIterator);

    int n;
    bIterator = DeserializeInteger(&n, 1, 2, bIterator);
    srbToAddMod.srbIdentity = n;

    if (optionalFields[1]) {
    }

    if (optionalFields[0]) {
      int sel;
      bIterator = DeserializeChoice(2, false, &sel, bIterator);

      if (sel == 1) {
        bIterator = DeserializeNull(bIterator);
      }

      else if (sel == 0) {
        bIterator = DeserializeLogicalChannelConfig(
            &srbToAddMod.logicalChannelConfig, bIterator);
      }
    }
    srbToAddModList->insert(srbToAddModList->end(), srbToAddMod);
  }

  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializeDrbToAddModList(
    std::list<LteRrcSap::DrbToAddMod> *drbToAddModList,
    Buffer::Iterator bIterator) {
  int n;
  int val;
  bIterator = DeserializeSequenceOf(&n, MAX_DRB, 1, bIterator);

  drbToAddModList->clear();

  for (int i = 0; i < n; i++) {
    LteRrcSap::DrbToAddMod drbToAddMod;

    std::bitset<5> optionalFields;
    bIterator = DeserializeSequence(&optionalFields, true, bIterator);

    if (optionalFields[4]) {
      bIterator = DeserializeInteger(&val, 0, 15, bIterator);
      drbToAddMod.epsBearerIdentity = val;
    }

    bIterator = DeserializeInteger(&val, 1, 32, bIterator);
    drbToAddMod.drbIdentity = val;

    if (optionalFields[3]) {
    }

    if (optionalFields[2]) {
      int chosen;
      bIterator = DeserializeChoice(4, true, &chosen, bIterator);

      int sel;
      std::bitset<0> bitset0;
      switch (chosen) {
      case 0:
        drbToAddMod.rlcConfig.choice = LteRrcSap::RlcConfig::AM;

        bIterator = DeserializeSequence(&bitset0, false, bIterator);
        bIterator = DeserializeEnum(64, &sel, bIterator);
        bIterator = DeserializeEnum(8, &sel, bIterator);
        bIterator = DeserializeEnum(16, &sel, bIterator);
        bIterator = DeserializeEnum(8, &sel, bIterator);

        bIterator = DeserializeSequence(&bitset0, false, bIterator);
        bIterator = DeserializeEnum(32, &sel, bIterator);
        bIterator = DeserializeEnum(64, &sel, bIterator);
        break;

      case 1:
        drbToAddMod.rlcConfig.choice = LteRrcSap::RlcConfig::UM_BI_DIRECTIONAL;

        bIterator = DeserializeSequence(&bitset0, false, bIterator);
        bIterator = DeserializeEnum(2, &sel, bIterator);

        bIterator = DeserializeSequence(&bitset0, false, bIterator);
        bIterator = DeserializeEnum(2, &sel, bIterator);
        bIterator = DeserializeEnum(32, &sel, bIterator);
        break;

      case 2:
        drbToAddMod.rlcConfig.choice =
            LteRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_UL;

        bIterator = DeserializeSequence(&bitset0, false, bIterator);
        bIterator = DeserializeEnum(2, &sel, bIterator);
        break;

      case 3:
        drbToAddMod.rlcConfig.choice =
            LteRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_DL;

        bIterator = DeserializeSequence(&bitset0, false, bIterator);
        bIterator = DeserializeEnum(2, &sel, bIterator);
        bIterator = DeserializeEnum(32, &sel, bIterator);
        break;
      }
    }

    if (optionalFields[1]) {
      bIterator = DeserializeInteger(&val, 3, 10, bIterator);
      drbToAddMod.logicalChannelIdentity = val;
    }

    if (optionalFields[0]) {
      bIterator = DeserializeLogicalChannelConfig(
          &drbToAddMod.logicalChannelConfig, bIterator);
    }

    drbToAddModList->insert(drbToAddModList->end(), drbToAddMod);
  }
  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializeLogicalChannelConfig(
    LteRrcSap::LogicalChannelConfig *logicalChannelConfig,
    Buffer::Iterator bIterator) {
  int n;

  std::bitset<1> bitset1;
  bIterator = DeserializeSequence(&bitset1, true, bIterator);

  if (bitset1[0]) {
    bIterator = DeserializeSequence(&bitset1, false, bIterator);

    bIterator = DeserializeInteger(&n, 1, 16, bIterator);
    logicalChannelConfig->priority = n;

    bIterator = DeserializeEnum(16, &n, bIterator);
    uint16_t prioritizedBitRateKbps;

    switch (n) {
    case 0:
      prioritizedBitRateKbps = 0;
      break;
    case 1:
      prioritizedBitRateKbps = 8;
      break;
    case 2:
      prioritizedBitRateKbps = 16;
      break;
    case 3:
      prioritizedBitRateKbps = 32;
      break;
    case 4:
      prioritizedBitRateKbps = 64;
      break;
    case 5:
      prioritizedBitRateKbps = 128;
      break;
    case 6:
      prioritizedBitRateKbps = 256;
      break;
    case 7:
      prioritizedBitRateKbps = 10000;
      break;
    default:
      prioritizedBitRateKbps = 10000;
    }
    logicalChannelConfig->prioritizedBitRateKbps = prioritizedBitRateKbps;

    bIterator = DeserializeEnum(8, &n, bIterator);
    uint16_t bucketSizeDurationMs;
    switch (n) {
    case 0:
      bucketSizeDurationMs = 50;
      break;
    case 1:
      bucketSizeDurationMs = 100;
      break;
    case 2:
      bucketSizeDurationMs = 150;
      break;
    case 3:
      bucketSizeDurationMs = 300;
      break;
    case 4:
      bucketSizeDurationMs = 500;
      break;
    case 5:
      bucketSizeDurationMs = 1000;
      break;
    default:
      bucketSizeDurationMs = 1000;
    }
    logicalChannelConfig->bucketSizeDurationMs = bucketSizeDurationMs;

    if (bitset1[0]) {
      bIterator = DeserializeInteger(&n, 0, 3, bIterator);
      logicalChannelConfig->logicalChannelGroup = n;
    }
  }
  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializePhysicalConfigDedicated(
    LteRrcSap::PhysicalConfigDedicated *physicalConfigDedicated,
    Buffer::Iterator bIterator) {
  std::bitset<10> optionalFieldPresent;
  bIterator = DeserializeSequence(&optionalFieldPresent, true, bIterator);

  physicalConfigDedicated->havePdschConfigDedicated = optionalFieldPresent[9];
  if (optionalFieldPresent[9]) {
    std::bitset<0> bitset0;
    bIterator = DeserializeSequence(&bitset0, false, bIterator);

    int slct;

    bIterator = DeserializeEnum(8, &slct, bIterator);
    physicalConfigDedicated->pdschConfigDedicated.pa = slct;

    bIterator = DeserializeNull(bIterator);
  }
  if (optionalFieldPresent[8]) {
  }
  if (optionalFieldPresent[7]) {
  }
  if (optionalFieldPresent[6]) {
  }
  if (optionalFieldPresent[5]) {
  }
  if (optionalFieldPresent[4]) {
  }
  if (optionalFieldPresent[3]) {
  }
  physicalConfigDedicated->haveSoundingRsUlConfigDedicated =
      optionalFieldPresent[2];
  if (optionalFieldPresent[2]) {
    int sel;
    bIterator = DeserializeChoice(2, false, &sel, bIterator);

    if (sel == 0) {
      physicalConfigDedicated->soundingRsUlConfigDedicated.type =
          LteRrcSap::SoundingRsUlConfigDedicated::RESET;

      bIterator = DeserializeNull(bIterator);
    }

    else if (sel == 1) {
      physicalConfigDedicated->soundingRsUlConfigDedicated.type =
          LteRrcSap::SoundingRsUlConfigDedicated::SETUP;

      std::bitset<0> bitset0;
      bIterator = DeserializeSequence(&bitset0, false, bIterator);

      int slct;

      bIterator = DeserializeEnum(4, &slct, bIterator);
      physicalConfigDedicated->soundingRsUlConfigDedicated.srsBandwidth = slct;

      bIterator = DeserializeEnum(4, &slct, bIterator);

      bIterator = DeserializeInteger(&slct, 0, 23, bIterator);

      bool duration;
      bIterator = DeserializeBoolean(&duration, bIterator);

      bIterator = DeserializeInteger(&slct, 0, 1023, bIterator);
      physicalConfigDedicated->soundingRsUlConfigDedicated.srsConfigIndex =
          slct;

      bIterator = DeserializeInteger(&slct, 0, 1, bIterator);

      bIterator = DeserializeEnum(8, &slct, bIterator);
    }
  }
  physicalConfigDedicated->haveAntennaInfoDedicated = optionalFieldPresent[1];
  if (optionalFieldPresent[1]) {
    int sel;
    bIterator = DeserializeChoice(2, false, &sel, bIterator);
    if (sel == 1) {
      bIterator = DeserializeNull(bIterator);
    } else if (sel == 0) {
      std::bitset<1> codebookSubsetRestrictionPresent;
      bIterator = DeserializeSequence(&codebookSubsetRestrictionPresent, false,
                                      bIterator);

      int txmode;
      bIterator = DeserializeEnum(8, &txmode, bIterator);
      physicalConfigDedicated->antennaInfo.transmissionMode = txmode;

      if (codebookSubsetRestrictionPresent[0]) {
      }

      int txantennaselchosen;
      bIterator = DeserializeChoice(2, false, &txantennaselchosen, bIterator);
      if (txantennaselchosen == 0) {
        bIterator = DeserializeNull(bIterator);
      } else if (txantennaselchosen == 1) {
      }
    }
  }
  if (optionalFieldPresent[0]) {
  }
  return bIterator;
}

void RrcAsn1Header::Print(std::ostream &os) const {
  NS_LOG_FUNCTION(this << &os);
  NS_FATAL_ERROR(
      "RrcAsn1Header Print() function must also specify "
      "LteRrcSap::RadioResourceConfigDedicated as a second argument");
}

Buffer::Iterator RrcAsn1Header::DeserializeNonCriticalExtensionConfig(
    LteRrcSap::NonCriticalExtensionConfiguration *nonCriticalExtension,
    Buffer::Iterator bIterator) {
  NS_LOG_FUNCTION(this);
  std::bitset<2> nonCriticalExtension_v890;
  bIterator = DeserializeSequence(&nonCriticalExtension_v890, false, bIterator);

  if (nonCriticalExtension_v890[0]) {
    std::bitset<3> nonCriticalExtension_v920;
    bIterator =
        DeserializeSequence(&nonCriticalExtension_v920, false, bIterator);
    if (nonCriticalExtension_v920[0]) {
      std::bitset<3> nonCriticalExtension_v1020;
      bIterator =
          DeserializeSequence(&nonCriticalExtension_v1020, false, bIterator);

      if (nonCriticalExtension_v1020[2]) {
        int numElems;

        bIterator =
            DeserializeSequenceOf(&numElems, MAX_OBJECT_ID, 1, bIterator);
        nonCriticalExtension->sCellToReleaseList.clear();

        for (int i = 0; i < numElems; i++) {
          int sCellIndex;
          bIterator = DeserializeInteger(&sCellIndex, 1, 7, bIterator);
          nonCriticalExtension->sCellToReleaseList.push_back(sCellIndex);
        }
      }

      if (nonCriticalExtension_v1020[1]) {

        int numElems;
        bIterator =
            DeserializeSequenceOf(&numElems, MAX_OBJECT_ID, 1, bIterator);
        nonCriticalExtension->sCellToAddModList.clear();
        for (int i = 0; i < numElems; i++) {
          std::bitset<4> sCellToAddMod_r10;
          bIterator = DeserializeSequence(&sCellToAddMod_r10, false, bIterator);

          LteRrcSap::SCellToAddMod sctam;
          NS_ASSERT(sCellToAddMod_r10[3]);
          int n;
          bIterator = DeserializeInteger(&n, 1, 7, bIterator);
          sctam.sCellIndex = n;
          NS_ASSERT(sCellToAddMod_r10[2]);
          bIterator = DeserializeCellIdentification(&sctam.cellIdentification,
                                                    bIterator);

          NS_ASSERT(sCellToAddMod_r10[1]);
          bIterator = DeserializeRadioResourceConfigCommonSCell(
              &sctam.radioResourceConfigCommonSCell, bIterator);
          if (sCellToAddMod_r10[0]) {
            sctam.haveRadioResourceConfigDedicatedSCell = true;
            bIterator = DeserializeRadioResourceConfigDedicatedSCell(
                &sctam.radioResourceConfigDedicatedSCell, bIterator);
          } else {
            sctam.haveRadioResourceConfigDedicatedSCell = false;
          }

          nonCriticalExtension->sCellToAddModList.push_back(sctam);
        }
      }

      NS_ASSERT(!nonCriticalExtension_v1020[0]);
    }
  }

  return bIterator;
}

Buffer::Iterator
RrcAsn1Header::DeserializeCellIdentification(LteRrcSap::CellIdentification *ci,
                                             Buffer::Iterator bIterator) {
  NS_LOG_FUNCTION(this);
  std::bitset<2> cellIdentification_r10;
  bIterator = DeserializeSequence(&cellIdentification_r10, false, bIterator);
  NS_ASSERT(cellIdentification_r10[1]);
  int n1;
  bIterator = DeserializeInteger(&n1, 1, 65536, bIterator);
  ci->physCellId = n1;
  int n2;
  NS_ASSERT(cellIdentification_r10[0]);
  bIterator = DeserializeInteger(&n2, 1, MAX_EARFCN, bIterator);
  ci->dlCarrierFreq = n2;

  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializeRadioResourceConfigCommonSCell(
    LteRrcSap::RadioResourceConfigCommonSCell *rrccsc,
    Buffer::Iterator bIterator) {
  NS_LOG_FUNCTION(this);
  std::bitset<2> radioResourceConfigCommonSCell_r10;
  bIterator = DeserializeSequence(&radioResourceConfigCommonSCell_r10, false,
                                  bIterator);
  rrccsc->haveNonUlConfiguration = radioResourceConfigCommonSCell_r10[1];
  rrccsc->haveUlConfiguration = radioResourceConfigCommonSCell_r10[0];
  if (rrccsc->haveNonUlConfiguration) {
    std::bitset<5> nonUlConfiguration_r10;
    bIterator = DeserializeSequence(&nonUlConfiguration_r10, false, bIterator);
    int n;
    bIterator = DeserializeInteger(&n, 6, 100, bIterator);
    rrccsc->nonUlConfiguration.dlBandwidth = n;

    std::bitset<1> antennaInfoCommon_r10;
    bIterator = DeserializeSequence(&antennaInfoCommon_r10, false, bIterator);
    bIterator = DeserializeInteger(&n, 0, 65536, bIterator);
    rrccsc->nonUlConfiguration.antennaInfoCommon.antennaPortsCount = n;

    std::bitset<2> pdschConfigCommon_r10;
    bIterator = DeserializeSequence(&pdschConfigCommon_r10, false, bIterator);
    bIterator = DeserializeInteger(&n, -60, 50, bIterator);
    rrccsc->nonUlConfiguration.pdschConfigCommon.referenceSignalPower = n;
    bIterator = DeserializeInteger(&n, 0, 3, bIterator);
    rrccsc->nonUlConfiguration.pdschConfigCommon.pb = n;
  }
  if (rrccsc->haveUlConfiguration) {
    std::bitset<7> UlConfiguration_r10;
    bIterator = DeserializeSequence(&UlConfiguration_r10, true, bIterator);

    std::bitset<3> FreqInfo_r10;
    bIterator = DeserializeSequence(&FreqInfo_r10, false, bIterator);
    int n;
    bIterator = DeserializeInteger(&n, 0, MAX_EARFCN, bIterator);
    rrccsc->ulConfiguration.ulFreqInfo.ulCarrierFreq = n;
    bIterator = DeserializeInteger(&n, 6, 100, bIterator);
    rrccsc->ulConfiguration.ulFreqInfo.ulBandwidth = n;

    std::bitset<2> UlPowerControlCommonSCell_r10;
    bIterator =
        DeserializeSequence(&UlPowerControlCommonSCell_r10, false, bIterator);
    bIterator = DeserializeInteger(&n, 0, 65536, bIterator);
    rrccsc->ulConfiguration.ulPowerControlCommonSCell.alpha = n;

    std::bitset<1> prachConfigSCell_r10;
    bIterator = DeserializeSequence(&prachConfigSCell_r10, false, bIterator);
    bIterator = DeserializeInteger(&n, 0, 256, bIterator);
    rrccsc->ulConfiguration.prachConfigSCell.index = n;
  }

  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializeRadioResourceConfigDedicatedSCell(
    LteRrcSap::RadioResourceConfigDedicatedSCell *rrcdsc,
    Buffer::Iterator bIterator) {
  NS_LOG_FUNCTION(this);
  std::bitset<1> RadioResourceConfigDedicatedSCell_r10;
  bIterator = DeserializeSequence(&RadioResourceConfigDedicatedSCell_r10, false,
                                  bIterator);
  bIterator = DeserializePhysicalConfigDedicatedSCell(
      &rrcdsc->physicalConfigDedicatedSCell, bIterator);

  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializePhysicalConfigDedicatedSCell(
    LteRrcSap::PhysicalConfigDedicatedSCell *pcdsc,
    Buffer::Iterator bIterator) {
  NS_LOG_FUNCTION(this);
  std::bitset<2> pcdscOpt;
  bIterator = DeserializeSequence(&pcdscOpt, true, bIterator);
  pcdsc->haveNonUlConfiguration = pcdscOpt[1];
  pcdsc->haveUlConfiguration = pcdscOpt[0];
  if (pcdsc->haveNonUlConfiguration) {
    std::bitset<4> nulOpt;
    bIterator = DeserializeSequence(&nulOpt, false, bIterator);
    pcdsc->haveAntennaInfoDedicated = nulOpt[3];
    NS_ASSERT(!nulOpt[2]);
    NS_ASSERT(!nulOpt[1]);
    pcdsc->havePdschConfigDedicated = nulOpt[0];

    if (pcdsc->haveAntennaInfoDedicated) {
      int sel;
      bIterator = DeserializeChoice(2, false, &sel, bIterator);
      if (sel == 1) {
        bIterator = DeserializeNull(bIterator);
      } else if (sel == 0) {
        std::bitset<1> codebookSubsetRestrictionPresent;
        bIterator = DeserializeSequence(&codebookSubsetRestrictionPresent,
                                        false, bIterator);

        int txmode;
        bIterator = DeserializeEnum(8, &txmode, bIterator);
        pcdsc->antennaInfo.transmissionMode = txmode;

        if (codebookSubsetRestrictionPresent[0]) {
          NS_FATAL_ERROR("Not implemented yet");
        }

        int txantennaselchosen;
        bIterator = DeserializeChoice(2, false, &txantennaselchosen, bIterator);
        if (txantennaselchosen == 0) {
          bIterator = DeserializeNull(bIterator);
        } else if (txantennaselchosen == 1) {
          NS_FATAL_ERROR("Not implemented yet");
        }
      }
    }
    if (pcdsc->havePdschConfigDedicated) {
      std::bitset<0> bitset0;
      bIterator = DeserializeSequence(&bitset0, false, bIterator);

      int slct;

      bIterator = DeserializeEnum(8, &slct, bIterator);
      pcdsc->pdschConfigDedicated.pa = slct;

      bIterator = DeserializeNull(bIterator);
    }
  }
  if (pcdsc->haveUlConfiguration) {
    std::bitset<7> ulOpt;
    bIterator = DeserializeSequence(&ulOpt, false, bIterator);
    pcdsc->haveAntennaInfoUlDedicated = ulOpt[6];
    NS_ASSERT(!ulOpt[5]);
    NS_ASSERT(!ulOpt[4]);
    NS_ASSERT(!ulOpt[3]);
    pcdsc->haveSoundingRsUlConfigDedicated = ulOpt[2];
    NS_ASSERT(!ulOpt[1]);
    NS_ASSERT(!ulOpt[0]);

    if (pcdsc->haveAntennaInfoUlDedicated) {
      int sel;
      bIterator = DeserializeChoice(2, false, &sel, bIterator);
      if (sel == 1) {
        bIterator = DeserializeNull(bIterator);
      } else if (sel == 0) {
        std::bitset<1> codebookSubsetRestrictionPresent;
        bIterator = DeserializeSequence(&codebookSubsetRestrictionPresent,
                                        false, bIterator);

        int txmode;
        bIterator = DeserializeEnum(8, &txmode, bIterator);
        pcdsc->antennaInfoUl.transmissionMode = txmode;

        if (codebookSubsetRestrictionPresent[0]) {
          NS_FATAL_ERROR("Not implemented yet");
        }

        int txantennaselchosen;
        bIterator = DeserializeChoice(2, false, &txantennaselchosen, bIterator);
        if (txantennaselchosen == 0) {
          bIterator = DeserializeNull(bIterator);
        } else if (txantennaselchosen == 1) {
          NS_FATAL_ERROR("Not implemented yet");
        }
      }
    }
    if (pcdsc->haveSoundingRsUlConfigDedicated) {
      int sel;
      bIterator = DeserializeChoice(2, false, &sel, bIterator);

      if (sel == 0) {
        pcdsc->soundingRsUlConfigDedicated.type =
            LteRrcSap::SoundingRsUlConfigDedicated::RESET;

        bIterator = DeserializeNull(bIterator);
      }

      else if (sel == 1) {
        pcdsc->soundingRsUlConfigDedicated.type =
            LteRrcSap::SoundingRsUlConfigDedicated::SETUP;

        std::bitset<0> bitset0;
        bIterator = DeserializeSequence(&bitset0, false, bIterator);

        int slct;

        bIterator = DeserializeEnum(4, &slct, bIterator);
        pcdsc->soundingRsUlConfigDedicated.srsBandwidth = slct;

        bIterator = DeserializeEnum(4, &slct, bIterator);

        bIterator = DeserializeInteger(&slct, 0, 23, bIterator);

        bool duration;
        bIterator = DeserializeBoolean(&duration, bIterator);

        bIterator = DeserializeInteger(&slct, 0, 1023, bIterator);
        pcdsc->soundingRsUlConfigDedicated.srsConfigIndex = slct;

        bIterator = DeserializeInteger(&slct, 0, 1, bIterator);

        bIterator = DeserializeEnum(8, &slct, bIterator);
      }
    }
  }

  return bIterator;
}

void RrcAsn1Header::Print(std::ostream &os,
                          LteRrcSap::RadioResourceConfigDedicated
                              radioResourceConfigDedicated) const {
  os << "   srbToAddModList: " << std::endl;
  auto it = radioResourceConfigDedicated.srbToAddModList.begin();
  for (; it != radioResourceConfigDedicated.srbToAddModList.end(); it++) {
    os << "      srbIdentity: " << (int)it->srbIdentity << std::endl;
    os << "      logicalChannelConfig: " << std::endl;
    os << "         priority: " << (int)it->logicalChannelConfig.priority
       << std::endl;
    os << "         prioritizedBitRateKbps: "
       << (int)it->logicalChannelConfig.prioritizedBitRateKbps << std::endl;
    os << "         bucketSizeDurationMs: "
       << (int)it->logicalChannelConfig.bucketSizeDurationMs << std::endl;
    os << "         logicalChannelGroup: "
       << (int)it->logicalChannelConfig.logicalChannelGroup << std::endl;
  }
  os << std::endl;

  os << "   drbToAddModList: " << std::endl;
  auto it2 = radioResourceConfigDedicated.drbToAddModList.begin();
  for (; it2 != radioResourceConfigDedicated.drbToAddModList.end(); it2++) {
    os << "      epsBearerIdentity: " << (int)it2->epsBearerIdentity
       << std::endl;
    os << "      drbIdentity: " << (int)it2->drbIdentity << std::endl;
    os << "      rlcConfig: " << it2->rlcConfig.choice << std::endl;
    os << "      logicalChannelIdentity: " << (int)it2->logicalChannelIdentity
       << std::endl;
    os << "      logicalChannelConfig: " << std::endl;
    os << "         priority: " << (int)it2->logicalChannelConfig.priority
       << std::endl;
    os << "         prioritizedBitRateKbps: "
       << (int)it2->logicalChannelConfig.prioritizedBitRateKbps << std::endl;
    os << "         bucketSizeDurationMs: "
       << (int)it2->logicalChannelConfig.bucketSizeDurationMs << std::endl;
    os << "         logicalChannelGroup: "
       << (int)it2->logicalChannelConfig.logicalChannelGroup << std::endl;
  }
  os << std::endl;

  os << "   drbToReleaseList: ";
  auto it3 = radioResourceConfigDedicated.drbToReleaseList.begin();
  for (; it3 != radioResourceConfigDedicated.drbToReleaseList.end(); it3++) {
    os << (int)*it3 << ", ";
  }
  os << std::endl;

  os << "   havePhysicalConfigDedicated: "
     << radioResourceConfigDedicated.havePhysicalConfigDedicated << std::endl;

  if (radioResourceConfigDedicated.havePhysicalConfigDedicated) {
    os << "   physicalConfigDedicated: " << std::endl;

    os << "      haveSoundingRsUlConfigDedicated: "
       << radioResourceConfigDedicated.physicalConfigDedicated
              .haveSoundingRsUlConfigDedicated
       << std::endl;
    if (radioResourceConfigDedicated.physicalConfigDedicated
            .haveSoundingRsUlConfigDedicated) {
      os << "      soundingRsUlConfigDedicated: " << std::endl;
      os << "         type: "
         << radioResourceConfigDedicated.physicalConfigDedicated
                .soundingRsUlConfigDedicated.type
         << std::endl;
      os << "         srsBandwidth: "
         << (int)radioResourceConfigDedicated.physicalConfigDedicated
                .soundingRsUlConfigDedicated.srsBandwidth
         << std::endl;
      os << "         srsConfigIndex: "
         << (int)radioResourceConfigDedicated.physicalConfigDedicated
                .soundingRsUlConfigDedicated.srsConfigIndex
         << std::endl;
    }

    os << "      haveAntennaInfoDedicated: "
       << radioResourceConfigDedicated.physicalConfigDedicated
              .haveAntennaInfoDedicated
       << std::endl;
    if (radioResourceConfigDedicated.physicalConfigDedicated
            .haveAntennaInfoDedicated) {
      os << "      antennaInfo Tx mode: "
         << (int)radioResourceConfigDedicated.physicalConfigDedicated
                .antennaInfo.transmissionMode
         << std::endl;
    }
  }
}

Buffer::Iterator RrcAsn1Header::DeserializeSystemInformationBlockType1(
    LteRrcSap::SystemInformationBlockType1 *systemInformationBlockType1,
    Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  std::bitset<3> sysInfoBlkT1Opts;
  bIterator = DeserializeSequence(&sysInfoBlkT1Opts, false, bIterator);

  std::bitset<1> cellAccessRelatedInfoOpts;
  bIterator = DeserializeSequence(&cellAccessRelatedInfoOpts, false, bIterator);

  int numPlmnIdentityInfoElements;
  bIterator =
      DeserializeSequenceOf(&numPlmnIdentityInfoElements, 6, 1, bIterator);
  for (int i = 0; i < numPlmnIdentityInfoElements; i++) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);

    bIterator = DeserializePlmnIdentity(
        &systemInformationBlockType1->cellAccessRelatedInfo.plmnIdentityInfo
             .plmnIdentity,
        bIterator);
  }

  std::bitset<16> trackingAreaCode;
  bIterator = DeserializeBitstring(&trackingAreaCode, bIterator);

  std::bitset<28> cellIdentity;
  bIterator = DeserializeBitstring(&cellIdentity, bIterator);
  systemInformationBlockType1->cellAccessRelatedInfo.cellIdentity =
      cellIdentity.to_ulong();

  bIterator = DeserializeEnum(2, &n, bIterator);

  bIterator = DeserializeEnum(2, &n, bIterator);

  bIterator = DeserializeBoolean(
      &systemInformationBlockType1->cellAccessRelatedInfo.csgIndication,
      bIterator);

  if (cellAccessRelatedInfoOpts[0]) {
    std::bitset<27> csgIdentity;
    bIterator = DeserializeBitstring(&csgIdentity, bIterator);
    systemInformationBlockType1->cellAccessRelatedInfo.csgIdentity =
        csgIdentity.to_ulong();
  }

  std::bitset<1> qRxLevMinOffsetPresent;
  bIterator = DeserializeSequence(&qRxLevMinOffsetPresent, false, bIterator);
  bIterator = DeserializeInteger(&n, -70, -22, bIterator);
  if (qRxLevMinOffsetPresent[0]) {
  }

  if (sysInfoBlkT1Opts[2]) {
  }

  bIterator = DeserializeInteger(&n, 1, 64, bIterator);

  int numSchedulingInfo;
  bIterator =
      DeserializeSequenceOf(&numSchedulingInfo, MAX_SI_MESSAGE, 1, bIterator);
  for (int i = 0; i < numSchedulingInfo; i++) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
    bIterator = DeserializeEnum(7, &n, bIterator);
    int numSibType;
    bIterator = DeserializeSequenceOf(&numSibType, MAX_SIB - 1, 0, bIterator);
    for (int j = 0; j < numSibType; j++) {
      bIterator = DeserializeEnum(16, &n, bIterator);
    }
  }

  if (sysInfoBlkT1Opts[1]) {
  }

  bIterator = DeserializeEnum(7, &n, bIterator);

  bIterator = DeserializeInteger(&n, 0, 31, bIterator);

  if (sysInfoBlkT1Opts[0]) {
  }
  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializeSystemInformationBlockType2(
    LteRrcSap::SystemInformationBlockType2 *systemInformationBlockType2,
    Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  std::bitset<2> sysInfoBlkT2Opts;
  bIterator = DeserializeSequence(&sysInfoBlkT2Opts, true, bIterator);
  if (sysInfoBlkT2Opts[1]) {
  }

  bIterator = DeserializeRadioResourceConfigCommonSib(
      &systemInformationBlockType2->radioResourceConfigCommon, bIterator);

  bIterator = DeserializeSequence(&bitset0, true, bIterator);
  bIterator = DeserializeEnum(8, &n, bIterator);
  bIterator = DeserializeEnum(8, &n, bIterator);
  bIterator = DeserializeEnum(7, &n, bIterator);
  bIterator = DeserializeEnum(8, &n, bIterator);
  bIterator = DeserializeEnum(7, &n, bIterator);
  bIterator = DeserializeEnum(8, &n, bIterator);

  std::bitset<2> freqInfoOpts;
  bIterator = DeserializeSequence(&freqInfoOpts, false, bIterator);
  if (freqInfoOpts[1]) {
    bIterator = DeserializeInteger(&n, 0, MAX_EARFCN, bIterator);
    systemInformationBlockType2->freqInfo.ulCarrierFreq = n;
  }
  if (freqInfoOpts[0]) {
    bIterator = DeserializeEnum(6, &n, bIterator);
    systemInformationBlockType2->freqInfo.ulBandwidth = EnumToBandwidth(n);
  }

  bIterator = DeserializeInteger(&n, 1, 32, bIterator);

  if (sysInfoBlkT2Opts[0]) {
  }

  bIterator = DeserializeEnum(8, &n, bIterator);

  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializeRadioResourceConfigCommon(
    LteRrcSap::RadioResourceConfigCommon *radioResourceConfigCommon,
    Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  std::bitset<9> rrCfgCommOptions;
  bIterator = DeserializeSequence(&rrCfgCommOptions, true, bIterator);

  if (rrCfgCommOptions[8]) {
    bIterator = DeserializeRachConfigCommon(
        &radioResourceConfigCommon->rachConfigCommon, bIterator);
  }

  std::bitset<1> prachConfigInfoPresent;
  bIterator = DeserializeSequence(&prachConfigInfoPresent, false, bIterator);

  bIterator = DeserializeInteger(&n, 0, 1023, bIterator);

  if (prachConfigInfoPresent[0]) {
  }

  if (rrCfgCommOptions[7]) {
  }

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeInteger(&n, 1, 4, bIterator);

  bIterator = DeserializeEnum(2, &n, bIterator);

  bIterator = DeserializeInteger(&n, 0, 98, bIterator);

  bool enable64QAM;
  bIterator = DeserializeBoolean(&enable64QAM, bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bool dummyBool;
  bIterator = DeserializeBoolean(&dummyBool, bIterator);

  bIterator = DeserializeInteger(&n, 0, 29, bIterator);

  bIterator = DeserializeBoolean(&dummyBool, bIterator);

  bIterator = DeserializeInteger(&n, 0, 7, bIterator);

  if (rrCfgCommOptions[6]) {
  }

  if (rrCfgCommOptions[5]) {
  }

  if (rrCfgCommOptions[4]) {
  }

  if (rrCfgCommOptions[3]) {
  }

  if (rrCfgCommOptions[2]) {
  }

  if (rrCfgCommOptions[1]) {
  }

  if (rrCfgCommOptions[0]) {
  }

  bIterator = DeserializeEnum(2, &n, bIterator);

  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializeRachConfigCommon(
    LteRrcSap::RachConfigCommon *rachConfigCommon, Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeSequence(&bitset0, true, bIterator);

  std::bitset<1> preamblesGroupAConfigPresent;
  bIterator =
      DeserializeSequence(&preamblesGroupAConfigPresent, false, bIterator);

  bIterator = DeserializeEnum(16, &n, bIterator);
  switch (n) {
  case 0:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 4;
    break;
  case 1:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 8;
    break;
  case 2:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 12;
    break;
  case 3:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 16;
    break;
  case 4:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 20;
    break;
  case 5:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 24;
    break;
  case 6:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 28;
    break;
  case 7:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 32;
    break;
  case 8:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 36;
    break;
  case 9:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 40;
    break;
  case 10:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 44;
    break;
  case 11:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 48;
    break;
  case 12:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 52;
    break;
  case 13:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 56;
    break;
  case 14:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 60;
    break;
  case 15:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 64;
    break;
  default:
    rachConfigCommon->preambleInfo.numberOfRaPreambles = 4;
  }

  if (preamblesGroupAConfigPresent[0]) {
  }

  bIterator = DeserializeSequence(&bitset0, false, bIterator);
  bIterator = DeserializeEnum(4, &n, bIterator);
  bIterator = DeserializeEnum(16, &n, bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);
  bIterator = DeserializeEnum(11, &n, bIterator);
  switch (n) {
  case 0:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 3;
    break;
  case 1:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 4;
    break;
  case 2:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 5;
    break;
  case 3:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 6;
    break;
  case 4:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 7;
    break;
  case 5:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 8;
    break;
  case 6:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 10;
    break;
  case 7:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 20;
    break;
  case 8:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 50;
    break;
  case 9:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 100;
    break;
  case 10:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 200;
    break;
  default:
    rachConfigCommon->raSupervisionInfo.preambleTransMax = 0;
  }

  bIterator = DeserializeEnum(8, &n, bIterator);
  switch (n) {
  case 0:
    rachConfigCommon->raSupervisionInfo.raResponseWindowSize = 2;
    break;
  case 1:
    rachConfigCommon->raSupervisionInfo.raResponseWindowSize = 3;
    break;
  case 2:
    rachConfigCommon->raSupervisionInfo.raResponseWindowSize = 4;
    break;
  case 3:
    rachConfigCommon->raSupervisionInfo.raResponseWindowSize = 5;
    break;
  case 4:
    rachConfigCommon->raSupervisionInfo.raResponseWindowSize = 6;
    break;
  case 5:
    rachConfigCommon->raSupervisionInfo.raResponseWindowSize = 7;
    break;
  case 6:
    rachConfigCommon->raSupervisionInfo.raResponseWindowSize = 8;
    break;
  case 7:
    rachConfigCommon->raSupervisionInfo.raResponseWindowSize = 10;
    break;
  default:
    rachConfigCommon->raSupervisionInfo.raResponseWindowSize = 0;
  }

  bIterator = DeserializeEnum(8, &n, bIterator);
  bIterator = DeserializeInteger(&n, 1, 8, bIterator);

  bIterator = DeserializeEnum(8, &n, bIterator);
  switch (n) {
  case 1:
    rachConfigCommon->txFailParam.connEstFailCount = 1;
    break;
  case 2:
    rachConfigCommon->txFailParam.connEstFailCount = 2;
    break;
  case 3:
    rachConfigCommon->txFailParam.connEstFailCount = 3;
    break;
  case 4:
    rachConfigCommon->txFailParam.connEstFailCount = 4;
    break;
  default:
    rachConfigCommon->txFailParam.connEstFailCount = 1;
  }
  return bIterator;
}

Buffer::Iterator RrcAsn1Header::DeserializeRadioResourceConfigCommonSib(
    LteRrcSap::RadioResourceConfigCommonSib *radioResourceConfigCommonSib,
    Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeSequence(&bitset0, true, bIterator);

  bIterator = DeserializeRachConfigCommon(
      &radioResourceConfigCommonSib->rachConfigCommon, bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);
  bIterator = DeserializeEnum(4, &n, bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);
  bIterator = DeserializeEnum(4, &n, bIterator);
  bIterator = DeserializeEnum(8, &n, bIterator);

  std::bitset<1> prachConfigInfoPresent;
  bIterator = DeserializeSequence(&prachConfigInfoPresent, false, bIterator);
  bIterator = DeserializeInteger(&n, 0, 1023, bIterator);
  if (prachConfigInfoPresent[0]) {
  }

  bIterator = DeserializeSequence(&bitset0, false, bIterator);
  bIterator = DeserializeInteger(&n, -60, 50, bIterator);
  bIterator = DeserializeInteger(&n, 0, 3, bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeInteger(&n, 1, 4, bIterator);

  bIterator = DeserializeEnum(2, &n, bIterator);

  bIterator = DeserializeInteger(&n, 0, 98, bIterator);

  bool dummyBoolean;
  bIterator = DeserializeBoolean(&dummyBoolean, bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeBoolean(&dummyBoolean, bIterator);

  bIterator = DeserializeInteger(&n, 0, 29, bIterator);

  bIterator = DeserializeBoolean(&dummyBoolean, bIterator);

  bIterator = DeserializeInteger(&n, 0, 7, bIterator);

  bIterator = DeserializeEnum(3, &n, bIterator);
  bIterator = DeserializeInteger(&n, 0, 98, bIterator);
  bIterator = DeserializeInteger(&n, 0, 7, bIterator);
  bIterator = DeserializeInteger(&n, 0, 2047, bIterator);

  int choice;
  bIterator = DeserializeChoice(2, false, &choice, bIterator);
  if (choice == 0) {
    bIterator = DeserializeNull(bIterator);
  }
  if (choice == 1) {
  }

  bIterator = DeserializeSequence(&bitset0, false, bIterator);
  bIterator = DeserializeInteger(&n, -126, 24, bIterator);
  bIterator = DeserializeEnum(8, &n, bIterator);
  bIterator = DeserializeInteger(&n, -127, -96, bIterator);
  bIterator = DeserializeSequence(&bitset0, false, bIterator);
  bIterator = DeserializeEnum(3, &n, bIterator);
  bIterator = DeserializeEnum(3, &n, bIterator);
  bIterator = DeserializeEnum(4, &n, bIterator);
  bIterator = DeserializeEnum(3, &n, bIterator);
  bIterator = DeserializeEnum(3, &n, bIterator);
  bIterator = DeserializeInteger(&n, -1, 6, bIterator);

  bIterator = DeserializeEnum(2, &n, bIterator);

  return bIterator;
}

Buffer::Iterator
RrcAsn1Header::DeserializeMeasResults(LteRrcSap::MeasResults *measResults,
                                      Buffer::Iterator bIterator) {
  int n;
  std::bitset<0> b0;
  std::bitset<4> measResultOptionalPresent;
  bIterator = DeserializeSequence(&measResultOptionalPresent, true, bIterator);

  bIterator = DeserializeInteger(&n, 1, MAX_MEAS_ID, bIterator);
  measResults->measId = n;

  bIterator = DeserializeSequence(&b0, false, bIterator);

  bIterator = DeserializeInteger(&n, 0, 97, bIterator);
  measResults->measResultPCell.rsrpResult = n;

  bIterator = DeserializeInteger(&n, 0, 34, bIterator);
  measResults->measResultPCell.rsrqResult = n;

  measResults->haveMeasResultNeighCells = measResultOptionalPresent[0];
  measResults->haveMeasResultServFreqList = measResultOptionalPresent[3];
  if (measResults->haveMeasResultNeighCells) {
    int measResultNeighCellsChoice;

    bIterator =
        DeserializeChoice(4, false, &measResultNeighCellsChoice, bIterator);

    if (measResultNeighCellsChoice == 0) {
      int numElems;
      bIterator =
          DeserializeSequenceOf(&numElems, MAX_CELL_REPORT, 1, bIterator);

      for (int i = 0; i < numElems; i++) {
        LteRrcSap::MeasResultEutra measResultEutra;

        std::bitset<1> isCgiInfoPresent;
        bIterator = DeserializeSequence(&isCgiInfoPresent, false, bIterator);

        bIterator = DeserializeInteger(&n, 0, 503, bIterator);
        measResultEutra.physCellId = n;

        measResultEutra.haveCgiInfo = isCgiInfoPresent[0];
        if (isCgiInfoPresent[0]) {
          std::bitset<1> havePlmnIdentityList;
          bIterator =
              DeserializeSequence(&havePlmnIdentityList, false, bIterator);

          bIterator = DeserializeSequence(&b0, false, bIterator);

          bIterator = DeserializePlmnIdentity(
              &measResultEutra.cgiInfo.plmnIdentity, bIterator);

          std::bitset<28> cellId;
          bIterator = DeserializeBitstring(&cellId, bIterator);
          measResultEutra.cgiInfo.cellIdentity = cellId.to_ulong();

          std::bitset<16> trArCo;
          bIterator = DeserializeBitstring(&trArCo, bIterator);
          measResultEutra.cgiInfo.trackingAreaCode = trArCo.to_ulong();

          if (havePlmnIdentityList[0]) {
            int numPlmnElems;
            bIterator = DeserializeSequenceOf(&numPlmnElems, 5, 1, bIterator);

            for (int j = 0; j < numPlmnElems; j++) {
              uint32_t plmnId;
              bIterator = DeserializePlmnIdentity(&plmnId, bIterator);
              measResultEutra.cgiInfo.plmnIdentityList.push_back(plmnId);
            }
          }
        }

        std::bitset<2> measResultOpts;
        bIterator = DeserializeSequence(&measResultOpts, true, bIterator);

        measResultEutra.haveRsrpResult = measResultOpts[1];
        if (measResultOpts[1]) {
          bIterator = DeserializeInteger(&n, 0, 97, bIterator);
          measResultEutra.rsrpResult = n;
        }

        measResultEutra.haveRsrqResult = measResultOpts[0];
        if (measResultOpts[0]) {
          bIterator = DeserializeInteger(&n, 0, 34, bIterator);
          measResultEutra.rsrqResult = n;
        }

        measResults->measResultListEutra.push_back(measResultEutra);
      }
    }

    if (measResultNeighCellsChoice == 1) {
    }

    if (measResultNeighCellsChoice == 2) {
    }
    if (measResultNeighCellsChoice == 3) {
    }
  }
  if (measResults->haveMeasResultServFreqList) {
    int numElems;
    bIterator =
        DeserializeSequenceOf(&numElems, MAX_SCELL_REPORT, 1, bIterator);
    for (int i = 0; i < numElems; i++) {
      LteRrcSap::MeasResultServFreq measResultServFreq;

      std::bitset<2> measResultScellPresent;
      bIterator = DeserializeSequence(&measResultScellPresent, true, bIterator);
      measResultServFreq.haveMeasResultSCell = measResultScellPresent[0];
      measResultServFreq.haveMeasResultBestNeighCell =
          measResultScellPresent[1];

      int servFreqId;
      bIterator = DeserializeInteger(&servFreqId, 0, 7, bIterator);
      measResultServFreq.servFreqId = servFreqId;

      if (measResultServFreq.haveMeasResultSCell) {
        bIterator = DeserializeInteger(&n, 0, 97, bIterator);
        measResultServFreq.measResultSCell.rsrpResult = n;

        bIterator = DeserializeInteger(&n, 0, 34, bIterator);
        measResultServFreq.measResultSCell.rsrqResult = n;
      }

      if (measResultServFreq.haveMeasResultBestNeighCell) {
        bIterator = DeserializeInteger(&n, 0, 503, bIterator);
        measResultServFreq.measResultBestNeighCell.physCellId = n;

        bIterator = DeserializeInteger(&n, 0, 97, bIterator);
        measResultServFreq.measResultBestNeighCell.rsrpResult = n;

        bIterator = DeserializeInteger(&n, 0, 34, bIterator);
        measResultServFreq.measResultBestNeighCell.rsrqResult = n;
      }
      measResults->measResultServFreqList.push_back(measResultServFreq);
    }
  }
  return bIterator;
}

Buffer::Iterator
RrcAsn1Header::DeserializePlmnIdentity(uint32_t *plmnId,
                                       Buffer::Iterator bIterator) {
  int n;
  std::bitset<1> isMccPresent;
  bIterator = DeserializeSequence(&isMccPresent, false, bIterator);

  if (isMccPresent[0]) {
  }

  int mncDigits;
  int mnc = 0;
  bIterator = DeserializeSequenceOf(&mncDigits, 3, 2, bIterator);

  for (int j = mncDigits - 1; j >= 0; j--) {
    bIterator = DeserializeInteger(&n, 0, 9, bIterator);
    mnc += n * pow(10, j);
  }

  *plmnId = mnc;

  bIterator = DeserializeEnum(2, &n, bIterator);
  return bIterator;
}

Buffer::Iterator
RrcAsn1Header::DeserializeMeasConfig(LteRrcSap::MeasConfig *measConfig,
                                     Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  std::bitset<2> bitset2;
  std::bitset<11> bitset11;
  int n;

  bIterator = DeserializeSequence(&bitset11, true, bIterator);

  if (bitset11[10]) {
    int measObjectToRemoveListElems;
    bIterator = DeserializeSequenceOf(&measObjectToRemoveListElems,
                                      MAX_OBJECT_ID, 1, bIterator);

    for (int i = 0; i < measObjectToRemoveListElems; i++) {
      bIterator = DeserializeInteger(&n, 1, MAX_OBJECT_ID, bIterator);
      measConfig->measObjectToRemoveList.push_back(n);
    }
  }

  if (bitset11[9]) {
    int measObjectToAddModListElems;
    bIterator = DeserializeSequenceOf(&measObjectToAddModListElems,
                                      MAX_OBJECT_ID, 1, bIterator);

    for (int i = 0; i < measObjectToAddModListElems; i++) {
      LteRrcSap::MeasObjectToAddMod elem;

      bIterator = DeserializeSequence(&bitset0, false, bIterator);

      bIterator = DeserializeInteger(&n, 1, MAX_OBJECT_ID, bIterator);
      elem.measObjectId = n;

      int measObjectChoice;
      bIterator = DeserializeChoice(4, true, &measObjectChoice, bIterator);

      switch (measObjectChoice) {
      case 1:
        break;

      case 2:
        break;

      case 3:
        break;

      case 0:
      default:
        std::bitset<5> measObjectEutraOpts;
        bIterator = DeserializeSequence(&measObjectEutraOpts, true, bIterator);

        bIterator = DeserializeInteger(&n, 0, MAX_EARFCN, bIterator);
        elem.measObjectEutra.carrierFreq = n;

        bIterator = DeserializeEnum(6, &n, bIterator);
        elem.measObjectEutra.allowedMeasBandwidth = EnumToBandwidth(n);

        bIterator = DeserializeBoolean(
            &elem.measObjectEutra.presenceAntennaPort1, bIterator);

        bIterator = DeserializeBitstring(&bitset2, bIterator);
        elem.measObjectEutra.neighCellConfig = bitset2.to_ulong();

        bIterator = DeserializeQoffsetRange(&elem.measObjectEutra.offsetFreq,
                                            bIterator);

        if (measObjectEutraOpts[4]) {
          int numElems;
          bIterator =
              DeserializeSequenceOf(&numElems, MAX_CELL_MEAS, 1, bIterator);

          for (int i = 0; i < numElems; i++) {
            bIterator = DeserializeInteger(&n, 1, MAX_CELL_MEAS, bIterator);
            elem.measObjectEutra.cellsToRemoveList.push_back(n);
          }
        }

        if (measObjectEutraOpts[3]) {
          int numElems;
          bIterator =
              DeserializeSequenceOf(&numElems, MAX_CELL_MEAS, 1, bIterator);

          for (int i = 0; i < numElems; i++) {
            LteRrcSap::CellsToAddMod cellsToAddMod;

            bIterator = DeserializeSequence(&bitset0, false, bIterator);

            bIterator = DeserializeInteger(&n, 1, MAX_CELL_MEAS, bIterator);
            cellsToAddMod.cellIndex = n;

            bIterator = DeserializeInteger(&n, 0, 503, bIterator);
            cellsToAddMod.physCellId = n;

            bIterator = DeserializeQoffsetRange(
                &cellsToAddMod.cellIndividualOffset, bIterator);

            elem.measObjectEutra.cellsToAddModList.push_back(cellsToAddMod);
          }
        }

        if (measObjectEutraOpts[2]) {
          int numElems;
          bIterator =
              DeserializeSequenceOf(&numElems, MAX_CELL_MEAS, 1, bIterator);

          for (int i = 0; i < numElems; i++) {
            bIterator = DeserializeInteger(&n, 1, MAX_CELL_MEAS, bIterator);
            elem.measObjectEutra.blackCellsToRemoveList.push_back(n);
          }
        }

        if (measObjectEutraOpts[1]) {
          int numElems;
          bIterator =
              DeserializeSequenceOf(&numElems, MAX_CELL_MEAS, 1, bIterator);

          for (int i = 0; i < numElems; i++) {
            LteRrcSap::BlackCellsToAddMod blackCellsToAddMod;
            bIterator = DeserializeSequence(&bitset0, false, bIterator);

            bIterator = DeserializeInteger(&n, 1, MAX_CELL_MEAS, bIterator);
            blackCellsToAddMod.cellIndex = n;

            std::bitset<1> isRangePresent;
            bIterator = DeserializeSequence(&isRangePresent, false, bIterator);

            bIterator = DeserializeInteger(&n, 0, 503, bIterator);
            blackCellsToAddMod.physCellIdRange.start = n;

            blackCellsToAddMod.physCellIdRange.haveRange = isRangePresent[0];
            blackCellsToAddMod.physCellIdRange.range = 0;
            if (blackCellsToAddMod.physCellIdRange.haveRange) {
              bIterator = DeserializeEnum(16, &n, bIterator);
              switch (n) {
              case 0:
                blackCellsToAddMod.physCellIdRange.range = 4;
                break;
              case 1:
                blackCellsToAddMod.physCellIdRange.range = 8;
                break;
              case 2:
                blackCellsToAddMod.physCellIdRange.range = 12;
                break;
              case 3:
                blackCellsToAddMod.physCellIdRange.range = 16;
                break;
              case 4:
                blackCellsToAddMod.physCellIdRange.range = 24;
                break;
              case 5:
                blackCellsToAddMod.physCellIdRange.range = 32;
                break;
              case 6:
                blackCellsToAddMod.physCellIdRange.range = 48;
                break;
              case 7:
                blackCellsToAddMod.physCellIdRange.range = 64;
                break;
              case 8:
                blackCellsToAddMod.physCellIdRange.range = 84;
                break;
              case 9:
                blackCellsToAddMod.physCellIdRange.range = 96;
                break;
              case 10:
                blackCellsToAddMod.physCellIdRange.range = 128;
                break;
              case 11:
                blackCellsToAddMod.physCellIdRange.range = 168;
                break;
              case 12:
                blackCellsToAddMod.physCellIdRange.range = 252;
                break;
              case 13:
                blackCellsToAddMod.physCellIdRange.range = 504;
                break;
              default:
                blackCellsToAddMod.physCellIdRange.range = 0;
              }
            }

            elem.measObjectEutra.blackCellsToAddModList.push_back(
                blackCellsToAddMod);
          }
        }

        elem.measObjectEutra.haveCellForWhichToReportCGI =
            measObjectEutraOpts[0];
        if (measObjectEutraOpts[0]) {
          bIterator = DeserializeInteger(&n, 0, 503, bIterator);
          elem.measObjectEutra.cellForWhichToReportCGI = n;
        }
      }
      measConfig->measObjectToAddModList.push_back(elem);
    }
  }

  if (bitset11[8]) {
    int reportConfigToRemoveListElems;
    bIterator = DeserializeSequenceOf(&reportConfigToRemoveListElems,
                                      MAX_REPORT_CONFIG_ID, 1, bIterator);

    for (int i = 0; i < reportConfigToRemoveListElems; i++) {
      bIterator = DeserializeInteger(&n, 1, MAX_REPORT_CONFIG_ID, bIterator);
      measConfig->reportConfigToRemoveList.push_back(n);
    }
  }

  if (bitset11[7]) {
    int reportConfigToAddModListElems;
    bIterator = DeserializeSequenceOf(&reportConfigToAddModListElems,
                                      MAX_REPORT_CONFIG_ID, 1, bIterator);

    for (int i = 0; i < reportConfigToAddModListElems; i++) {
      LteRrcSap::ReportConfigToAddMod elem;

      bIterator = DeserializeSequence(&bitset0, false, bIterator);
      bIterator = DeserializeInteger(&n, 1, MAX_REPORT_CONFIG_ID, bIterator);
      elem.reportConfigId = n;

      int reportConfigChoice;
      bIterator = DeserializeChoice(2, false, &reportConfigChoice, bIterator);

      if (reportConfigChoice == 0) {
        bIterator = DeserializeSequence(&bitset0, true, bIterator);

        int triggerTypeChoice;
        bIterator = DeserializeChoice(2, false, &triggerTypeChoice, bIterator);

        if (triggerTypeChoice == 0) {
          elem.reportConfigEutra.triggerType =
              LteRrcSap::ReportConfigEutra::EVENT;
          bIterator = DeserializeSequence(&bitset0, false, bIterator);

          int eventIdChoice;
          bIterator = DeserializeChoice(5, true, &eventIdChoice, bIterator);

          switch (eventIdChoice) {
          case 0:
            elem.reportConfigEutra.eventId =
                LteRrcSap::ReportConfigEutra::EVENT_A1;
            bIterator = DeserializeSequence(&bitset0, false, bIterator);
            bIterator = DeserializeThresholdEutra(
                &elem.reportConfigEutra.threshold1, bIterator);
            break;

          case 1:
            elem.reportConfigEutra.eventId =
                LteRrcSap::ReportConfigEutra::EVENT_A2;
            bIterator = DeserializeSequence(&bitset0, false, bIterator);
            bIterator = DeserializeThresholdEutra(
                &elem.reportConfigEutra.threshold1, bIterator);
            break;

          case 2:
            elem.reportConfigEutra.eventId =
                LteRrcSap::ReportConfigEutra::EVENT_A3;
            bIterator = DeserializeSequence(&bitset0, false, bIterator);
            bIterator = DeserializeInteger(&n, -30, 30, bIterator);
            elem.reportConfigEutra.a3Offset = n;
            bIterator = DeserializeBoolean(
                &elem.reportConfigEutra.reportOnLeave, bIterator);
            break;

          case 3:
            elem.reportConfigEutra.eventId =
                LteRrcSap::ReportConfigEutra::EVENT_A4;
            bIterator = DeserializeSequence(&bitset0, false, bIterator);
            bIterator = DeserializeThresholdEutra(
                &elem.reportConfigEutra.threshold1, bIterator);
            break;

          case 4:
          default:
            elem.reportConfigEutra.eventId =
                LteRrcSap::ReportConfigEutra::EVENT_A5;
            bIterator = DeserializeSequence(&bitset0, false, bIterator);
            bIterator = DeserializeThresholdEutra(
                &elem.reportConfigEutra.threshold1, bIterator);
            bIterator = DeserializeThresholdEutra(
                &elem.reportConfigEutra.threshold2, bIterator);
          }

          bIterator = DeserializeInteger(&n, 0, 30, bIterator);
          elem.reportConfigEutra.hysteresis = n;

          bIterator = DeserializeEnum(16, &n, bIterator);
          switch (n) {
          case 0:
            elem.reportConfigEutra.timeToTrigger = 0;
            break;
          case 1:
            elem.reportConfigEutra.timeToTrigger = 40;
            break;
          case 2:
            elem.reportConfigEutra.timeToTrigger = 64;
            break;
          case 3:
            elem.reportConfigEutra.timeToTrigger = 80;
            break;
          case 4:
            elem.reportConfigEutra.timeToTrigger = 100;
            break;
          case 5:
            elem.reportConfigEutra.timeToTrigger = 128;
            break;
          case 6:
            elem.reportConfigEutra.timeToTrigger = 160;
            break;
          case 7:
            elem.reportConfigEutra.timeToTrigger = 256;
            break;
          case 8:
            elem.reportConfigEutra.timeToTrigger = 320;
            break;
          case 9:
            elem.reportConfigEutra.timeToTrigger = 480;
            break;
          case 10:
            elem.reportConfigEutra.timeToTrigger = 512;
            break;
          case 11:
            elem.reportConfigEutra.timeToTrigger = 640;
            break;
          case 12:
            elem.reportConfigEutra.timeToTrigger = 1024;
            break;
          case 13:
            elem.reportConfigEutra.timeToTrigger = 1280;
            break;
          case 14:
            elem.reportConfigEutra.timeToTrigger = 2560;
            break;
          case 15:
          default:
            elem.reportConfigEutra.timeToTrigger = 5120;
            break;
          }
        }

        if (triggerTypeChoice == 1) {
          elem.reportConfigEutra.triggerType =
              LteRrcSap::ReportConfigEutra::PERIODICAL;

          bIterator = DeserializeSequence(&bitset0, false, bIterator);
          bIterator = DeserializeEnum(2, &n, bIterator);
          if (n == 0) {
            elem.reportConfigEutra.purpose =
                LteRrcSap::ReportConfigEutra::REPORT_STRONGEST_CELLS;
          } else {
            elem.reportConfigEutra.purpose =
                LteRrcSap::ReportConfigEutra::REPORT_CGI;
          }
        }

        bIterator = DeserializeEnum(2, &n, bIterator);
        if (n == 0) {
          elem.reportConfigEutra.triggerQuantity =
              LteRrcSap::ReportConfigEutra::RSRP;
        } else {
          elem.reportConfigEutra.triggerQuantity =
              LteRrcSap::ReportConfigEutra::RSRQ;
        }

        bIterator = DeserializeEnum(2, &n, bIterator);
        if (n == 0) {
          elem.reportConfigEutra.reportQuantity =
              LteRrcSap::ReportConfigEutra::SAME_AS_TRIGGER_QUANTITY;
        } else {
          elem.reportConfigEutra.reportQuantity =
              LteRrcSap::ReportConfigEutra::BOTH;
        }

        bIterator = DeserializeInteger(&n, 1, MAX_CELL_REPORT, bIterator);
        elem.reportConfigEutra.maxReportCells = n;

        bIterator = DeserializeEnum(16, &n, bIterator);
        switch (n) {
        case 0:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MS120;
          break;
        case 1:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MS240;
          break;
        case 2:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MS480;
          break;
        case 3:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MS640;
          break;
        case 4:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MS1024;
          break;
        case 5:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MS2048;
          break;
        case 6:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MS5120;
          break;
        case 7:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MS10240;
          break;
        case 8:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MIN1;
          break;
        case 9:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MIN6;
          break;
        case 10:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MIN12;
          break;
        case 11:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MIN30;
          break;
        case 12:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::MIN60;
          break;
        case 13:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::SPARE3;
          break;
        case 14:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::SPARE2;
          break;
        case 15:
        default:
          elem.reportConfigEutra.reportInterval =
              LteRrcSap::ReportConfigEutra::SPARE1;
        }

        bIterator = DeserializeEnum(8, &n, bIterator);
        switch (n) {
        case 0:
          elem.reportConfigEutra.reportAmount = 1;
          break;
        case 1:
          elem.reportConfigEutra.reportAmount = 2;
          break;
        case 2:
          elem.reportConfigEutra.reportAmount = 4;
          break;
        case 3:
          elem.reportConfigEutra.reportAmount = 8;
          break;
        case 4:
          elem.reportConfigEutra.reportAmount = 16;
          break;
        case 5:
          elem.reportConfigEutra.reportAmount = 32;
          break;
        case 6:
          elem.reportConfigEutra.reportAmount = 64;
          break;
        default:
          elem.reportConfigEutra.reportAmount = 0;
        }
      }

      if (reportConfigChoice == 1) {
      }

      measConfig->reportConfigToAddModList.push_back(elem);
    }
  }

  if (bitset11[6]) {
    int measIdToRemoveListElems;
    bIterator = DeserializeSequenceOf(&measIdToRemoveListElems, MAX_MEAS_ID, 1,
                                      bIterator);

    for (int i = 0; i < measIdToRemoveListElems; i++) {
      bIterator = DeserializeInteger(&n, 1, MAX_MEAS_ID, bIterator);
      measConfig->measIdToRemoveList.push_back(n);
    }
  }

  if (bitset11[5]) {
    int measIdToAddModListElems;
    bIterator = DeserializeSequenceOf(&measIdToAddModListElems, MAX_MEAS_ID, 1,
                                      bIterator);

    for (int i = 0; i < measIdToAddModListElems; i++) {
      LteRrcSap::MeasIdToAddMod elem;

      bIterator = DeserializeSequence(&bitset0, false, bIterator);

      bIterator = DeserializeInteger(&n, 1, MAX_MEAS_ID, bIterator);
      elem.measId = n;

      bIterator = DeserializeInteger(&n, 1, MAX_OBJECT_ID, bIterator);
      elem.measObjectId = n;

      bIterator = DeserializeInteger(&n, 1, MAX_REPORT_CONFIG_ID, bIterator);
      elem.reportConfigId = n;

      measConfig->measIdToAddModList.push_back(elem);
    }
  }

  measConfig->haveQuantityConfig = bitset11[4];
  if (measConfig->haveQuantityConfig) {
    std::bitset<4> quantityConfigOpts;
    bIterator = DeserializeSequence(&quantityConfigOpts, true, bIterator);

    if (quantityConfigOpts[3]) {
      bIterator = DeserializeSequence(&bitset0, false, bIterator);
      bIterator = DeserializeEnum(16, &n, bIterator);
      switch (n) {
      case 0:
        measConfig->quantityConfig.filterCoefficientRSRP = 0;
        break;
      case 1:
        measConfig->quantityConfig.filterCoefficientRSRP = 1;
        break;
      case 2:
        measConfig->quantityConfig.filterCoefficientRSRP = 2;
        break;
      case 3:
        measConfig->quantityConfig.filterCoefficientRSRP = 3;
        break;
      case 4:
        measConfig->quantityConfig.filterCoefficientRSRP = 4;
        break;
      case 5:
        measConfig->quantityConfig.filterCoefficientRSRP = 5;
        break;
      case 6:
        measConfig->quantityConfig.filterCoefficientRSRP = 6;
        break;
      case 7:
        measConfig->quantityConfig.filterCoefficientRSRP = 7;
        break;
      case 8:
        measConfig->quantityConfig.filterCoefficientRSRP = 8;
        break;
      case 9:
        measConfig->quantityConfig.filterCoefficientRSRP = 9;
        break;
      case 10:
        measConfig->quantityConfig.filterCoefficientRSRP = 11;
        break;
      case 11:
        measConfig->quantityConfig.filterCoefficientRSRP = 13;
        break;
      case 12:
        measConfig->quantityConfig.filterCoefficientRSRP = 15;
        break;
      case 13:
        measConfig->quantityConfig.filterCoefficientRSRP = 17;
        break;
      case 14:
        measConfig->quantityConfig.filterCoefficientRSRP = 19;
        break;
      case 15:
        measConfig->quantityConfig.filterCoefficientRSRP = 0;
        break;
      default:
        measConfig->quantityConfig.filterCoefficientRSRP = 4;
      }
      bIterator = DeserializeEnum(16, &n, bIterator);
      switch (n) {
      case 0:
        measConfig->quantityConfig.filterCoefficientRSRQ = 0;
        break;
      case 1:
        measConfig->quantityConfig.filterCoefficientRSRQ = 1;
        break;
      case 2:
        measConfig->quantityConfig.filterCoefficientRSRQ = 2;
        break;
      case 3:
        measConfig->quantityConfig.filterCoefficientRSRQ = 3;
        break;
      case 4:
        measConfig->quantityConfig.filterCoefficientRSRQ = 4;
        break;
      case 5:
        measConfig->quantityConfig.filterCoefficientRSRQ = 5;
        break;
      case 6:
        measConfig->quantityConfig.filterCoefficientRSRQ = 6;
        break;
      case 7:
        measConfig->quantityConfig.filterCoefficientRSRQ = 7;
        break;
      case 8:
        measConfig->quantityConfig.filterCoefficientRSRQ = 8;
        break;
      case 9:
        measConfig->quantityConfig.filterCoefficientRSRQ = 9;
        break;
      case 10:
        measConfig->quantityConfig.filterCoefficientRSRQ = 11;
        break;
      case 11:
        measConfig->quantityConfig.filterCoefficientRSRQ = 13;
        break;
      case 12:
        measConfig->quantityConfig.filterCoefficientRSRQ = 15;
        break;
      case 13:
        measConfig->quantityConfig.filterCoefficientRSRQ = 17;
        break;
      case 14:
        measConfig->quantityConfig.filterCoefficientRSRQ = 19;
        break;
      case 15:
        measConfig->quantityConfig.filterCoefficientRSRQ = 0;
        break;
      default:
        measConfig->quantityConfig.filterCoefficientRSRQ = 4;
      }
    }
    if (quantityConfigOpts[2]) {
    }
    if (quantityConfigOpts[1]) {
    }
    if (quantityConfigOpts[0]) {
    }
  }

  measConfig->haveMeasGapConfig = bitset11[3];
  if (measConfig->haveMeasGapConfig) {
    int measGapConfigChoice;
    bIterator = DeserializeChoice(2, false, &measGapConfigChoice, bIterator);
    switch (measGapConfigChoice) {
    case 0:
      measConfig->measGapConfig.type = LteRrcSap::MeasGapConfig::RESET;
      bIterator = DeserializeNull(bIterator);
      break;
    case 1:
    default:
      measConfig->measGapConfig.type = LteRrcSap::MeasGapConfig::SETUP;
      bIterator = DeserializeSequence(&bitset0, false, bIterator);

      int gapOffsetChoice;
      bIterator = DeserializeChoice(2, true, &gapOffsetChoice, bIterator);
      switch (gapOffsetChoice) {
      case 0:
        measConfig->measGapConfig.gapOffsetChoice =
            LteRrcSap::MeasGapConfig::GP0;
        bIterator = DeserializeInteger(&n, 0, 39, bIterator);
        measConfig->measGapConfig.gapOffsetValue = n;
        break;
      case 1:
      default:
        measConfig->measGapConfig.gapOffsetChoice =
            LteRrcSap::MeasGapConfig::GP1;
        bIterator = DeserializeInteger(&n, 0, 79, bIterator);
        measConfig->measGapConfig.gapOffsetValue = n;
      }
    }
  }

  measConfig->haveSmeasure = bitset11[2];
  if (measConfig->haveSmeasure) {
    bIterator = DeserializeInteger(&n, 0, 97, bIterator);
    measConfig->sMeasure = n;
  }

  if (bitset11[1]) {
  }

  measConfig->haveSpeedStatePars = bitset11[0];
  if (measConfig->haveSpeedStatePars) {
    int speedStateParsChoice;
    bIterator = DeserializeChoice(2, false, &speedStateParsChoice, bIterator);
    switch (speedStateParsChoice) {
    case 0:
      measConfig->speedStatePars.type = LteRrcSap::SpeedStatePars::RESET;
      bIterator = DeserializeNull(bIterator);
      break;
    case 1:
    default:
      measConfig->speedStatePars.type = LteRrcSap::SpeedStatePars::SETUP;
      bIterator = DeserializeSequence(&bitset0, false, bIterator);

      bIterator = DeserializeEnum(8, &n, bIterator);
      switch (n) {
      case 0:
        measConfig->speedStatePars.mobilityStateParameters.tEvaluation = 30;
        break;
      case 1:
        measConfig->speedStatePars.mobilityStateParameters.tEvaluation = 60;
        break;
      case 2:
        measConfig->speedStatePars.mobilityStateParameters.tEvaluation = 120;
        break;
      case 3:
        measConfig->speedStatePars.mobilityStateParameters.tEvaluation = 180;
        break;
      case 4:
        measConfig->speedStatePars.mobilityStateParameters.tEvaluation = 240;
        break;
      default:
        measConfig->speedStatePars.mobilityStateParameters.tEvaluation = 0;
      }
      bIterator = DeserializeEnum(8, &n, bIterator);
      switch (n) {
      case 0:
        measConfig->speedStatePars.mobilityStateParameters.tHystNormal = 30;
        break;
      case 1:
        measConfig->speedStatePars.mobilityStateParameters.tHystNormal = 60;
        break;
      case 2:
        measConfig->speedStatePars.mobilityStateParameters.tHystNormal = 120;
        break;
      case 3:
        measConfig->speedStatePars.mobilityStateParameters.tHystNormal = 180;
        break;
      case 4:
        measConfig->speedStatePars.mobilityStateParameters.tHystNormal = 240;
        break;
      default:
        measConfig->speedStatePars.mobilityStateParameters.tHystNormal = 0;
      }

      bIterator = DeserializeInteger(&n, 1, 16, bIterator);
      measConfig->speedStatePars.mobilityStateParameters.nCellChangeMedium = n;

      bIterator = DeserializeInteger(&n, 1, 16, bIterator);
      measConfig->speedStatePars.mobilityStateParameters.nCellChangeHigh = n;

      bIterator = DeserializeEnum(4, &n, bIterator);
      measConfig->speedStatePars.timeToTriggerSf.sfMedium = (n + 1) * 25;
      bIterator = DeserializeEnum(4, &n, bIterator);
      measConfig->speedStatePars.timeToTriggerSf.sfHigh = (n + 1) * 25;
    }
  }
  return bIterator;
}

RrcConnectionRequestHeader::RrcConnectionRequestHeader() : RrcUlCcchMessage() {
  m_mmec = std::bitset<8>(0UL);
  m_mTmsi = std::bitset<32>(0UL);
  m_establishmentCause = MO_SIGNALLING;
  m_spare = std::bitset<1>(0UL);
}

RrcConnectionRequestHeader::~RrcConnectionRequestHeader() {}

TypeId RrcConnectionRequestHeader::GetTypeId() {
  static TypeId tid = TypeId("ns3::RrcConnectionRequestHeader")
                          .SetParent<Header>()
                          .SetGroupName("Lte");
  return tid;
}

void RrcConnectionRequestHeader::Print(std::ostream &os) const {
  os << "MMEC:" << m_mmec << std::endl;
  os << "MTMSI:" << m_mTmsi << std::endl;
  os << "EstablishmentCause:" << m_establishmentCause << std::endl;
  os << "Spare: " << m_spare << std::endl;
}

void RrcConnectionRequestHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeUlCcchMessage(1);

  SerializeSequence(std::bitset<0>(), false);

  SerializeChoice(2, 0, false);

  SerializeSequence(std::bitset<0>(), false);

  SerializeChoice(2, 0, false);

  SerializeSequence(std::bitset<0>(), false);

  SerializeBitstring(m_mmec);

  SerializeBitstring(m_mTmsi);

  SerializeEnum(8, m_establishmentCause);

  SerializeBitstring(std::bitset<1>());

  FinalizeSerialization();
}

uint32_t RrcConnectionRequestHeader::Deserialize(Buffer::Iterator bIterator) {
  std::bitset<1> dummy;
  std::bitset<0> optionalOrDefaultMask;
  int selectedOption;

  bIterator = DeserializeUlCcchMessage(bIterator);

  bIterator = DeserializeSequence(&optionalOrDefaultMask, false, bIterator);

  bIterator = DeserializeChoice(2, false, &selectedOption, bIterator);

  bIterator = DeserializeSequence(&optionalOrDefaultMask, false, bIterator);

  bIterator = DeserializeChoice(2, false, &selectedOption, bIterator);

  bIterator = DeserializeSequence(&optionalOrDefaultMask, false, bIterator);

  bIterator = DeserializeBitstring(&m_mmec, bIterator);

  bIterator = DeserializeBitstring(&m_mTmsi, bIterator);

  bIterator = DeserializeEnum(8, &selectedOption, bIterator);

  bIterator = DeserializeBitstring(&dummy, bIterator);

  return GetSerializedSize();
}

void RrcConnectionRequestHeader::SetMessage(
    LteRrcSap::RrcConnectionRequest msg) {
  m_mTmsi = std::bitset<32>((uint32_t)msg.ueIdentity);
  m_mmec = std::bitset<8>((uint32_t)(msg.ueIdentity >> 32));
  m_isDataSerialized = false;
}

LteRrcSap::RrcConnectionRequest RrcConnectionRequestHeader::GetMessage() const {
  LteRrcSap::RrcConnectionRequest msg;
  msg.ueIdentity = (((uint64_t)m_mmec.to_ulong()) << 32) | (m_mTmsi.to_ulong());

  return msg;
}

std::bitset<8> RrcConnectionRequestHeader::GetMmec() const { return m_mmec; }

std::bitset<32> RrcConnectionRequestHeader::GetMtmsi() const { return m_mTmsi; }

RrcConnectionSetupHeader::RrcConnectionSetupHeader() {}

RrcConnectionSetupHeader::~RrcConnectionSetupHeader() {}

void RrcConnectionSetupHeader::Print(std::ostream &os) const {
  os << "rrcTransactionIdentifier: " << (int)m_rrcTransactionIdentifier
     << std::endl;
  os << "radioResourceConfigDedicated:" << std::endl;
  RrcAsn1Header::Print(os, m_radioResourceConfigDedicated);
}

void RrcConnectionSetupHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeDlCcchMessage(3);

  SerializeInteger(15, 0, 15);

  SerializeSequence(std::bitset<0>(), false);

  SerializeInteger(m_rrcTransactionIdentifier, 0, 3);

  SerializeChoice(2, 0, false);

  SerializeChoice(8, 0, false);

  SerializeSequence(std::bitset<1>(0), false);

  SerializeRadioResourceConfigDedicated(m_radioResourceConfigDedicated);

  SerializeSequence(std::bitset<2>(0), false);

  FinalizeSerialization();
}

uint32_t RrcConnectionSetupHeader::Deserialize(Buffer::Iterator bIterator) {
  int n;

  std::bitset<0> bitset0;
  std::bitset<1> bitset1;
  std::bitset<2> bitset2;

  bIterator = DeserializeDlCcchMessage(bIterator);

  bIterator = DeserializeInteger(&n, 0, 15, bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeInteger(&n, 0, 3, bIterator);
  m_rrcTransactionIdentifier = n;

  int criticalExtensionChoice;
  bIterator = DeserializeChoice(2, false, &criticalExtensionChoice, bIterator);
  if (criticalExtensionChoice == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (criticalExtensionChoice == 0) {
    int c1;
    bIterator = DeserializeChoice(8, false, &c1, bIterator);

    if (c1 > 0) {
      bIterator = DeserializeNull(bIterator);
    } else if (c1 == 0) {
      bIterator = DeserializeSequence(&bitset1, false, bIterator);

      bIterator = DeserializeRadioResourceConfigDedicated(
          &m_radioResourceConfigDedicated, bIterator);

      if (bitset1[0]) {
        bIterator = DeserializeSequence(&bitset2, false, bIterator);
      }
    }
  }
  return GetSerializedSize();
}

void RrcConnectionSetupHeader::SetMessage(LteRrcSap::RrcConnectionSetup msg) {
  m_rrcTransactionIdentifier = msg.rrcTransactionIdentifier;
  m_radioResourceConfigDedicated = msg.radioResourceConfigDedicated;
  m_isDataSerialized = false;
}

LteRrcSap::RrcConnectionSetup RrcConnectionSetupHeader::GetMessage() const {
  LteRrcSap::RrcConnectionSetup msg;
  msg.rrcTransactionIdentifier = m_rrcTransactionIdentifier;
  msg.radioResourceConfigDedicated = m_radioResourceConfigDedicated;
  return msg;
}

uint8_t RrcConnectionSetupHeader::GetRrcTransactionIdentifier() const {
  return m_rrcTransactionIdentifier;
}

bool RrcConnectionSetupHeader::HavePhysicalConfigDedicated() const {
  return m_radioResourceConfigDedicated.havePhysicalConfigDedicated;
}

std::list<LteRrcSap::SrbToAddMod>
RrcConnectionSetupHeader::GetSrbToAddModList() const {
  return m_radioResourceConfigDedicated.srbToAddModList;
}

std::list<LteRrcSap::DrbToAddMod>
RrcConnectionSetupHeader::GetDrbToAddModList() const {
  return m_radioResourceConfigDedicated.drbToAddModList;
}

std::list<uint8_t> RrcConnectionSetupHeader::GetDrbToReleaseList() const {
  return m_radioResourceConfigDedicated.drbToReleaseList;
}

LteRrcSap::PhysicalConfigDedicated
RrcConnectionSetupHeader::GetPhysicalConfigDedicated() const {
  return m_radioResourceConfigDedicated.physicalConfigDedicated;
}

LteRrcSap::RadioResourceConfigDedicated
RrcConnectionSetupHeader::GetRadioResourceConfigDedicated() const {
  return m_radioResourceConfigDedicated;
}

RrcConnectionSetupCompleteHeader::RrcConnectionSetupCompleteHeader() {}

RrcConnectionSetupCompleteHeader::~RrcConnectionSetupCompleteHeader() {}

void RrcConnectionSetupCompleteHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeUlDcchMessage(4);

  SerializeSequence(std::bitset<0>(), false);

  SerializeInteger(m_rrcTransactionIdentifier, 0, 3);

  SerializeChoice(2, 0, false);

  SerializeChoice(4, 1, false);

  SerializeNull();

  FinalizeSerialization();
}

uint32_t
RrcConnectionSetupCompleteHeader::Deserialize(Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;

  bIterator = DeserializeUlDcchMessage(bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  int n;
  bIterator = DeserializeInteger(&n, 0, 3, bIterator);
  m_rrcTransactionIdentifier = n;

  bIterator = DeserializeChoice(2, false, &n, bIterator);

  if (n == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (n == 0) {
    int c1Chosen;
    bIterator = DeserializeChoice(4, false, &c1Chosen, bIterator);

    if (c1Chosen == 0) {
    } else {
      bIterator = DeserializeNull(bIterator);
    }
  }

  return GetSerializedSize();
}

void RrcConnectionSetupCompleteHeader::Print(std::ostream &os) const {
  os << "rrcTransactionIdentifier: " << (int)m_rrcTransactionIdentifier
     << std::endl;
}

void RrcConnectionSetupCompleteHeader::SetMessage(
    LteRrcSap::RrcConnectionSetupCompleted msg) {
  m_rrcTransactionIdentifier = msg.rrcTransactionIdentifier;
  m_isDataSerialized = false;
}

uint8_t RrcConnectionSetupCompleteHeader::GetRrcTransactionIdentifier() const {
  return m_rrcTransactionIdentifier;
}

LteRrcSap::RrcConnectionSetupCompleted
RrcConnectionSetupCompleteHeader::GetMessage() const {
  LteRrcSap::RrcConnectionSetupCompleted msg;
  msg.rrcTransactionIdentifier = m_rrcTransactionIdentifier;
  return msg;
}

RrcConnectionReconfigurationCompleteHeader::
    RrcConnectionReconfigurationCompleteHeader() {}

RrcConnectionReconfigurationCompleteHeader::
    ~RrcConnectionReconfigurationCompleteHeader() {}

void RrcConnectionReconfigurationCompleteHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeUlDcchMessage(2);

  SerializeSequence(std::bitset<0>(), false);

  SerializeInteger(m_rrcTransactionIdentifier, 0, 3);

  SerializeChoice(2, 1, false);

  SerializeSequence(std::bitset<0>(), false);

  FinalizeSerialization();
}

uint32_t RrcConnectionReconfigurationCompleteHeader::Deserialize(
    Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeUlDcchMessage(bIterator);
  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeInteger(&n, 0, 3, bIterator);
  m_rrcTransactionIdentifier = n;

  bIterator = DeserializeChoice(2, false, &n, bIterator);

  if (n == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (n == 0) {
  }

  return GetSerializedSize();
}

void RrcConnectionReconfigurationCompleteHeader::Print(std::ostream &os) const {
  os << "rrcTransactionIdentifier: " << (int)m_rrcTransactionIdentifier
     << std::endl;
}

void RrcConnectionReconfigurationCompleteHeader::SetMessage(
    LteRrcSap::RrcConnectionReconfigurationCompleted msg) {
  m_rrcTransactionIdentifier = msg.rrcTransactionIdentifier;
  m_isDataSerialized = false;
}

LteRrcSap::RrcConnectionReconfigurationCompleted
RrcConnectionReconfigurationCompleteHeader::GetMessage() const {
  LteRrcSap::RrcConnectionReconfigurationCompleted msg;
  msg.rrcTransactionIdentifier = m_rrcTransactionIdentifier;
  return msg;
}

uint8_t
RrcConnectionReconfigurationCompleteHeader::GetRrcTransactionIdentifier()
    const {
  return m_rrcTransactionIdentifier;
}

RrcConnectionReconfigurationHeader::RrcConnectionReconfigurationHeader() {}

RrcConnectionReconfigurationHeader::~RrcConnectionReconfigurationHeader() {}

void RrcConnectionReconfigurationHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeDlDcchMessage(4);

  SerializeSequence(std::bitset<0>(), false);

  SerializeInteger(m_rrcTransactionIdentifier, 0, 3);

  SerializeChoice(2, 0, false);

  SerializeChoice(8, 0, false);

  std::bitset<6> options;
  options.set(5, m_haveMeasConfig);
  options.set(4, m_haveMobilityControlInfo);
  options.set(3, false);
  options.set(2, m_haveRadioResourceConfigDedicated);
  options.set(1, false);
  options.set(0, m_haveNonCriticalExtension);
  SerializeSequence(options, false);

  if (m_haveMeasConfig) {
    SerializeMeasConfig(m_measConfig);
  }

  if (m_haveMobilityControlInfo) {

    std::bitset<4> mobCtrlIntoOptional;
    mobCtrlIntoOptional.set(3, m_mobilityControlInfo.haveCarrierFreq);
    mobCtrlIntoOptional.set(2, m_mobilityControlInfo.haveCarrierBandwidth);
    mobCtrlIntoOptional.set(1, false);
    mobCtrlIntoOptional.set(0, m_mobilityControlInfo.haveRachConfigDedicated);
    SerializeSequence(mobCtrlIntoOptional, true);

    SerializeInteger(m_mobilityControlInfo.targetPhysCellId, 0, 503);

    if (m_mobilityControlInfo.haveCarrierFreq) {
      SerializeSequence(std::bitset<1>(1), false);
      SerializeInteger(m_mobilityControlInfo.carrierFreq.dlCarrierFreq, 0,
                       MAX_EARFCN);
      SerializeInteger(m_mobilityControlInfo.carrierFreq.ulCarrierFreq, 0,
                       MAX_EARFCN);
    }

    if (m_mobilityControlInfo.haveCarrierBandwidth) {
      SerializeSequence(std::bitset<1>(1), false);

      SerializeEnum(
          16,
          BandwidthToEnum(m_mobilityControlInfo.carrierBandwidth.dlBandwidth));

      SerializeEnum(
          16,
          BandwidthToEnum(m_mobilityControlInfo.carrierBandwidth.ulBandwidth));
    }

    SerializeEnum(8, 0);

    SerializeBitstring(std::bitset<16>(m_mobilityControlInfo.newUeIdentity));

    SerializeRadioResourceConfigCommon(
        m_mobilityControlInfo.radioResourceConfigCommon);

    if (m_mobilityControlInfo.haveRachConfigDedicated) {
      SerializeSequence(std::bitset<0>(), false);
      SerializeInteger(
          m_mobilityControlInfo.rachConfigDedicated.raPreambleIndex, 0, 63);
      SerializeInteger(
          m_mobilityControlInfo.rachConfigDedicated.raPrachMaskIndex, 0, 15);
    }
  }

  if (m_haveRadioResourceConfigDedicated) {
    SerializeRadioResourceConfigDedicated(m_radioResourceConfigDedicated);
  }

  if (m_haveNonCriticalExtension) {
    std::bitset<2> noncriticalExtension_v890;
    noncriticalExtension_v890.set(1, false);
    noncriticalExtension_v890.set(0, m_haveNonCriticalExtension);
    SerializeSequence(noncriticalExtension_v890, false);

    std::bitset<3> noncriticalExtension_v920;
    noncriticalExtension_v920.set(1, false);
    noncriticalExtension_v920.set(1, false);
    noncriticalExtension_v920.set(0, m_haveNonCriticalExtension);
    SerializeSequence(noncriticalExtension_v920, false);

    SerializeNonCriticalExtensionConfiguration(m_nonCriticalExtension);
  }

  FinalizeSerialization();
}

uint32_t
RrcConnectionReconfigurationHeader::Deserialize(Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;

  bIterator = DeserializeDlDcchMessage(bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  int n;
  bIterator = DeserializeInteger(&n, 0, 3, bIterator);
  m_rrcTransactionIdentifier = n;

  int sel;
  bIterator = DeserializeChoice(2, false, &sel, bIterator);
  if (sel == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (sel == 0) {
    int c1Chosen;
    bIterator = DeserializeChoice(8, false, &c1Chosen, bIterator);
    if (c1Chosen > 0) {
      bIterator = DeserializeNull(bIterator);
    } else if (c1Chosen == 0) {
      std::bitset<6> rrcConnRecOpts;
      bIterator = DeserializeSequence(&rrcConnRecOpts, false, bIterator);

      m_haveMeasConfig = rrcConnRecOpts[5];
      if (m_haveMeasConfig) {
        bIterator = DeserializeMeasConfig(&m_measConfig, bIterator);
      }

      m_haveMobilityControlInfo = rrcConnRecOpts[4];
      if (m_haveMobilityControlInfo) {
        std::bitset<4> mobCtrlOpts;
        bIterator = DeserializeSequence(&mobCtrlOpts, true, bIterator);

        bIterator = DeserializeInteger(&n, 0, 503, bIterator);
        m_mobilityControlInfo.targetPhysCellId = n;

        m_mobilityControlInfo.haveCarrierFreq = mobCtrlOpts[3];
        if (m_mobilityControlInfo.haveCarrierFreq) {
          std::bitset<1> ulCarrierFreqPresent;
          bIterator =
              DeserializeSequence(&ulCarrierFreqPresent, false, bIterator);

          bIterator = DeserializeInteger(&n, 0, MAX_EARFCN, bIterator);
          m_mobilityControlInfo.carrierFreq.dlCarrierFreq = n;

          if (ulCarrierFreqPresent[0]) {
            bIterator = DeserializeInteger(&n, 0, MAX_EARFCN, bIterator);
            m_mobilityControlInfo.carrierFreq.ulCarrierFreq = n;
          }
        }

        m_mobilityControlInfo.haveCarrierBandwidth = mobCtrlOpts[2];
        if (m_mobilityControlInfo.haveCarrierBandwidth) {
          std::bitset<1> ulBandwidthPresent;
          bIterator =
              DeserializeSequence(&ulBandwidthPresent, false, bIterator);

          bIterator = DeserializeEnum(16, &n, bIterator);
          m_mobilityControlInfo.carrierBandwidth.dlBandwidth =
              EnumToBandwidth(n);

          if (ulBandwidthPresent[0]) {
            bIterator = DeserializeEnum(16, &n, bIterator);
            m_mobilityControlInfo.carrierBandwidth.ulBandwidth =
                EnumToBandwidth(n);
          }
        }

        if (mobCtrlOpts[1]) {
        }

        bIterator = DeserializeEnum(8, &n, bIterator);

        std::bitset<16> cRnti;
        bIterator = DeserializeBitstring(&cRnti, bIterator);
        m_mobilityControlInfo.newUeIdentity = cRnti.to_ulong();

        bIterator = DeserializeRadioResourceConfigCommon(
            &m_mobilityControlInfo.radioResourceConfigCommon, bIterator);

        m_mobilityControlInfo.haveRachConfigDedicated = mobCtrlOpts[0];
        if (m_mobilityControlInfo.haveRachConfigDedicated) {
          bIterator = DeserializeSequence(&bitset0, false, bIterator);
          bIterator = DeserializeInteger(&n, 0, 63, bIterator);
          m_mobilityControlInfo.rachConfigDedicated.raPreambleIndex = n;
          bIterator = DeserializeInteger(&n, 0, 15, bIterator);
          m_mobilityControlInfo.rachConfigDedicated.raPrachMaskIndex = n;
        }
      }

      if (rrcConnRecOpts[3]) {
      }

      m_haveRadioResourceConfigDedicated = rrcConnRecOpts[2];
      if (m_haveRadioResourceConfigDedicated) {
        bIterator = DeserializeRadioResourceConfigDedicated(
            &m_radioResourceConfigDedicated, bIterator);
      }

      if (rrcConnRecOpts[1]) {
      }

      m_haveNonCriticalExtension = rrcConnRecOpts[0];
      if (m_haveNonCriticalExtension) {
        bIterator = DeserializeNonCriticalExtensionConfig(
            &m_nonCriticalExtension, bIterator);
      }
    }
  }

  return GetSerializedSize();
}

void RrcConnectionReconfigurationHeader::Print(std::ostream &os) const {
  os << "rrcTransactionIdentifier: " << (int)m_rrcTransactionIdentifier
     << std::endl;
  os << "haveMeasConfig: " << m_haveMeasConfig << std::endl;
  if (m_haveMeasConfig) {
    if (!m_measConfig.measObjectToRemoveList.empty()) {
      os << "  measObjectToRemoveList: ";
      std::list<uint8_t> auxList = m_measConfig.measObjectToRemoveList;
      auto it = auxList.begin();
      for (; it != auxList.end(); it++) {
        os << (int)*it << ", ";
      }
      os << std::endl;
    }
    if (!m_measConfig.reportConfigToRemoveList.empty()) {
      os << "  reportConfigToRemoveList: ";
      std::list<uint8_t> auxList = m_measConfig.reportConfigToRemoveList;
      auto it = auxList.begin();
      for (; it != auxList.end(); it++) {
        os << (int)*it << ", ";
      }
      os << std::endl;
    }
    if (!m_measConfig.measIdToRemoveList.empty()) {
      os << "  measIdToRemoveList: ";
      std::list<uint8_t> auxList = m_measConfig.measIdToRemoveList;
      auto it = auxList.begin();
      for (; it != auxList.end(); it++) {
        os << (int)*it << ", ";
      }
      os << std::endl;
    }

    if (!m_measConfig.measObjectToAddModList.empty()) {
      os << "  measObjectToAddMod: " << std::endl;
      std::list<LteRrcSap::MeasObjectToAddMod> auxList =
          m_measConfig.measObjectToAddModList;
      auto it = auxList.begin();
      for (; it != auxList.end(); it++) {
        os << "    measObjectId: " << (int)it->measObjectId << std::endl;
        os << "    carrierFreq: " << (int)it->measObjectEutra.carrierFreq
           << std::endl;
        os << "    allowedMeasBandwidth: "
           << (int)it->measObjectEutra.allowedMeasBandwidth << std::endl;
        os << "    presenceAntennaPort1: "
           << it->measObjectEutra.presenceAntennaPort1 << std::endl;
        os << "    neighCellConfig: "
           << (int)it->measObjectEutra.neighCellConfig << std::endl;
        os << "    offsetFreq: " << (int)it->measObjectEutra.offsetFreq
           << std::endl;

        if (!it->measObjectEutra.cellsToRemoveList.empty()) {
          os << "    cellsToRemoveList: ";
          std::list<uint8_t> auxList = it->measObjectEutra.cellsToRemoveList;
          auto it = auxList.begin();
          for (; it != auxList.end(); it++) {
            os << (int)*it << ", ";
          }
          os << std::endl;
        }

        if (!it->measObjectEutra.blackCellsToRemoveList.empty()) {
          os << "    blackCellsToRemoveList: ";
          std::list<uint8_t> auxList =
              it->measObjectEutra.blackCellsToRemoveList;
          auto it = auxList.begin();
          for (; it != auxList.end(); it++) {
            os << (int)*it << ", ";
          }
          os << std::endl;
        }

        if (!it->measObjectEutra.cellsToAddModList.empty()) {
          os << "    cellsToAddModList: " << std::endl;
          std::list<LteRrcSap::CellsToAddMod> auxList =
              it->measObjectEutra.cellsToAddModList;
          auto it = auxList.begin();
          for (; it != auxList.end(); it++) {
            os << "      cellIndex: " << (int)it->cellIndex << std::endl;
            os << "      physCellId: " << (int)it->physCellId << std::endl;
            os << "      cellIndividualOffset: "
               << (int)it->cellIndividualOffset << std::endl;
            os << "      ------ " << std::endl;
          }
        }

        if (!it->measObjectEutra.blackCellsToAddModList.empty()) {
          os << "    blackCellsToAddModList: " << std::endl;
          std::list<LteRrcSap::BlackCellsToAddMod> auxList =
              it->measObjectEutra.blackCellsToAddModList;
          auto it = auxList.begin();
          for (; it != auxList.end(); it++) {
            os << "      cellIndex: " << (int)it->cellIndex << std::endl;
            os << "      physCellIdRange.start: "
               << (int)it->physCellIdRange.start << std::endl;
            os << "      physCellIdRange.haveRange: "
               << it->physCellIdRange.haveRange << std::endl;
            os << "      physCellIdRange.range: "
               << (int)it->physCellIdRange.range << std::endl;
            os << "      ------ " << std::endl;
          }
        }

        os << "    haveCellForWhichToReportCGI: "
           << it->measObjectEutra.haveCellForWhichToReportCGI << std::endl;
        os << "    cellForWhichToReportCGI: "
           << (int)it->measObjectEutra.cellForWhichToReportCGI << std::endl;
        os << "    ------------- " << std::endl;
      }
    }

    if (!m_measConfig.reportConfigToAddModList.empty()) {
      os << "  reportConfigToAddModList: " << std::endl;
      std::list<LteRrcSap::ReportConfigToAddMod> auxList =
          m_measConfig.reportConfigToAddModList;
      auto it = auxList.begin();
      for (; it != auxList.end(); it++) {
        os << "    reportConfigId: " << (int)it->reportConfigId << std::endl;
        os << "    reportConfigEutra.triggerType  "
           << (int)it->reportConfigEutra.triggerType << std::endl;
        if (it->reportConfigEutra.triggerType ==
            LteRrcSap::ReportConfigEutra::EVENT) {
          os << "    reportConfigEutra.eventId  "
             << (int)it->reportConfigEutra.eventId << std::endl;
          if (it->reportConfigEutra.eventId ==
              LteRrcSap::ReportConfigEutra::EVENT_A3) {
            os << "    reportConfigEutra.reportOnLeave  "
               << (int)it->reportConfigEutra.reportOnLeave << std::endl;
            os << "    reportConfigEutra.a3Offset  "
               << (int)it->reportConfigEutra.a3Offset << std::endl;
          } else {
            os << "    reportConfigEutra.threshold1.choice  "
               << (int)it->reportConfigEutra.threshold1.choice << std::endl;
            os << "    reportConfigEutra.threshold1.range  "
               << (int)it->reportConfigEutra.threshold1.range << std::endl;
            if (it->reportConfigEutra.eventId ==
                LteRrcSap::ReportConfigEutra::EVENT_A5) {
              os << "    reportConfigEutra.threshold2.choice  "
                 << (int)it->reportConfigEutra.threshold2.choice << std::endl;
              os << "    reportConfigEutra.threshold2.range  "
                 << (int)it->reportConfigEutra.threshold2.range << std::endl;
            }
          }
          os << "    reportConfigEutra.hysteresis  "
             << (int)it->reportConfigEutra.hysteresis << std::endl;
          os << "    reportConfigEutra.timeToTrigger  "
             << (int)it->reportConfigEutra.timeToTrigger << std::endl;
        } else {
          os << "    reportConfigEutra.purpose  "
             << (int)it->reportConfigEutra.purpose << std::endl;
        }
        os << "    reportConfigEutra.triggerQuantity  "
           << (int)it->reportConfigEutra.triggerQuantity << std::endl;
        os << "    reportConfigEutra.reportQuantity  "
           << (int)it->reportConfigEutra.reportQuantity << std::endl;
        os << "    reportConfigEutra.maxReportCells  "
           << (int)it->reportConfigEutra.maxReportCells << std::endl;
        os << "    reportConfigEutra.reportInterval  "
           << (int)it->reportConfigEutra.reportInterval << std::endl;
        os << "    reportConfigEutra.reportAmount  "
           << (int)it->reportConfigEutra.reportAmount << std::endl;
      }
    }

    if (!m_measConfig.measIdToAddModList.empty()) {
      os << "  measIdToAddModList: " << std::endl;
      std::list<LteRrcSap::MeasIdToAddMod> auxList =
          m_measConfig.measIdToAddModList;
      auto it = auxList.begin();
      for (; it != auxList.end(); it++) {
        os << "    measId: " << (int)it->measId << std::endl;
        os << "    measObjectId: " << (int)it->measObjectId << std::endl;
        os << "    reportConfigId: " << (int)it->reportConfigId << std::endl;
        os << "    ------ " << std::endl;
      }
    }

    os << "  haveQuantityConfig: " << m_measConfig.haveQuantityConfig
       << std::endl;
    if (m_measConfig.haveQuantityConfig) {
      os << "    filterCoefficientRSRP: "
         << (int)m_measConfig.quantityConfig.filterCoefficientRSRP << std::endl;
      os << "    filterCoefficientRSRQ:"
         << (int)m_measConfig.quantityConfig.filterCoefficientRSRQ << std::endl;
    }

    os << "  haveMeasGapConfig: " << m_measConfig.haveMeasGapConfig
       << std::endl;
    if (m_measConfig.haveMeasGapConfig) {
      os << "    measGapConfig.type: " << m_measConfig.measGapConfig.type
         << std::endl;
      os << "    measGapConfig.gap (gap0/1,value): ("
         << m_measConfig.measGapConfig.gapOffsetChoice << ","
         << (int)m_measConfig.measGapConfig.gapOffsetValue << ")" << std::endl;
    }

    os << "  haveSmeasure: " << m_measConfig.haveSmeasure << std::endl;
    if (m_measConfig.haveSmeasure) {
      os << "    sMeasure: " << (int)m_measConfig.sMeasure << std::endl;
    }

    os << "  haveSpeedStatePars: " << m_measConfig.haveSpeedStatePars
       << std::endl;
    if (m_measConfig.haveSpeedStatePars) {
      os << "    speedStatePars.type: " << m_measConfig.speedStatePars.type
         << std::endl;
      os << "    speedStatePars.mobilityStateParameters.tEvaluation: "
         << (int)m_measConfig.speedStatePars.mobilityStateParameters.tEvaluation
         << std::endl;
      os << "    speedStatePars.mobilityStateParameters.tHystNormal: "
         << (int)m_measConfig.speedStatePars.mobilityStateParameters.tHystNormal
         << std::endl;
      os << "    speedStatePars.mobilityStateParameters.nCellChangeMedium: "
         << (int)m_measConfig.speedStatePars.mobilityStateParameters
                .nCellChangeMedium
         << std::endl;
      os << "    speedStatePars.mobilityStateParameters.nCellChangeHigh: "
         << (int)m_measConfig.speedStatePars.mobilityStateParameters
                .nCellChangeHigh
         << std::endl;
      os << "    speedStatePars.timeToTriggerSf.sfMedium: "
         << (int)m_measConfig.speedStatePars.timeToTriggerSf.sfMedium
         << std::endl;
      os << "    speedStatePars.timeToTriggerSf.sfHigh: "
         << (int)m_measConfig.speedStatePars.timeToTriggerSf.sfHigh
         << std::endl;
    }
  }

  os << "haveMobilityControlInfo: " << m_haveMobilityControlInfo << std::endl;
  if (m_haveMobilityControlInfo) {
    os << "targetPhysCellId: " << (int)m_mobilityControlInfo.targetPhysCellId
       << std::endl;
    os << "haveCarrierFreq: " << m_mobilityControlInfo.haveCarrierFreq
       << std::endl;
    if (m_mobilityControlInfo.haveCarrierFreq) {
      os << "  carrierFreq.dlCarrierFreq: "
         << (int)m_mobilityControlInfo.carrierFreq.dlCarrierFreq << std::endl;
      os << "  carrierFreq.dlCarrierFreq: "
         << (int)m_mobilityControlInfo.carrierFreq.ulCarrierFreq << std::endl;
    }
    os << "haveCarrierBandwidth: " << m_mobilityControlInfo.haveCarrierBandwidth
       << std::endl;
    if (m_mobilityControlInfo.haveCarrierBandwidth) {
      os << "  carrierBandwidth.dlBandwidth: "
         << (int)m_mobilityControlInfo.carrierBandwidth.dlBandwidth
         << std::endl;
      os << "  carrierBandwidth.ulBandwidth: "
         << (int)m_mobilityControlInfo.carrierBandwidth.ulBandwidth
         << std::endl;
    }
    os << "newUeIdentity: " << (int)m_mobilityControlInfo.newUeIdentity
       << std::endl;
    os << "haveRachConfigDedicated: "
       << m_mobilityControlInfo.haveRachConfigDedicated << std::endl;
    if (m_mobilityControlInfo.haveRachConfigDedicated) {
      os << "raPreambleIndex: "
         << (int)m_mobilityControlInfo.rachConfigDedicated.raPreambleIndex
         << std::endl;
      os << "raPrachMaskIndex: "
         << (int)m_mobilityControlInfo.rachConfigDedicated.raPrachMaskIndex
         << std::endl;
    }
  }
  os << "haveRadioResourceConfigDedicated: "
     << m_haveRadioResourceConfigDedicated << std::endl;
  if (m_haveRadioResourceConfigDedicated) {
    RrcAsn1Header::Print(os, m_radioResourceConfigDedicated);
  }
}

void RrcConnectionReconfigurationHeader::SetMessage(
    LteRrcSap::RrcConnectionReconfiguration msg) {
  m_rrcTransactionIdentifier = msg.rrcTransactionIdentifier;
  m_haveMeasConfig = msg.haveMeasConfig;
  m_measConfig = msg.measConfig;
  m_haveMobilityControlInfo = msg.haveMobilityControlInfo;
  m_mobilityControlInfo = msg.mobilityControlInfo;
  m_haveRadioResourceConfigDedicated = msg.haveRadioResourceConfigDedicated;
  m_radioResourceConfigDedicated = msg.radioResourceConfigDedicated;
  m_haveNonCriticalExtension = msg.haveNonCriticalExtension;
  m_nonCriticalExtension = msg.nonCriticalExtension;

  m_isDataSerialized = false;
}

LteRrcSap::RrcConnectionReconfiguration
RrcConnectionReconfigurationHeader::GetMessage() const {
  LteRrcSap::RrcConnectionReconfiguration msg;

  msg.rrcTransactionIdentifier = m_rrcTransactionIdentifier;
  msg.haveMeasConfig = m_haveMeasConfig;
  msg.measConfig = m_measConfig;
  msg.haveMobilityControlInfo = m_haveMobilityControlInfo;
  msg.mobilityControlInfo = m_mobilityControlInfo;
  msg.haveRadioResourceConfigDedicated = m_haveRadioResourceConfigDedicated;
  msg.radioResourceConfigDedicated = m_radioResourceConfigDedicated;
  msg.haveNonCriticalExtension = m_haveNonCriticalExtension;
  msg.nonCriticalExtension = m_nonCriticalExtension;

  return msg;
}

uint8_t
RrcConnectionReconfigurationHeader::GetRrcTransactionIdentifier() const {
  return m_rrcTransactionIdentifier;
}

bool RrcConnectionReconfigurationHeader::GetHaveMeasConfig() const {
  return m_haveMeasConfig;
}

LteRrcSap::MeasConfig RrcConnectionReconfigurationHeader::GetMeasConfig() {
  return m_measConfig;
}

bool RrcConnectionReconfigurationHeader::GetHaveMobilityControlInfo() const {
  return m_haveMobilityControlInfo;
}

LteRrcSap::MobilityControlInfo
RrcConnectionReconfigurationHeader::GetMobilityControlInfo() {
  return m_mobilityControlInfo;
}

bool RrcConnectionReconfigurationHeader::GetHaveRadioResourceConfigDedicated()
    const {
  return m_haveRadioResourceConfigDedicated;
}

LteRrcSap::RadioResourceConfigDedicated
RrcConnectionReconfigurationHeader::GetRadioResourceConfigDedicated() {
  return m_radioResourceConfigDedicated;
}

bool RrcConnectionReconfigurationHeader::GetHaveNonCriticalExtensionConfig()
    const {
  return m_haveNonCriticalExtension;
}

LteRrcSap::NonCriticalExtensionConfiguration
RrcConnectionReconfigurationHeader::GetNonCriticalExtensionConfig() {
  return m_nonCriticalExtension;
}

bool RrcConnectionReconfigurationHeader::HavePhysicalConfigDedicated() const {
  return m_radioResourceConfigDedicated.havePhysicalConfigDedicated;
}

std::list<LteRrcSap::SrbToAddMod>
RrcConnectionReconfigurationHeader::GetSrbToAddModList() const {
  return m_radioResourceConfigDedicated.srbToAddModList;
}

std::list<LteRrcSap::DrbToAddMod>
RrcConnectionReconfigurationHeader::GetDrbToAddModList() const {
  return m_radioResourceConfigDedicated.drbToAddModList;
}

std::list<uint8_t>
RrcConnectionReconfigurationHeader::GetDrbToReleaseList() const {
  return m_radioResourceConfigDedicated.drbToReleaseList;
}

LteRrcSap::PhysicalConfigDedicated
RrcConnectionReconfigurationHeader::GetPhysicalConfigDedicated() const {
  return m_radioResourceConfigDedicated.physicalConfigDedicated;
}

HandoverPreparationInfoHeader::HandoverPreparationInfoHeader() {}

void HandoverPreparationInfoHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeSequence(std::bitset<0>(), false);

  SerializeChoice(2, 0, false);

  SerializeChoice(8, 0, false);

  std::bitset<4> handoverPrepInfoOpts;
  handoverPrepInfoOpts.set(3, true);
  handoverPrepInfoOpts.set(2, false);
  handoverPrepInfoOpts.set(1, false);
  handoverPrepInfoOpts.set(0, false);
  SerializeSequence(handoverPrepInfoOpts, false);

  SerializeSequenceOf(0, MAX_RAT_CAPABILITIES, 0);

  SerializeSequence(std::bitset<0>(), true);

  SerializeMeasConfig(m_asConfig.sourceMeasConfig);

  SerializeRadioResourceConfigDedicated(m_asConfig.sourceRadioResourceConfig);

  SerializeSequence(std::bitset<0>(), false);
  SerializeEnum(8, 0);
  SerializeEnum(8, 0);

  SerializeBitstring(std::bitset<16>(m_asConfig.sourceUeIdentity));

  SerializeSequence(std::bitset<0>(), false);
  SerializeEnum(
      6, BandwidthToEnum(m_asConfig.sourceMasterInformationBlock.dlBandwidth));
  SerializeSequence(std::bitset<0>(), false);
  SerializeEnum(2, 0);
  SerializeEnum(4, 0);
  SerializeBitstring(std::bitset<8>(
      m_asConfig.sourceMasterInformationBlock.systemFrameNumber));
  SerializeBitstring(std::bitset<10>(321));

  SerializeSystemInformationBlockType1(
      m_asConfig.sourceSystemInformationBlockType1);

  SerializeSystemInformationBlockType2(
      m_asConfig.sourceSystemInformationBlockType2);

  SerializeSequence(std::bitset<0>(0), false);
  SerializeEnum(4, 0);

  SerializeInteger(m_asConfig.sourceDlCarrierFreq, 0, MAX_EARFCN);

  FinalizeSerialization();
}

uint32_t
HandoverPreparationInfoHeader::Deserialize(Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  int criticalExtensionsChosen;
  bIterator = DeserializeChoice(2, false, &criticalExtensionsChosen, bIterator);

  if (criticalExtensionsChosen == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (criticalExtensionsChosen == 0) {
    int c1Chosen;
    bIterator = DeserializeChoice(8, false, &c1Chosen, bIterator);
    if (c1Chosen > 0) {
      bIterator = DeserializeNull(bIterator);
    } else if (c1Chosen == 0) {
      std::bitset<4> handoverPrepInfoOpts;
      bIterator = DeserializeSequence(&handoverPrepInfoOpts, false, bIterator);

      bIterator = DeserializeSequenceOf(&n, MAX_RAT_CAPABILITIES, 0, bIterator);
      for (int i = 0; i < n; i++) {
      }

      if (handoverPrepInfoOpts[3]) {
        bIterator = DeserializeSequence(&bitset0, true, bIterator);

        bIterator =
            DeserializeMeasConfig(&m_asConfig.sourceMeasConfig, bIterator);

        bIterator = DeserializeRadioResourceConfigDedicated(
            &m_asConfig.sourceRadioResourceConfig, bIterator);

        bIterator = DeserializeSequence(&bitset0, false, bIterator);
        bIterator = DeserializeEnum(8, &n, bIterator);
        bIterator = DeserializeEnum(8, &n, bIterator);

        std::bitset<16> cRnti;
        bIterator = DeserializeBitstring(&cRnti, bIterator);
        m_asConfig.sourceUeIdentity = cRnti.to_ulong();

        bIterator = DeserializeSequence(&bitset0, false, bIterator);
        bIterator = DeserializeEnum(6, &n, bIterator);
        m_asConfig.sourceMasterInformationBlock.dlBandwidth =
            EnumToBandwidth(n);

        bIterator = DeserializeSequence(&bitset0, false, bIterator);
        bIterator = DeserializeEnum(2, &n, bIterator);
        bIterator = DeserializeEnum(4, &n, bIterator);

        std::bitset<8> systemFrameNumber;
        bIterator = DeserializeBitstring(&systemFrameNumber, bIterator);
        m_asConfig.sourceMasterInformationBlock.systemFrameNumber =
            systemFrameNumber.to_ulong();
        std::bitset<10> spare;
        bIterator = DeserializeBitstring(&spare, bIterator);

        bIterator = DeserializeSystemInformationBlockType1(
            &m_asConfig.sourceSystemInformationBlockType1, bIterator);

        bIterator = DeserializeSystemInformationBlockType2(
            &m_asConfig.sourceSystemInformationBlockType2, bIterator);

        bIterator = DeserializeSequence(&bitset0, false, bIterator);
        bIterator = DeserializeEnum(4, &n, bIterator);

        bIterator = DeserializeInteger(&n, 0, MAX_EARFCN, bIterator);
        m_asConfig.sourceDlCarrierFreq = n;
      }
      if (handoverPrepInfoOpts[2]) {
      }
      if (handoverPrepInfoOpts[1]) {
      }
      if (handoverPrepInfoOpts[0]) {
      }
    }
  }

  return GetSerializedSize();
}

void HandoverPreparationInfoHeader::Print(std::ostream &os) const {
  RrcAsn1Header::Print(os, m_asConfig.sourceRadioResourceConfig);
  os << "sourceUeIdentity: " << m_asConfig.sourceUeIdentity << std::endl;
  os << "dlBandwidth: "
     << (int)m_asConfig.sourceMasterInformationBlock.dlBandwidth << std::endl;
  os << "systemFrameNumber: "
     << (int)m_asConfig.sourceMasterInformationBlock.systemFrameNumber
     << std::endl;
  os << "plmnIdentityInfo.plmnIdentity: "
     << (int)m_asConfig.sourceSystemInformationBlockType1.cellAccessRelatedInfo
            .plmnIdentityInfo.plmnIdentity
     << std::endl;
  os << "cellAccessRelatedInfo.cellIdentity "
     << (int)m_asConfig.sourceSystemInformationBlockType1.cellAccessRelatedInfo
            .cellIdentity
     << std::endl;
  os << "cellAccessRelatedInfo.csgIndication: "
     << m_asConfig.sourceSystemInformationBlockType1.cellAccessRelatedInfo
            .csgIndication
     << std::endl;
  os << "cellAccessRelatedInfo.csgIdentity: "
     << (int)m_asConfig.sourceSystemInformationBlockType1.cellAccessRelatedInfo
            .csgIdentity
     << std::endl;
  os << "sourceDlCarrierFreq: " << m_asConfig.sourceDlCarrierFreq << std::endl;
}

void HandoverPreparationInfoHeader::SetMessage(
    LteRrcSap::HandoverPreparationInfo msg) {
  m_asConfig = msg.asConfig;
  m_isDataSerialized = false;
}

LteRrcSap::HandoverPreparationInfo
HandoverPreparationInfoHeader::GetMessage() const {
  LteRrcSap::HandoverPreparationInfo msg;
  msg.asConfig = m_asConfig;

  return msg;
}

LteRrcSap::AsConfig HandoverPreparationInfoHeader::GetAsConfig() const {
  return m_asConfig;
}

RrcConnectionReestablishmentRequestHeader::
    RrcConnectionReestablishmentRequestHeader() {}

RrcConnectionReestablishmentRequestHeader::
    ~RrcConnectionReestablishmentRequestHeader() {}

void RrcConnectionReestablishmentRequestHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeUlCcchMessage(0);

  SerializeSequence(std::bitset<0>(), false);

  SerializeChoice(2, 0, false);

  SerializeSequence(std::bitset<0>(), false);

  SerializeSequence(std::bitset<0>(), false);
  SerializeBitstring(std::bitset<16>(m_ueIdentity.cRnti));
  SerializeInteger(m_ueIdentity.physCellId, 0, 503);
  SerializeBitstring(std::bitset<16>(0));

  switch (m_reestablishmentCause) {
  case LteRrcSap::RECONFIGURATION_FAILURE:
    SerializeEnum(4, 0);
    break;
  case LteRrcSap::HANDOVER_FAILURE:
    SerializeEnum(4, 1);
    break;
  case LteRrcSap::OTHER_FAILURE:
    SerializeEnum(4, 2);
    break;
  default:
    SerializeEnum(4, 3);
  }

  SerializeBitstring(std::bitset<2>(0));

  FinalizeSerialization();
}

uint32_t RrcConnectionReestablishmentRequestHeader::Deserialize(
    Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeUlCcchMessage(bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeChoice(2, false, &n, bIterator);
  if (n == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (n == 0) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);

    bIterator = DeserializeSequence(&bitset0, false, bIterator);

    std::bitset<16> cRnti;
    bIterator = DeserializeBitstring(&cRnti, bIterator);
    m_ueIdentity.cRnti = cRnti.to_ulong();

    int physCellId;
    bIterator = DeserializeInteger(&physCellId, 0, 503, bIterator);
    m_ueIdentity.physCellId = physCellId;

    std::bitset<16> shortMacI;
    bIterator = DeserializeBitstring(&shortMacI, bIterator);

    int reestCs;
    bIterator = DeserializeEnum(4, &reestCs, bIterator);
    switch (reestCs) {
    case 0:
      m_reestablishmentCause = LteRrcSap::RECONFIGURATION_FAILURE;
      break;
    case 1:
      m_reestablishmentCause = LteRrcSap::HANDOVER_FAILURE;
      break;
    case 2:
      m_reestablishmentCause = LteRrcSap::OTHER_FAILURE;
      break;
    case 3:
      break;
    }

    std::bitset<2> spare;
    bIterator = DeserializeBitstring(&spare, bIterator);
  }

  return GetSerializedSize();
}

void RrcConnectionReestablishmentRequestHeader::Print(std::ostream &os) const {
  os << "ueIdentity.cRnti: " << (int)m_ueIdentity.cRnti << std::endl;
  os << "ueIdentity.physCellId: " << (int)m_ueIdentity.physCellId << std::endl;
  os << "m_reestablishmentCause: " << m_reestablishmentCause << std::endl;
}

void RrcConnectionReestablishmentRequestHeader::SetMessage(
    LteRrcSap::RrcConnectionReestablishmentRequest msg) {
  m_ueIdentity = msg.ueIdentity;
  m_reestablishmentCause = msg.reestablishmentCause;
  m_isDataSerialized = false;
}

LteRrcSap::RrcConnectionReestablishmentRequest
RrcConnectionReestablishmentRequestHeader::GetMessage() const {
  LteRrcSap::RrcConnectionReestablishmentRequest msg;
  msg.ueIdentity = m_ueIdentity;
  msg.reestablishmentCause = m_reestablishmentCause;

  return msg;
}

LteRrcSap::ReestabUeIdentity
RrcConnectionReestablishmentRequestHeader::GetUeIdentity() const {
  return m_ueIdentity;
}

LteRrcSap::ReestablishmentCause
RrcConnectionReestablishmentRequestHeader::GetReestablishmentCause() const {
  return m_reestablishmentCause;
}

RrcConnectionReestablishmentHeader::RrcConnectionReestablishmentHeader() {}

RrcConnectionReestablishmentHeader::~RrcConnectionReestablishmentHeader() {}

void RrcConnectionReestablishmentHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeDlCcchMessage(0);

  SerializeSequence(std::bitset<0>(), false);

  SerializeInteger(m_rrcTransactionIdentifier, 0, 3);

  SerializeChoice(2, 0, false);

  SerializeChoice(8, 0, false);

  SerializeSequence(std::bitset<1>(0), false);

  SerializeRadioResourceConfigDedicated(m_radioResourceConfigDedicated);

  SerializeInteger(0, 0, 7);

  FinalizeSerialization();
}

uint32_t
RrcConnectionReestablishmentHeader::Deserialize(Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeDlCcchMessage(bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeInteger(&n, 0, 3, bIterator);
  m_rrcTransactionIdentifier = n;

  int criticalExtensionsChoice;
  bIterator = DeserializeChoice(2, false, &criticalExtensionsChoice, bIterator);
  if (criticalExtensionsChoice == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (criticalExtensionsChoice == 0) {
    int c1;
    bIterator = DeserializeChoice(8, false, &c1, bIterator);
    if (c1 > 0) {
      bIterator = DeserializeNull(bIterator);
    } else if (c1 == 0) {
      std::bitset<1> nonCriticalExtensionPresent;
      bIterator =
          DeserializeSequence(&nonCriticalExtensionPresent, false, bIterator);

      bIterator = DeserializeRadioResourceConfigDedicated(
          &m_radioResourceConfigDedicated, bIterator);

      bIterator = DeserializeInteger(&n, 0, 7, bIterator);
    }
  }

  return GetSerializedSize();
}

void RrcConnectionReestablishmentHeader::Print(std::ostream &os) const {
  os << "rrcTransactionIdentifier: " << (int)m_rrcTransactionIdentifier
     << std::endl;
  os << "RadioResourceConfigDedicated: " << std::endl;
  RrcAsn1Header::Print(os, m_radioResourceConfigDedicated);
}

void RrcConnectionReestablishmentHeader::SetMessage(
    LteRrcSap::RrcConnectionReestablishment msg) {
  m_rrcTransactionIdentifier = msg.rrcTransactionIdentifier;
  m_radioResourceConfigDedicated = msg.radioResourceConfigDedicated;
  m_isDataSerialized = false;
}

LteRrcSap::RrcConnectionReestablishment
RrcConnectionReestablishmentHeader::GetMessage() const {
  LteRrcSap::RrcConnectionReestablishment msg;
  msg.rrcTransactionIdentifier = m_rrcTransactionIdentifier;
  msg.radioResourceConfigDedicated = m_radioResourceConfigDedicated;
  return msg;
}

uint8_t
RrcConnectionReestablishmentHeader::GetRrcTransactionIdentifier() const {
  return m_rrcTransactionIdentifier;
}

LteRrcSap::RadioResourceConfigDedicated
RrcConnectionReestablishmentHeader::GetRadioResourceConfigDedicated() const {
  return m_radioResourceConfigDedicated;
}

RrcConnectionReestablishmentCompleteHeader::
    RrcConnectionReestablishmentCompleteHeader() {}

void RrcConnectionReestablishmentCompleteHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeUlDcchMessage(3);

  SerializeSequence(std::bitset<0>(), false);

  SerializeInteger(m_rrcTransactionIdentifier, 0, 3);

  SerializeChoice(2, 0, false);

  SerializeSequence(std::bitset<1>(0), false);

  FinalizeSerialization();
}

uint32_t RrcConnectionReestablishmentCompleteHeader::Deserialize(
    Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeUlDcchMessage(bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeInteger(&n, 0, 3, bIterator);
  m_rrcTransactionIdentifier = n;

  int criticalExtensionsChoice;
  bIterator = DeserializeChoice(2, false, &criticalExtensionsChoice, bIterator);
  if (criticalExtensionsChoice == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (criticalExtensionsChoice == 0) {
    std::bitset<1> opts;
    bIterator = DeserializeSequence(&opts, false, bIterator);
    if (opts[0]) {
    }
  }

  return GetSerializedSize();
}

void RrcConnectionReestablishmentCompleteHeader::Print(std::ostream &os) const {
  os << "rrcTransactionIdentifier: " << (int)m_rrcTransactionIdentifier
     << std::endl;
}

void RrcConnectionReestablishmentCompleteHeader::SetMessage(
    LteRrcSap::RrcConnectionReestablishmentComplete msg) {
  m_rrcTransactionIdentifier = msg.rrcTransactionIdentifier;
  m_isDataSerialized = false;
}

LteRrcSap::RrcConnectionReestablishmentComplete
RrcConnectionReestablishmentCompleteHeader::GetMessage() const {
  LteRrcSap::RrcConnectionReestablishmentComplete msg;
  msg.rrcTransactionIdentifier = m_rrcTransactionIdentifier;
  return msg;
}

uint8_t
RrcConnectionReestablishmentCompleteHeader::GetRrcTransactionIdentifier()
    const {
  return m_rrcTransactionIdentifier;
}

RrcConnectionReestablishmentRejectHeader::
    RrcConnectionReestablishmentRejectHeader() {}

RrcConnectionReestablishmentRejectHeader::
    ~RrcConnectionReestablishmentRejectHeader() {}

void RrcConnectionReestablishmentRejectHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeDlCcchMessage(1);

  SerializeSequence(std::bitset<0>(), false);

  SerializeChoice(2, 0, false);

  SerializeSequence(std::bitset<1>(0), false);

  FinalizeSerialization();
}

uint32_t RrcConnectionReestablishmentRejectHeader::Deserialize(
    Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;

  bIterator = DeserializeDlCcchMessage(bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  int criticalExtensionsChoice;
  bIterator = DeserializeChoice(2, false, &criticalExtensionsChoice, bIterator);
  if (criticalExtensionsChoice == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (criticalExtensionsChoice == 0) {
    std::bitset<1> opts;
    bIterator = DeserializeSequence(&opts, false, bIterator);
    if (opts[0]) {
    }
  }

  return GetSerializedSize();
}

void RrcConnectionReestablishmentRejectHeader::Print(std::ostream &os) const {}

void RrcConnectionReestablishmentRejectHeader::SetMessage(
    LteRrcSap::RrcConnectionReestablishmentReject msg) {
  m_rrcConnectionReestablishmentReject = msg;
  m_isDataSerialized = false;
}

LteRrcSap::RrcConnectionReestablishmentReject
RrcConnectionReestablishmentRejectHeader::GetMessage() const {
  return m_rrcConnectionReestablishmentReject;
}

RrcConnectionReleaseHeader::RrcConnectionReleaseHeader() {}

RrcConnectionReleaseHeader::~RrcConnectionReleaseHeader() {}

void RrcConnectionReleaseHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeDlDcchMessage(5);

  SerializeSequence(std::bitset<0>(), false);

  SerializeInteger(m_rrcConnectionRelease.rrcTransactionIdentifier, 0, 3);

  SerializeChoice(2, 0, false);

  SerializeChoice(4, 0, false);

  SerializeSequence(std::bitset<3>(0), false);

  SerializeEnum(4, 1);

  FinalizeSerialization();
}

uint32_t RrcConnectionReleaseHeader::Deserialize(Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeDlDcchMessage(bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeInteger(&n, 0, 3, bIterator);
  m_rrcConnectionRelease.rrcTransactionIdentifier = n;

  int criticalExtensionsChoice;
  bIterator = DeserializeChoice(2, false, &criticalExtensionsChoice, bIterator);
  if (criticalExtensionsChoice == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (criticalExtensionsChoice == 0) {
    int c1Choice;
    bIterator = DeserializeChoice(4, false, &c1Choice, bIterator);

    if (c1Choice == 0) {
      std::bitset<3> opts;
      bIterator = DeserializeSequence(&opts, false, bIterator);

      bIterator = DeserializeEnum(4, &n, bIterator);

      if (opts[2]) {
      }
      if (opts[1]) {
      }
      if (opts[0]) {
      }
    }

    else {
      bIterator = DeserializeNull(bIterator);
    }
  }

  return GetSerializedSize();
}

void RrcConnectionReleaseHeader::Print(std::ostream &os) const {}

void RrcConnectionReleaseHeader::SetMessage(
    LteRrcSap::RrcConnectionRelease msg) {
  m_rrcConnectionRelease = msg;
  m_isDataSerialized = false;
}

LteRrcSap::RrcConnectionRelease RrcConnectionReleaseHeader::GetMessage() const {
  return m_rrcConnectionRelease;
}

RrcConnectionRejectHeader::RrcConnectionRejectHeader() {}

RrcConnectionRejectHeader::~RrcConnectionRejectHeader() {}

void RrcConnectionRejectHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeDlCcchMessage(2);

  SerializeSequence(std::bitset<0>(), false);

  SerializeChoice(2, 0, false);

  SerializeChoice(4, 0, false);

  SerializeSequence(std::bitset<1>(0), false);

  SerializeInteger(m_rrcConnectionReject.waitTime, 1, 16);

  FinalizeSerialization();
}

uint32_t RrcConnectionRejectHeader::Deserialize(Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeDlCcchMessage(bIterator);

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  int criticalExtensionsChoice;
  bIterator = DeserializeChoice(2, false, &criticalExtensionsChoice, bIterator);
  if (criticalExtensionsChoice == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (criticalExtensionsChoice == 0) {
    int c1Choice;
    bIterator = DeserializeChoice(4, false, &c1Choice, bIterator);

    if (c1Choice > 0) {
      bIterator = DeserializeNull(bIterator);
    } else if (c1Choice == 0) {
      std::bitset<1> opts;
      bIterator = DeserializeSequence(&opts, false, bIterator);

      bIterator = DeserializeInteger(&n, 1, 16, bIterator);
      m_rrcConnectionReject.waitTime = n;

      if (opts[0]) {
      }
    }
  }

  return GetSerializedSize();
}

void RrcConnectionRejectHeader::Print(std::ostream &os) const {
  os << "wait time: " << (int)m_rrcConnectionReject.waitTime << std::endl;
}

void RrcConnectionRejectHeader::SetMessage(LteRrcSap::RrcConnectionReject msg) {
  m_rrcConnectionReject = msg;
  m_isDataSerialized = false;
}

LteRrcSap::RrcConnectionReject RrcConnectionRejectHeader::GetMessage() const {
  return m_rrcConnectionReject;
}

MeasurementReportHeader::MeasurementReportHeader() {}

MeasurementReportHeader::~MeasurementReportHeader() {}

void MeasurementReportHeader::PreSerialize() const {
  m_serializationResult = Buffer();

  SerializeUlDcchMessage(1);

  SerializeSequence(std::bitset<0>(), false);

  SerializeChoice(2, 0, false);

  SerializeChoice(8, 0, false);

  SerializeSequence(std::bitset<1>(0), false);

  SerializeMeasResults(m_measurementReport.measResults);

  FinalizeSerialization();
}

uint32_t MeasurementReportHeader::Deserialize(Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;

  bIterator = DeserializeSequence(&bitset0, false, bIterator);

  bIterator = DeserializeUlDcchMessage(bIterator);

  int criticalExtensionsChoice;
  bIterator = DeserializeChoice(2, false, &criticalExtensionsChoice, bIterator);

  if (criticalExtensionsChoice == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
  } else if (criticalExtensionsChoice == 0) {
    int c1Choice;
    bIterator = DeserializeChoice(8, false, &c1Choice, bIterator);

    if (c1Choice > 0) {
      bIterator = DeserializeNull(bIterator);
    } else {
      std::bitset<1> isNonCriticalExtensionPresent;
      bIterator =
          DeserializeSequence(&isNonCriticalExtensionPresent, false, bIterator);

      bIterator =
          DeserializeMeasResults(&m_measurementReport.measResults, bIterator);

      if (isNonCriticalExtensionPresent[0]) {
      }
    }
  }

  return GetSerializedSize();
}

void MeasurementReportHeader::Print(std::ostream &os) const {
  os << "measId = " << (int)m_measurementReport.measResults.measId << std::endl;
  os << "rsrpResult = "
     << (int)m_measurementReport.measResults.measResultPCell.rsrpResult
     << std::endl;
  os << "rsrqResult = "
     << (int)m_measurementReport.measResults.measResultPCell.rsrqResult
     << std::endl;
  os << "haveMeasResultNeighCells = "
     << (int)m_measurementReport.measResults.haveMeasResultNeighCells
     << std::endl;

  if (m_measurementReport.measResults.haveMeasResultNeighCells) {
    std::list<LteRrcSap::MeasResultEutra> measResultListEutra =
        m_measurementReport.measResults.measResultListEutra;
    auto it = measResultListEutra.begin();
    for (; it != measResultListEutra.end(); it++) {
      os << "   physCellId =" << (int)it->physCellId << std::endl;
      os << "   haveCgiInfo =" << it->haveCgiInfo << std::endl;
      if (it->haveCgiInfo) {
        os << "      plmnIdentity = " << (int)it->cgiInfo.plmnIdentity
           << std::endl;
        os << "      cellIdentity = " << (int)it->cgiInfo.cellIdentity
           << std::endl;
        os << "      trackingAreaCode = " << (int)it->cgiInfo.trackingAreaCode
           << std::endl;
        os << "      havePlmnIdentityList = "
           << !it->cgiInfo.plmnIdentityList.empty() << std::endl;
        if (!it->cgiInfo.plmnIdentityList.empty()) {
          for (auto it2 = it->cgiInfo.plmnIdentityList.begin();
               it2 != it->cgiInfo.plmnIdentityList.end(); it2++) {
            os << "         plmnId : " << *it2 << std::endl;
          }
        }
      }

      os << "   haveRsrpResult =" << it->haveRsrpResult << std::endl;
      if (it->haveRsrpResult) {
        os << "   rsrpResult =" << (int)it->rsrpResult << std::endl;
      }

      os << "   haveRsrqResult =" << it->haveRsrqResult << std::endl;
      if (it->haveRsrqResult) {
        os << "   rsrqResult =" << (int)it->rsrqResult << std::endl;
      }
    }
  }
}

void MeasurementReportHeader::SetMessage(LteRrcSap::MeasurementReport msg) {
  m_measurementReport = msg;
  m_isDataSerialized = false;
}

LteRrcSap::MeasurementReport MeasurementReportHeader::GetMessage() const {
  LteRrcSap::MeasurementReport msg;
  msg = m_measurementReport;
  return msg;
}

RrcUlDcchMessage::RrcUlDcchMessage() : RrcAsn1Header() {}

RrcUlDcchMessage::~RrcUlDcchMessage() {}

uint32_t RrcUlDcchMessage::Deserialize(Buffer::Iterator bIterator) {
  DeserializeUlDcchMessage(bIterator);
  return 1;
}

void RrcUlDcchMessage::Print(std::ostream &os) const {
  std::cout << "UL DCCH MSG TYPE: " << m_messageType << std::endl;
}

void RrcUlDcchMessage::PreSerialize() const {
  SerializeUlDcchMessage(m_messageType);
}

Buffer::Iterator
RrcUlDcchMessage::DeserializeUlDcchMessage(Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeSequence(&bitset0, false, bIterator);
  bIterator = DeserializeChoice(2, false, &n, bIterator);
  if (n == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
    m_messageType = -1;
  } else if (n == 0) {
    bIterator = DeserializeChoice(16, false, &m_messageType, bIterator);
  }

  return bIterator;
}

void RrcUlDcchMessage::SerializeUlDcchMessage(int messageType) const {
  SerializeSequence(std::bitset<0>(), false);
  SerializeChoice(2, 0, false);
  SerializeChoice(16, messageType, false);
}

RrcDlDcchMessage::RrcDlDcchMessage() : RrcAsn1Header() {}

RrcDlDcchMessage::~RrcDlDcchMessage() {}

uint32_t RrcDlDcchMessage::Deserialize(Buffer::Iterator bIterator) {
  DeserializeDlDcchMessage(bIterator);
  return 1;
}

void RrcDlDcchMessage::Print(std::ostream &os) const {
  std::cout << "DL DCCH MSG TYPE: " << m_messageType << std::endl;
}

void RrcDlDcchMessage::PreSerialize() const {
  SerializeDlDcchMessage(m_messageType);
}

Buffer::Iterator
RrcDlDcchMessage::DeserializeDlDcchMessage(Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeSequence(&bitset0, false, bIterator);
  bIterator = DeserializeChoice(2, false, &n, bIterator);
  if (n == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
    m_messageType = -1;
  } else if (n == 0) {
    bIterator = DeserializeChoice(16, false, &m_messageType, bIterator);
  }

  return bIterator;
}

void RrcDlDcchMessage::SerializeDlDcchMessage(int messageType) const {
  SerializeSequence(std::bitset<0>(), false);
  SerializeChoice(2, 0, false);
  SerializeChoice(16, messageType, false);
}

RrcUlCcchMessage::RrcUlCcchMessage() : RrcAsn1Header() {}

RrcUlCcchMessage::~RrcUlCcchMessage() {}

uint32_t RrcUlCcchMessage::Deserialize(Buffer::Iterator bIterator) {
  DeserializeUlCcchMessage(bIterator);
  return 1;
}

void RrcUlCcchMessage::Print(std::ostream &os) const {
  std::cout << "UL CCCH MSG TYPE: " << m_messageType << std::endl;
}

void RrcUlCcchMessage::PreSerialize() const {
  SerializeUlCcchMessage(m_messageType);
}

Buffer::Iterator
RrcUlCcchMessage::DeserializeUlCcchMessage(Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeSequence(&bitset0, false, bIterator);
  bIterator = DeserializeChoice(2, false, &n, bIterator);
  if (n == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
    m_messageType = -1;
  } else if (n == 0) {
    bIterator = DeserializeChoice(2, false, &m_messageType, bIterator);
  }

  return bIterator;
}

void RrcUlCcchMessage::SerializeUlCcchMessage(int messageType) const {
  SerializeSequence(std::bitset<0>(), false);
  SerializeChoice(2, 0, false);
  SerializeChoice(2, messageType, false);
}

RrcDlCcchMessage::RrcDlCcchMessage() : RrcAsn1Header() {}

RrcDlCcchMessage::~RrcDlCcchMessage() {}

uint32_t RrcDlCcchMessage::Deserialize(Buffer::Iterator bIterator) {
  DeserializeDlCcchMessage(bIterator);
  return 1;
}

void RrcDlCcchMessage::Print(std::ostream &os) const {
  std::cout << "DL CCCH MSG TYPE: " << m_messageType << std::endl;
}

void RrcDlCcchMessage::PreSerialize() const {
  SerializeDlCcchMessage(m_messageType);
}

Buffer::Iterator
RrcDlCcchMessage::DeserializeDlCcchMessage(Buffer::Iterator bIterator) {
  std::bitset<0> bitset0;
  int n;

  bIterator = DeserializeSequence(&bitset0, false, bIterator);
  bIterator = DeserializeChoice(2, false, &n, bIterator);
  if (n == 1) {
    bIterator = DeserializeSequence(&bitset0, false, bIterator);
    m_messageType = -1;
  } else if (n == 0) {
    bIterator = DeserializeChoice(4, false, &m_messageType, bIterator);
  }

  return bIterator;
}

void RrcDlCcchMessage::SerializeDlCcchMessage(int messageType) const {
  SerializeSequence(std::bitset<0>(), false);
  SerializeChoice(2, 0, false);
  SerializeChoice(4, messageType, false);
}

} // namespace ns3
