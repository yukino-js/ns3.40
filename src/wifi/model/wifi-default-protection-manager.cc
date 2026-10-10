
#include "wifi-default-protection-manager.h"

#include "ap-wifi-mac.h"
#include "frame-exchange-manager.h"
#include "sta-wifi-mac.h"
#include "wifi-mpdu.h"
#include "wifi-tx-parameters.h"

#include "ns3/boolean.h"
#include "ns3/emlsr-manager.h"
#include "ns3/erp-ofdm-phy.h"
#include "ns3/log.h"

#include <type_traits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("WifiDefaultProtectionManager");

NS_OBJECT_ENSURE_REGISTERED(WifiDefaultProtectionManager);

TypeId WifiDefaultProtectionManager::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::WifiDefaultProtectionManager")
          .SetParent<WifiProtectionManager>()
          .SetGroupName("Wifi")
          .AddConstructor<WifiDefaultProtectionManager>()
          .AddAttribute(
              "EnableMuRts",
              "If enabled, always protect a DL/UL MU frame exchange with "
              "MU-RTS/CTS.",
              BooleanValue(false),
              MakeBooleanAccessor(&WifiDefaultProtectionManager::m_sendMuRts),
              MakeBooleanChecker());
  return tid;
}

WifiDefaultProtectionManager::WifiDefaultProtectionManager() {
  NS_LOG_FUNCTION(this);
}

WifiDefaultProtectionManager::~WifiDefaultProtectionManager() {
  NS_LOG_FUNCTION_NOARGS();
}

std::unique_ptr<WifiProtection>
WifiDefaultProtectionManager::TryAddMpdu(Ptr<const WifiMpdu> mpdu,
                                         const WifiTxParameters &txParams) {
  NS_LOG_FUNCTION(this << *mpdu << &txParams);

  const auto &psduInfoMap = txParams.GetPsduInfoMap();
  auto dlMuPpdu =
      txParams.m_txVector.IsDlMu() &&
      (psduInfoMap.size() > 1 ||
       (psduInfoMap.size() == 1 &&
        psduInfoMap.begin()->first != mpdu->GetHeader().GetAddr1()));
  auto isEmlsrDestination = GetWifiRemoteStationManager()->GetEmlsrEnabled(
      mpdu->GetHeader().GetAddr1());

  if (dlMuPpdu || isEmlsrDestination ||
      (txParams.m_protection &&
       txParams.m_protection->method == WifiProtection::MU_RTS_CTS)) {
    return TryAddMpduToMuPpdu(mpdu, txParams);
  }

  if (txParams.m_txVector.IsUlMu()) {
    if (txParams.m_protection) {
      NS_ASSERT(txParams.m_protection->method == WifiProtection::NONE);
      return nullptr;
    }
    return std::make_unique<WifiNoProtection>();
  }

  if (mpdu->GetHeader().IsTrigger()) {
    return TryUlMuTransmission(mpdu, txParams);
  }

  if (txParams.m_protection &&
      (txParams.m_protection->method == WifiProtection::RTS_CTS ||
       txParams.m_protection->method == WifiProtection::CTS_TO_SELF)) {
    return nullptr;
  }

  NS_ASSERT(!txParams.m_protection ||
            txParams.m_protection->method == WifiProtection::NONE);

  std::unique_ptr<WifiProtection> protection;
  protection = GetPsduProtection(
      mpdu->GetHeader(), txParams.GetSizeIfAddMpdu(mpdu), txParams.m_txVector);

  if (!txParams.m_protection || protection->method != WifiProtection::NONE) {
    return protection;
  }
  return nullptr;
}

std::unique_ptr<WifiProtection> WifiDefaultProtectionManager::TryAggregateMsdu(
    Ptr<const WifiMpdu> msdu, const WifiTxParameters &txParams) {
  NS_LOG_FUNCTION(this << *msdu << &txParams);

  NS_ASSERT(txParams.m_protection);
  if (txParams.m_protection->method == WifiProtection::RTS_CTS ||
      txParams.m_protection->method == WifiProtection::CTS_TO_SELF ||
      txParams.m_protection->method == WifiProtection::MU_RTS_CTS) {
    return nullptr;
  }

  NS_ASSERT(txParams.m_protection->method == WifiProtection::NONE);

  if (txParams.m_txVector.IsUlMu() ||
      (txParams.m_txVector.IsDlMu() && txParams.GetPsduInfoMap().size() > 1)) {
    return nullptr;
  }

  std::unique_ptr<WifiProtection> protection;
  protection = GetPsduProtection(msdu->GetHeader(),
                                 txParams.GetSizeIfAggregateMsdu(msdu).second,
                                 txParams.m_txVector);

  if (protection->method == WifiProtection::NONE) {
    return nullptr;
  }

  return protection;
}

