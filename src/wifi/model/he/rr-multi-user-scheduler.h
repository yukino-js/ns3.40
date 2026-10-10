
#ifndef RR_MULTI_USER_SCHEDULER_H
#define RR_MULTI_USER_SCHEDULER_H

#include "multi-user-scheduler.h"

#include <list>

namespace ns3 {

class RrMultiUserScheduler : public MultiUserScheduler {
public:
  static TypeId GetTypeId();
  RrMultiUserScheduler();
  ~RrMultiUserScheduler() override;

protected:
  void DoDispose() override;
  void DoInitialize() override;

private:
  TxFormat SelectTxFormat() override;
  DlMuInfo ComputeDlMuInfo() override;
  UlMuInfo ComputeUlMuInfo() override;

  virtual TxFormat TrySendingBsrpTf();

  virtual TxFormat TrySendingBasicTf();

  virtual TxFormat TrySendingDlMuPpdu();

  template <class Func> WifiTxVector GetTxVectorForUlMu(Func canBeSolicited);

  void NotifyStationAssociated(uint16_t aid, Mac48Address address);
  void NotifyStationDeassociated(uint16_t aid, Mac48Address address);

  struct MasterInfo {
    uint16_t aid;
    Mac48Address address;
    double credits;
  };

  void FinalizeTxVector(WifiTxVector &txVector);
  void UpdateCredits(std::list<MasterInfo> &staList, Time txDuration,
                     const WifiTxVector &txVector);

  typedef std::pair<std::list<MasterInfo>::iterator, Ptr<WifiMpdu>>
      CandidateInfo;

  uint8_t m_nStations;
  bool m_enableTxopSharing;
  bool m_forceDlOfdma;
  bool m_enableUlOfdma;
  bool m_enableBsrp;
  bool m_useCentral26TonesRus;
  uint32_t m_ulPsduSize;
  std::map<AcIndex, std::list<MasterInfo>> m_staListDl;
  std::list<MasterInfo> m_staListUl;
  std::list<CandidateInfo> m_candidates;
  Time m_maxCredits;
  CtrlTriggerHeader m_trigger;
  WifiMacHeader m_triggerMacHdr;
  WifiTxParameters m_txParams;
};

} // namespace ns3

#endif
