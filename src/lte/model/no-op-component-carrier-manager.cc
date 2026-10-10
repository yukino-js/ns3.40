
#include "no-op-component-carrier-manager.h"

#include "lte-common.h"

#include <ns3/log.h>
#include <ns3/random-variable-stream.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("NoOpComponentCarrierManager");
NS_OBJECT_ENSURE_REGISTERED(NoOpComponentCarrierManager);

NoOpComponentCarrierManager::NoOpComponentCarrierManager() {
  NS_LOG_FUNCTION(this);
  m_ccmRrcSapProvider =
      new MemberLteCcmRrcSapProvider<NoOpComponentCarrierManager>(this);
  m_ccmMacSapUser =
      new MemberLteCcmMacSapUser<NoOpComponentCarrierManager>(this);
  m_macSapProvider =
      new EnbMacMemberLteMacSapProvider<NoOpComponentCarrierManager>(this);
  m_ccmRrcSapUser = nullptr;
}

NoOpComponentCarrierManager::~NoOpComponentCarrierManager() {
  NS_LOG_FUNCTION(this);
}

void NoOpComponentCarrierManager::DoDispose() {
  NS_LOG_FUNCTION(this);
  delete m_ccmRrcSapProvider;
  delete m_ccmMacSapUser;
  delete m_macSapProvider;
}

TypeId NoOpComponentCarrierManager::GetTypeId() {
  static TypeId tid = TypeId("ns3::NoOpComponentCarrierManager")
                          .SetParent<LteEnbComponentCarrierManager>()
                          .SetGroupName("Lte")
                          .AddConstructor<NoOpComponentCarrierManager>();
  return tid;
}

void NoOpComponentCarrierManager::DoInitialize() {
  NS_LOG_FUNCTION(this);
  LteEnbComponentCarrierManager::DoInitialize();
}

void NoOpComponentCarrierManager::DoTransmitPdu(
    LteMacSapProvider::TransmitPduParameters params) {
  NS_LOG_FUNCTION(this);
  auto it = m_macSapProvidersMap.find(params.componentCarrierId);
  NS_ASSERT_MSG(it != m_macSapProvidersMap.end(),
                "could not find Sap for ComponentCarrier "
                    << params.componentCarrierId);
  it->second->TransmitPdu(params);
}

void NoOpComponentCarrierManager::DoReportBufferStatus(
    LteMacSapProvider::ReportBufferStatusParameters params) {
  NS_LOG_FUNCTION(this);
  auto ueManager = m_ccmRrcSapUser->GetUeManager(params.rnti);
  auto it = m_macSapProvidersMap.find(ueManager->GetComponentCarrierId());
  NS_ASSERT_MSG(it != m_macSapProvidersMap.end(),
                "could not find Sap for ComponentCarrier ");
  it->second->ReportBufferStatus(params);
}

void NoOpComponentCarrierManager::DoNotifyTxOpportunity(
    LteMacSapUser::TxOpportunityParameters txOpParams) {
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG(this << " rnti= " << txOpParams.rnti << " lcid= "
                    << +txOpParams.lcid << " layer= " << +txOpParams.layer
                    << " ccId=" << +txOpParams.componentCarrierId);
  m_ueInfo.at(txOpParams.rnti)
      .m_ueAttached.at(txOpParams.lcid)
      ->NotifyTxOpportunity(txOpParams);
}

void NoOpComponentCarrierManager::DoReceivePdu(
    LteMacSapUser::ReceivePduParameters rxPduParams) {
  NS_LOG_FUNCTION(this);
  auto lcidIt =
      m_ueInfo.at(rxPduParams.rnti).m_ueAttached.find(rxPduParams.lcid);
  if (lcidIt != m_ueInfo.at(rxPduParams.rnti).m_ueAttached.end()) {
    lcidIt->second->ReceivePdu(rxPduParams);
  }
}

void NoOpComponentCarrierManager::DoNotifyHarqDeliveryFailure() {
  NS_LOG_FUNCTION(this);
}

void NoOpComponentCarrierManager::DoReportUeMeas(
    uint16_t rnti, LteRrcSap::MeasResults measResults) {
  NS_LOG_FUNCTION(this << rnti << (uint16_t)measResults.measId);
}

