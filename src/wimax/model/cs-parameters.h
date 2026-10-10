
#ifndef WIMAX_CS_PARAMETERS_H
#define WIMAX_CS_PARAMETERS_H

#include "ipcs-classifier-record.h"
#include "wimax-tlv.h"

namespace ns3 {

class CsParameters {
public:
  enum Action { ADD = 0, REPLACE = 1, DELETE = 2 };

  CsParameters();
  ~CsParameters();
  CsParameters(Tlv tlv);
  CsParameters(Action classifierDscAction, IpcsClassifierRecord classifier);
  void SetClassifierDscAction(Action action);
  void SetPacketClassifierRule(IpcsClassifierRecord packetClassifierRule);
  Action GetClassifierDscAction() const;
  IpcsClassifierRecord GetPacketClassifierRule() const;
  Tlv ToTlv() const;

private:
  Action m_classifierDscAction;
  IpcsClassifierRecord m_packetClassifierRule;
};

} // namespace ns3
#endif
