
#ifndef EPC_UE_NAS_H
#define EPC_UE_NAS_H

#include "epc-tft-classifier.h"
#include "eps-bearer.h"
#include "lte-as-sap.h"

#include <ns3/object.h>
#include <ns3/traced-callback.h>

namespace ns3 {

class EpcHelper;
class NetDevice;

class EpcUeNas : public Object {
  friend class MemberLteAsSapUser<EpcUeNas>;

public:
  EpcUeNas();

  ~EpcUeNas() override;

  void DoDispose() override;
  static TypeId GetTypeId();

  void SetDevice(Ptr<NetDevice> dev);

  void SetImsi(uint64_t imsi);

  void SetCsgId(uint32_t csgId);

  uint32_t GetCsgId() const;

  void SetAsSapProvider(LteAsSapProvider *s);

  LteAsSapUser *GetAsSapUser();

  void SetForwardUpCallback(Callback<void, Ptr<Packet>> cb);

  void StartCellSelection(uint32_t dlEarfcn);

  void Connect();

  void Connect(uint16_t cellId, uint32_t dlEarfcn);

  void Disconnect();

  void ActivateEpsBearer(EpsBearer bearer, Ptr<EpcTft> tft);

  bool Send(Ptr<Packet> p, uint16_t protocolNumber);

  enum State {
    OFF = 0,
    ATTACHING,
    IDLE_REGISTERED,
    CONNECTING_TO_EPC,
    ACTIVE,
    NUM_STATES
  };

  State GetState() const;

  typedef void (*StateTracedCallback)(const State oldState,
                                      const State newState);

private:
  void DoNotifyConnectionSuccessful();
  void DoNotifyConnectionFailed();
  void DoNotifyConnectionReleased();
  void DoRecvData(Ptr<Packet> packet);

  void DoActivateEpsBearer(EpsBearer bearer, Ptr<EpcTft> tft);
  void SwitchToState(State s);

  State m_state;

  TracedCallback<State, State> m_stateTransitionCallback;

  Ptr<NetDevice> m_device;

  uint64_t m_imsi;

  uint32_t m_csgId;

  LteAsSapProvider *m_asSapProvider;
  LteAsSapUser *m_asSapUser;

  uint8_t m_bidCounter;
  EpcTftClassifier m_tftClassifier;

  Callback<void, Ptr<Packet>> m_forwardUpCallback;

  struct BearerToBeActivated {
    EpsBearer bearer;
    Ptr<EpcTft> tft;
  };

  std::list<BearerToBeActivated> m_bearersToBeActivatedList;

  std::list<BearerToBeActivated> m_bearersToBeActivatedListForReconnection;
};

} // namespace ns3

#endif