void NoOpComponentCarrierManager::DoAddUe(uint16_t rnti, uint8_t state) {
  NS_LOG_FUNCTION(this << rnti << (uint16_t)state);
  auto ueInfoIt = m_ueInfo.find(rnti);
  if (ueInfoIt == m_ueInfo.end()) {
    NS_LOG_DEBUG(this << " UE " << rnti
                      << " was not found, now it is added in the map");
    UeInfo info;
    info.m_ueState = state;

    info.m_enabledComponentCarrier = 1;
    m_ueInfo.emplace(rnti, info);
  } else {
    NS_LOG_DEBUG(this << " UE " << rnti << "found, updating the state from "
                      << +ueInfoIt->second.m_ueState << " to " << +state);
    ueInfoIt->second.m_ueState = state;
  }
}

void NoOpComponentCarrierManager::DoAddLc(LteEnbCmacSapProvider::LcInfo lcInfo,
                                          LteMacSapUser *msu) {
  NS_LOG_FUNCTION(this);
  m_ueInfo.at(lcInfo.rnti).m_rlcLcInstantiated.emplace(lcInfo.lcId, lcInfo);
}

void NoOpComponentCarrierManager::DoRemoveUe(uint16_t rnti) {
  NS_LOG_FUNCTION(this);
  auto rntiIt = m_ueInfo.find(rnti);
  NS_ASSERT_MSG(rntiIt != m_ueInfo.end(),
                "request to remove UE info with unknown RNTI " << rnti);
  m_ueInfo.erase(rntiIt);
}

std::vector<LteCcmRrcSapProvider::LcsConfig>
NoOpComponentCarrierManager::DoSetupDataRadioBearer(EpsBearer bearer,
                                                    uint8_t bearerId,
                                                    uint16_t rnti, uint8_t lcid,
                                                    uint8_t lcGroup,
                                                    LteMacSapUser *msu) {
  NS_LOG_FUNCTION(this << rnti);
  auto rntiIt = m_ueInfo.find(rnti);
  NS_ASSERT_MSG(rntiIt != m_ueInfo.end(),
                "SetupDataRadioBearer on unknown RNTI " << rnti);

  rntiIt->second.m_enabledComponentCarrier = m_noOfComponentCarriers;

  std::vector<LteCcmRrcSapProvider::LcsConfig> res;
  LteCcmRrcSapProvider::LcsConfig entry;
  LteEnbCmacSapProvider::LcInfo lcinfo;
  for (uint16_t ncc = 0; ncc < m_noOfComponentCarriers; ncc++) {
    LteEnbCmacSapProvider::LcInfo lci;
    lci.rnti = rnti;
    lci.lcId = lcid;
    lci.lcGroup = lcGroup;
    lci.qci = bearer.qci;
    if (ncc == 0) {
      lci.resourceType = bearer.GetResourceType();
      lci.mbrUl = bearer.gbrQosInfo.mbrUl;
      lci.mbrDl = bearer.gbrQosInfo.mbrDl;
      lci.gbrUl = bearer.gbrQosInfo.gbrUl;
      lci.gbrDl = bearer.gbrQosInfo.gbrDl;
    } else {
      lci.resourceType = 0;
      lci.mbrUl = 0;
      lci.mbrDl = 0;
      lci.gbrUl = 0;
      lci.gbrDl = 0;
    }
    NS_LOG_DEBUG(this << " RNTI " << lci.rnti << "Lcid " << (uint16_t)lci.lcId
                      << " lcGroup " << (uint16_t)lci.lcGroup);
    entry.componentCarrierId = ncc;
    entry.lc = lci;
    entry.msu = m_ccmMacSapUser;
    res.push_back(entry);
  }

  auto lcidIt = rntiIt->second.m_rlcLcInstantiated.find(lcid);
  if (lcidIt == rntiIt->second.m_rlcLcInstantiated.end()) {
    lcinfo.rnti = rnti;
    lcinfo.lcId = lcid;
    lcinfo.lcGroup = lcGroup;
    lcinfo.qci = bearer.qci;
    lcinfo.resourceType = bearer.GetResourceType();
    lcinfo.mbrUl = bearer.gbrQosInfo.mbrUl;
    lcinfo.mbrDl = bearer.gbrQosInfo.mbrDl;
    lcinfo.gbrUl = bearer.gbrQosInfo.gbrUl;
    lcinfo.gbrDl = bearer.gbrQosInfo.gbrDl;
    rntiIt->second.m_rlcLcInstantiated.emplace(lcinfo.lcId, lcinfo);
    rntiIt->second.m_ueAttached.emplace(lcinfo.lcId, msu);
  } else {
    NS_LOG_ERROR("LC already exists");
  }
  return res;
}