std::unique_ptr<WifiProtection> WifiDefaultProtectionManager::GetPsduProtection(
    const WifiMacHeader &hdr, uint32_t size,
    const WifiTxVector &txVector) const {
  NS_LOG_FUNCTION(this << hdr << size << txVector);

  if (hdr.GetFragmentNumber() > 0 && !hdr.IsRetry()) {
    return std::make_unique<WifiNoProtection>();
  }

  if (m_mac->GetFrameExchangeManager(m_linkId)->GetProtectedStas().count(
          hdr.GetAddr1()) == 1) {
    return std::make_unique<WifiNoProtection>();
  }

  bool emlsrNeedRts = false;

  if (auto staMac = DynamicCast<StaWifiMac>(m_mac)) {
    auto emlsrManager = staMac->GetEmlsrManager();

    emlsrNeedRts =
        emlsrManager && staMac->IsEmlsrLink(m_linkId) &&
        m_mac->GetLinkForPhy(emlsrManager->GetMainPhyId()) != m_linkId;
  }

  if (emlsrNeedRts || GetWifiRemoteStationManager()->NeedRts(hdr, size)) {
    auto protection = std::make_unique<WifiRtsCtsProtection>();
    protection->rtsTxVector =
        GetWifiRemoteStationManager()->GetRtsTxVector(hdr.GetAddr1());
    protection->ctsTxVector = GetWifiRemoteStationManager()->GetCtsTxVector(
        hdr.GetAddr1(), protection->rtsTxVector.GetMode());
    return protection;
  }

  if (GetWifiRemoteStationManager()->GetUseNonErpProtection() &&
      GetWifiRemoteStationManager()->NeedCtsToSelf(txVector)) {
    auto protection = std::make_unique<WifiCtsToSelfProtection>();
    protection->ctsTxVector =
        GetWifiRemoteStationManager()->GetCtsToSelfTxVector();
    return protection;
  }

  return std::make_unique<WifiNoProtection>();
}

std::unique_ptr<WifiProtection>
WifiDefaultProtectionManager::TryAddMpduToMuPpdu(
    Ptr<const WifiMpdu> mpdu, const WifiTxParameters &txParams) {
  NS_LOG_FUNCTION(this << *mpdu << &txParams);

  auto receiver = mpdu->GetHeader().GetAddr1();
  const auto &psduInfoMap = txParams.GetPsduInfoMap();
  auto dlMuPpdu =
      txParams.m_txVector.IsDlMu() &&
      (psduInfoMap.size() > 1 ||
       (psduInfoMap.size() == 1 && psduInfoMap.begin()->first != receiver));
  auto isEmlsrDestination =
      GetWifiRemoteStationManager()->GetEmlsrEnabled(receiver);
  NS_ASSERT(dlMuPpdu || isEmlsrDestination ||
            (txParams.m_protection &&
             txParams.m_protection->method == WifiProtection::MU_RTS_CTS));

  auto isProtected =
      m_mac->GetFrameExchangeManager(m_linkId)->GetProtectedStas().count(
          receiver) == 1;
  bool needMuRts = (txParams.m_protection && txParams.m_protection->method ==
                                                 WifiProtection::MU_RTS_CTS) ||
                   (dlMuPpdu && m_sendMuRts && !isProtected) ||
                   (isEmlsrDestination && !isProtected);

  if (!needMuRts) {
    if (txParams.m_protection &&
        txParams.m_protection->method == WifiProtection::NONE) {
      return nullptr;
    }
    return std::make_unique<WifiNoProtection>();
  }

  WifiMuRtsCtsProtection *protection = nullptr;
  if (txParams.m_protection &&
      txParams.m_protection->method == WifiProtection::MU_RTS_CTS) {
    protection =
        static_cast<WifiMuRtsCtsProtection *>(txParams.m_protection.get());
  }

  if (txParams.GetPsduInfo(receiver) == nullptr) {
    NS_ABORT_MSG_IF(m_mac->GetTypeOfStation() != AP,
                    "HE APs only can send DL MU PPDUs");
    auto apMac = StaticCast<ApWifiMac>(m_mac);
    auto modClass = txParams.m_txVector.GetModulationClass();
    auto txWidth =
        modClass == WIFI_MOD_CLASS_DSSS || modClass == WIFI_MOD_CLASS_HR_DSSS
            ? 20
            : txParams.m_txVector.GetChannelWidth();

    if (protection != nullptr) {
      protection = new WifiMuRtsCtsProtection(*protection);
    } else {
      protection = new WifiMuRtsCtsProtection;

      protection->muRts.SetType(TriggerFrameType::MU_RTS_TRIGGER);
      protection->muRts.SetUlBandwidth(txWidth);

      for (const auto &[address, info] : txParams.GetPsduInfoMap()) {
        NS_ASSERT_MSG(address != receiver,
                      "This must be the first MPDU for " << receiver);
        AddUserInfoToMuRts(protection->muRts, txWidth, address);
      }

      protection->muRtsTxVector =
          GetWifiRemoteStationManager()->GetRtsTxVector(receiver);
      protection->muRtsTxVector.SetChannelWidth(txWidth);
      const auto modulation = protection->muRtsTxVector.GetModulationClass();
      if (modulation == WIFI_MOD_CLASS_DSSS ||
          modulation == WIFI_MOD_CLASS_HR_DSSS) {
        protection->muRtsTxVector.SetMode(ErpOfdmPhy::GetErpOfdmRate6Mbps());
      }
    }

    if (isEmlsrDestination && !isProtected) {
      GetWifiRemoteStationManager()->AdjustTxVectorForIcf(
          protection->muRtsTxVector);
    }

    AddUserInfoToMuRts(protection->muRts, txWidth, receiver);

    return std::unique_ptr<WifiMuRtsCtsProtection>(protection);
  }

  NS_ASSERT(protection != nullptr);

  return nullptr;
}

