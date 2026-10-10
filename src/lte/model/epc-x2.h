
#ifndef EPC_X2_H
#define EPC_X2_H

#include "epc-x2-sap.h"

#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"

#include <map>

namespace ns3 {

class X2IfaceInfo : public SimpleRefCount<X2IfaceInfo> {
public:
  X2IfaceInfo(Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket,
              Ptr<Socket> localUserPlaneSocket);
  virtual ~X2IfaceInfo();

  X2IfaceInfo &operator=(const X2IfaceInfo &value);

public:
  Ipv4Address m_remoteIpAddr;
  Ptr<Socket> m_localCtrlPlaneSocket;
  Ptr<Socket> m_localUserPlaneSocket;
};

class X2CellInfo : public SimpleRefCount<X2CellInfo> {
public:
  X2CellInfo(std::vector<uint16_t> localCellIds,
             std::vector<uint16_t> remoteCellIds);
  virtual ~X2CellInfo();

  X2CellInfo &operator=(const X2CellInfo &value);

public:
  std::vector<uint16_t> m_localCellIds;
  std::vector<uint16_t> m_remoteCellIds;
};

class EpcX2 : public Object {
  friend class EpcX2SpecificEpcX2SapProvider<EpcX2>;

public:
  EpcX2();

  ~EpcX2() override;

  static TypeId GetTypeId();
  void DoDispose() override;

  void SetEpcX2SapUser(EpcX2SapUser *s);

  EpcX2SapProvider *GetEpcX2SapProvider();

  void AddX2Interface(uint16_t enb1CellId, Ipv4Address enb1X2Address,
                      std::vector<uint16_t> enb2CellIds,
                      Ipv4Address enb2X2Address);

  void RecvFromX2cSocket(Ptr<Socket> socket);

  void RecvFromX2uSocket(Ptr<Socket> socket);

protected:
  virtual void
  DoSendHandoverRequest(EpcX2SapProvider::HandoverRequestParams params);
  virtual void
  DoSendHandoverRequestAck(EpcX2SapProvider::HandoverRequestAckParams params);
  virtual void DoSendHandoverPreparationFailure(
      EpcX2SapProvider::HandoverPreparationFailureParams params);
  virtual void
  DoSendSnStatusTransfer(EpcX2SapProvider::SnStatusTransferParams params);
  virtual void
  DoSendUeContextRelease(EpcX2SapProvider::UeContextReleaseParams params);
  virtual void
  DoSendLoadInformation(EpcX2SapProvider::LoadInformationParams params);
  virtual void DoSendResourceStatusUpdate(
      EpcX2SapProvider::ResourceStatusUpdateParams params);
  virtual void DoSendUeData(EpcX2SapProvider::UeDataParams params);
  virtual void
  DoSendHandoverCancel(EpcX2SapProvider::HandoverCancelParams params);

  EpcX2SapUser *m_x2SapUser;
  EpcX2SapProvider *m_x2SapProvider;

private:
  std::map<uint16_t, Ptr<X2IfaceInfo>> m_x2InterfaceSockets;

  std::map<Ptr<Socket>, Ptr<X2CellInfo>> m_x2InterfaceCellIds;

  uint16_t m_x2cUdpPort;
  uint16_t m_x2uUdpPort;
};

} // namespace ns3

#endif