std::vector<uint8_t>
NoOpComponentCarrierManager::DoReleaseDataRadioBearer(uint16_t rnti,
                                                      uint8_t lcid) {
  NS_LOG_FUNCTION(this << rnti << +lcid);

  auto rntiIt = m_ueInfo.find(rnti);
  NS_ASSERT_MSG(rntiIt != m_ueInfo.end(),
                "request to Release Data Radio Bearer on UE with unknown RNTI "
                    << rnti);

  NS_LOG_DEBUG(this << " remove LCID " << +lcid << " for RNTI " << rnti);
  std::vector<uint8_t> res;
  for (uint16_t i = 0; i < rntiIt->second.m_enabledComponentCarrier; i++) {
    res.insert(res.end(), i);
  }

  auto lcIt = rntiIt->second.m_ueAttached.find(lcid);
  NS_ASSERT_MSG(lcIt != rntiIt->second.m_ueAttached.end(),
                "Logical Channel not found");
  rntiIt->second.m_ueAttached.erase(lcIt);

  auto rlcInstancesIt = rntiIt->second.m_rlcLcInstantiated.find(rnti);
  NS_ASSERT_MSG(rlcInstancesIt != rntiIt->second.m_rlcLcInstantiated.end(),
                "Logical Channel not found");
  rntiIt->second.m_rlcLcInstantiated.erase(rlcInstancesIt);

  return res;
}

LteMacSapUser *NoOpComponentCarrierManager::DoConfigureSignalBearer(
    LteEnbCmacSapProvider::LcInfo lcinfo, LteMacSapUser *msu) {
  NS_LOG_FUNCTION(this);

  auto rntiIt = m_ueInfo.find(lcinfo.rnti);
  NS_ASSERT_MSG(rntiIt != m_ueInfo.end(),
                "request to add a signal bearer to unknown RNTI "
                    << lcinfo.rnti);

  auto lcidIt = rntiIt->second.m_ueAttached.find(lcinfo.lcId);
  if (lcidIt == rntiIt->second.m_ueAttached.end()) {
    rntiIt->second.m_ueAttached.emplace(lcinfo.lcId, msu);
  } else {
    NS_LOG_ERROR("LC already exists");
  }

  return m_ccmMacSapUser;
}

void NoOpComponentCarrierManager::DoNotifyPrbOccupancy(
    double prbOccupancy, uint8_t componentCarrierId) {
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("Update PRB occupancy:" << prbOccupancy << " at carrier:"
                                       << (uint32_t)componentCarrierId);
  m_ccPrbOccupancy.insert(
      std::pair<uint8_t, double>(componentCarrierId, prbOccupancy));
}

void NoOpComponentCarrierManager::DoUlReceiveMacCe(MacCeListElement_s bsr,
                                                   uint8_t componentCarrierId) {
  NS_LOG_FUNCTION(this);
  NS_ASSERT_MSG(bsr.m_macCeType == MacCeListElement_s::BSR,
                "Received a Control Message not allowed " << bsr.m_macCeType);
  if (bsr.m_macCeType == MacCeListElement_s::BSR) {
    MacCeListElement_s newBsr;
    newBsr.m_rnti = bsr.m_rnti;
    newBsr.m_macCeType = bsr.m_macCeType;
    newBsr.m_macCeValue.m_phr = bsr.m_macCeValue.m_phr;
    newBsr.m_macCeValue.m_crnti = bsr.m_macCeValue.m_crnti;
    newBsr.m_macCeValue.m_bufferStatus.resize(4);
    for (uint16_t i = 0; i < 4; i++) {
      uint8_t bsrId = bsr.m_macCeValue.m_bufferStatus.at(i);
      uint32_t buffer = BufferSizeLevelBsr::BsrId2BufferSize(bsrId);
      newBsr.m_macCeValue.m_bufferStatus.at(i) =
          BufferSizeLevelBsr::BufferSize2BsrId(buffer);
    }
    auto sapIt = m_ccmMacSapProviderMap.find(componentCarrierId);
    if (sapIt == m_ccmMacSapProviderMap.end()) {
      NS_FATAL_ERROR("Sap not found in the CcmMacSapProviderMap");
    } else {
      sapIt->second->ReportMacCeToScheduler(newBsr);
    }
  } else {
    NS_FATAL_ERROR("Expected BSR type of message.");
  }
}

void NoOpComponentCarrierManager::DoUlReceiveSr(uint16_t rnti,
                                                uint8_t componentCarrierId) {
  NS_LOG_FUNCTION(this);

  auto sapIt = m_ccmMacSapProviderMap.find(componentCarrierId);
  NS_ABORT_MSG_IF(sapIt == m_ccmMacSapProviderMap.end(),
                  "Sap not found in the CcmMacSapProviderMap");

  sapIt->second->ReportSrToScheduler(rnti);
}

NS_OBJECT_ENSURE_REGISTERED(RrComponentCarrierManager);

