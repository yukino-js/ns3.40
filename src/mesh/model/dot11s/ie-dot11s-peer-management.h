
#ifndef IE_DOT11S_PEER_MANAGEMENT_H
#define IE_DOT11S_PEER_MANAGEMENT_H

#include "ns3/mesh-information-element-vector.h"

namespace ns3 {
namespace dot11s {

enum PmpReasonCode {
  REASON11S_PEERING_CANCELLED = 52,
  REASON11S_MESH_MAX_PEERS = 53,
  REASON11S_MESH_CAPABILITY_POLICY_VIOLATION = 54,
  REASON11S_MESH_CLOSE_RCVD = 55,
  REASON11S_MESH_MAX_RETRIES = 56,
  REASON11S_MESH_CONFIRM_TIMEOUT = 57,
  REASON11S_MESH_INVALID_GTK = 58,
  REASON11S_MESH_INCONSISTENT_PARAMETERS = 59,
  REASON11S_MESH_INVALID_SECURITY_CAPABILITY = 60,
  REASON11S_RESERVED = 67,
};

class IePeerManagement : public WifiInformationElement {
public:
  IePeerManagement();

  enum Subtype {
    PEER_OPEN = 1,
    PEER_CONFIRM = 2,
    PEER_CLOSE = 3,
  };

  void SetPeerOpen(uint16_t localLinkId);
  void SetPeerClose(uint16_t localLinkID, uint16_t peerLinkId,
                    PmpReasonCode reasonCode);
  void SetPeerConfirm(uint16_t localLinkID, uint16_t peerLinkId);

  PmpReasonCode GetReasonCode() const;
  uint16_t GetLocalLinkId() const;
  uint16_t GetPeerLinkId() const;
  bool SubtypeIsOpen() const;
  bool SubtypeIsClose() const;
  bool SubtypeIsConfirm() const;
  uint8_t GetSubtype() const;

  WifiInformationElementId ElementId() const override;
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator i) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator i,
                                       uint16_t length) override;
  void Print(std::ostream &os) const override;

private:
  uint8_t m_length;
  uint8_t m_subtype;
  uint16_t m_localLinkId;
  uint16_t m_peerLinkId;
  PmpReasonCode m_reasonCode;
  friend bool operator==(const IePeerManagement &a, const IePeerManagement &b);
};

bool operator==(const IePeerManagement &a, const IePeerManagement &b);
std::ostream &operator<<(std::ostream &os, const IePeerManagement &peerMan);
} // namespace dot11s
} // namespace ns3

#endif