std::unique_ptr<WifiProtection>
WifiDefaultProtectionManager::TryUlMuTransmission(
    Ptr<const WifiMpdu> mpdu, const WifiTxParameters &txParams) {
  NS_LOG_FUNCTION(this << *mpdu << &txParams);
  NS_ASSERT(mpdu->GetHeader().IsTrigger());

  CtrlTriggerHeader trigger;
  mpdu->GetPacket()->PeekHeader(trigger);
  NS_ASSERT(trigger.GetNUserInfoFields() > 0);
  auto txWidth = trigger.GetUlBandwidth();

  auto protection = std::make_unique<WifiMuRtsCtsProtection>();
  protection->muRts.SetType(TriggerFrameType::MU_RTS_TRIGGER);
  protection->muRts.SetUlBandwidth(txWidth);

  NS_ABORT_MSG_IF(m_mac->GetTypeOfStation() != AP,
                  "HE APs only can send DL MU PPDUs");
  const auto &staList = StaticCast<ApWifiMac>(m_mac)->GetStaList(m_linkId);

  bool allProtected = true;
  bool isUnprotectedEmlsrDst = false;

  for (const auto &userInfo : trigger) {
    auto staIt = staList.find(userInfo.GetAid12());
    NS_ASSERT(staIt != staList.cend());
    AddUserInfoToMuRts(protection->muRts, txWidth, staIt->second);
    bool isProtected =
        m_mac->GetFrameExchangeManager(m_linkId)->GetProtectedStas().count(
            staIt->second) == 1;
    allProtected = allProtected && isProtected;

    isUnprotectedEmlsrDst =
        isUnprotectedEmlsrDst ||
        (!isProtected &&
         GetWifiRemoteStationManager()->GetEmlsrEnabled(staIt->second));
  }

  bool needMuRts = (m_sendMuRts && !allProtected) || isUnprotectedEmlsrDst;

  if (!needMuRts) {
    return std::make_unique<WifiNoProtection>();
  }

  protection->muRtsTxVector = GetWifiRemoteStationManager()->GetRtsTxVector(
      mpdu->GetHeader().GetAddr1());
  protection->muRtsTxVector.SetChannelWidth(txWidth);
  const auto modulation = protection->muRtsTxVector.GetModulationClass();
  if (modulation == WIFI_MOD_CLASS_DSSS ||
      modulation == WIFI_MOD_CLASS_HR_DSSS) {
    protection->muRtsTxVector.SetMode(ErpOfdmPhy::GetErpOfdmRate6Mbps());
  }
  if (isUnprotectedEmlsrDst) {
    GetWifiRemoteStationManager()->AdjustTxVectorForIcf(
        protection->muRtsTxVector);
  }

  return protection;
}

} // namespace ns3