RrComponentCarrierManager::RrComponentCarrierManager() {
  NS_LOG_FUNCTION(this);
}

RrComponentCarrierManager::~RrComponentCarrierManager() {
  NS_LOG_FUNCTION(this);
}

TypeId RrComponentCarrierManager::GetTypeId() {
  static TypeId tid = TypeId("ns3::RrComponentCarrierManager")
                          .SetParent<NoOpComponentCarrierManager>()
                          .SetGroupName("Lte")
                          .AddConstructor<RrComponentCarrierManager>();
  return tid;
}

void RrComponentCarrierManager::DoReportBufferStatus(
    LteMacSapProvider::ReportBufferStatusParameters params) {
  NS_LOG_FUNCTION(this);

  uint32_t numberOfCarriersForUe =
      m_ueInfo.at(params.rnti).m_enabledComponentCarrier;
  if (params.lcid == 0 || params.lcid == 1 || numberOfCarriersForUe == 1) {
    NS_LOG_INFO("Buffer status forwarded to the primary carrier.");
    auto ueManager = m_ccmRrcSapUser->GetUeManager(params.rnti);
    m_macSapProvidersMap.at(ueManager->GetComponentCarrierId())
        ->ReportBufferStatus(params);
  } else {
    params.retxQueueSize /= numberOfCarriersForUe;
    params.txQueueSize /= numberOfCarriersForUe;
    for (uint32_t i = 0; i < numberOfCarriersForUe; i++) {
      NS_ASSERT_MSG(m_macSapProvidersMap.find(i) != m_macSapProvidersMap.end(),
                    "Mac sap provider does not exist.");
      m_macSapProvidersMap.at(i)->ReportBufferStatus(params);
    }
  }
}

void RrComponentCarrierManager::DoUlReceiveMacCe(MacCeListElement_s bsr,
                                                 uint8_t componentCarrierId) {
  NS_LOG_FUNCTION(this);
  NS_ASSERT_MSG(
      componentCarrierId == 0,
      "Received BSR from a ComponentCarrier not allowed, ComponentCarrierId = "
          << componentCarrierId);
  NS_ASSERT_MSG(bsr.m_macCeType == MacCeListElement_s::BSR,
                "Received a Control Message not allowed " << bsr.m_macCeType);

  uint32_t numberOfCarriersForUe =
      m_ueInfo.at(bsr.m_rnti).m_enabledComponentCarrier;

  if (bsr.m_macCeType == MacCeListElement_s::BSR) {
    MacCeListElement_s newBsr;
    newBsr.m_rnti = bsr.m_rnti;
    newBsr.m_macCeType = bsr.m_macCeType;
    newBsr.m_macCeValue.m_phr = bsr.m_macCeValue.m_phr;
    newBsr.m_macCeValue.m_crnti = bsr.m_macCeValue.m_crnti;
    newBsr.m_macCeValue.m_bufferStatus.resize(4);
    for (uint16_t i = 0; i < 4; i++) {
      uint8_t bsrStatusId = bsr.m_macCeValue.m_bufferStatus.at(i);
      uint32_t bufferSize = BufferSizeLevelBsr::BsrId2BufferSize(bsrStatusId);
      newBsr.m_macCeValue.m_bufferStatus.at(i) =
          BufferSizeLevelBsr::BufferSize2BsrId(bufferSize /
                                               numberOfCarriersForUe);
    }
    for (uint32_t i = 0; i < numberOfCarriersForUe; i++) {
      NS_ASSERT_MSG(m_ccmMacSapProviderMap.find(i) !=
                        m_ccmMacSapProviderMap.end(),
                    "Mac sap provider does not exist.");
      m_ccmMacSapProviderMap.find(i)->second->ReportMacCeToScheduler(newBsr);
    }
  } else {
    auto ueManager = m_ccmRrcSapUser->GetUeManager(bsr.m_rnti);
    m_ccmMacSapProviderMap.at(ueManager->GetComponentCarrierId())
        ->ReportMacCeToScheduler(bsr);
  }
}

void RrComponentCarrierManager::DoUlReceiveSr(uint16_t rnti, uint8_t) {
  NS_LOG_FUNCTION(this);
  uint32_t numberOfCarriersForUe = m_ueInfo.at(rnti).m_enabledComponentCarrier;

  m_ccmMacSapProviderMap.find(m_lastCcIdForSr)
      ->second->ReportSrToScheduler(rnti);

  m_lastCcIdForSr++;
  if (m_lastCcIdForSr > numberOfCarriersForUe - 1) {
    m_lastCcIdForSr = 0;
  }
}

} // namespace ns3
