
#ifndef LTE_UE_CCM_RRC_SAP_H
#define LTE_UE_CCM_RRC_SAP_H

#include "lte-mac-sap.h"
#include "lte-ue-cmac-sap.h"

#include <map>

namespace ns3 {
class LteUeCmacSapProvider;
class LteMacSapUser;

class LteUeCcmRrcSapProvider {
  friend class LteMacSapUser;

public:
  virtual ~LteUeCcmRrcSapProvider();

  struct LcsConfig {
    uint8_t componentCarrierId;
    LteUeCmacSapProvider::LogicalChannelConfig lcConfig;
    LteMacSapUser *msu;
  };

  virtual std::vector<LteUeCcmRrcSapProvider::LcsConfig>
  AddLc(uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
        LteMacSapUser *msu) = 0;

  virtual std::vector<uint16_t> RemoveLc(uint8_t lcid) = 0;
  virtual void Reset() = 0;
  virtual void NotifyConnectionReconfigurationMsg() = 0;

  virtual LteMacSapUser *
  ConfigureSignalBearer(uint8_t lcid,
                        LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
                        LteMacSapUser *msu) = 0;
};

template <class C>
class MemberLteUeCcmRrcSapProvider : public LteUeCcmRrcSapProvider {
public:
  MemberLteUeCcmRrcSapProvider(C *owner);

  std::vector<uint16_t> RemoveLc(uint8_t lcid) override;
  void Reset() override;
  std::vector<LteUeCcmRrcSapProvider::LcsConfig>
  AddLc(uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
        LteMacSapUser *msu) override;
  void NotifyConnectionReconfigurationMsg() override;
  LteMacSapUser *
  ConfigureSignalBearer(uint8_t lcid,
                        LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
                        LteMacSapUser *msu) override;

private:
  C *m_owner;
};

template <class C>
MemberLteUeCcmRrcSapProvider<C>::MemberLteUeCcmRrcSapProvider(C *owner)
    : m_owner(owner) {}

template <class C>
std::vector<uint16_t> MemberLteUeCcmRrcSapProvider<C>::RemoveLc(uint8_t lcid) {
  return m_owner->DoRemoveLc(lcid);
}

template <class C> void MemberLteUeCcmRrcSapProvider<C>::Reset() {
  return m_owner->DoReset();
}

template <class C>
std::vector<LteUeCcmRrcSapProvider::LcsConfig>
MemberLteUeCcmRrcSapProvider<C>::AddLc(
    uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
    LteMacSapUser *msu) {
  return m_owner->DoAddLc(lcId, lcConfig, msu);
}

template <class C>
void MemberLteUeCcmRrcSapProvider<C>::NotifyConnectionReconfigurationMsg() {
  NS_FATAL_ERROR(
      "Function should not be called because it is not implemented.");
}

template <class C>
LteMacSapUser *MemberLteUeCcmRrcSapProvider<C>::ConfigureSignalBearer(
    uint8_t lcid, LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
    LteMacSapUser *msu) {
  return m_owner->DoConfigureSignalBearer(lcid, lcConfig, msu);
}

class LteUeCcmRrcSapUser {
public:
  virtual ~LteUeCcmRrcSapUser();

  virtual void
  ComponentCarrierEnabling(std::vector<uint8_t> componentCarrierList) = 0;
  virtual void SetNumberOfComponentCarriers(uint16_t noOfComponentCarriers) = 0;
};

template <class C> class MemberLteUeCcmRrcSapUser : public LteUeCcmRrcSapUser {
public:
  MemberLteUeCcmRrcSapUser(C *owner);
  void
  ComponentCarrierEnabling(std::vector<uint8_t> componentCarrierList) override;
  void SetNumberOfComponentCarriers(uint16_t noOfComponentCarriers) override;

private:
  C *m_owner;
};

template <class C>
MemberLteUeCcmRrcSapUser<C>::MemberLteUeCcmRrcSapUser(C *owner)
    : m_owner(owner) {}

template <class C>
void MemberLteUeCcmRrcSapUser<C>::ComponentCarrierEnabling(
    std::vector<uint8_t> componentCarrierList) {
  NS_FATAL_ERROR(
      "Function should not be called because it is not implemented.");
}

template <class C>
void MemberLteUeCcmRrcSapUser<C>::SetNumberOfComponentCarriers(
    uint16_t noOfComponentCarriers) {
  m_owner->DoSetNumberOfComponentCarriers(noOfComponentCarriers);
}

} // namespace ns3

#endif
