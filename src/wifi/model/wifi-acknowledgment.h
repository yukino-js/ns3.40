
#ifndef WIFI_ACKNOWLEDGMENT_H
#define WIFI_ACKNOWLEDGMENT_H

#include "block-ack-type.h"
#include "ctrl-headers.h"
#include "wifi-mac-header.h"
#include "wifi-tx-vector.h"

#include "ns3/nstime.h"

#include <map>
#include <memory>

namespace ns3 {

class Mac48Address;

struct WifiAcknowledgment {
  enum Method {
    NONE = 0,
    NORMAL_ACK,
    BLOCK_ACK,
    BAR_BLOCK_ACK,
    DL_MU_BAR_BA_SEQUENCE,
    DL_MU_TF_MU_BAR,
    DL_MU_AGGREGATE_TF,
    UL_MU_MULTI_STA_BA,
    ACK_AFTER_TB_PPDU
  };

  WifiAcknowledgment(Method m);
  virtual ~WifiAcknowledgment();

  virtual std::unique_ptr<WifiAcknowledgment> Copy() const = 0;

  WifiMacHeader::QosAckPolicy GetQosAckPolicy(Mac48Address receiver,
                                              uint8_t tid) const;

  void SetQosAckPolicy(Mac48Address receiver, uint8_t tid,
                       WifiMacHeader::QosAckPolicy ackPolicy);

  virtual void Print(std::ostream &os) const = 0;

  const Method method;
  Time acknowledgmentTime;

private:
  virtual bool
  CheckQosAckPolicy(Mac48Address receiver, uint8_t tid,
                    WifiMacHeader::QosAckPolicy ackPolicy) const = 0;

  std::map<std::pair<Mac48Address, uint8_t>, WifiMacHeader::QosAckPolicy>
      m_ackPolicy;
};

struct WifiNoAck : public WifiAcknowledgment {
  WifiNoAck();

  std::unique_ptr<WifiAcknowledgment> Copy() const override;
  bool CheckQosAckPolicy(Mac48Address receiver, uint8_t tid,
                         WifiMacHeader::QosAckPolicy ackPolicy) const override;
  void Print(std::ostream &os) const override;
};

struct WifiNormalAck : public WifiAcknowledgment {
  WifiNormalAck();

  std::unique_ptr<WifiAcknowledgment> Copy() const override;
  bool CheckQosAckPolicy(Mac48Address receiver, uint8_t tid,
                         WifiMacHeader::QosAckPolicy ackPolicy) const override;
  void Print(std::ostream &os) const override;

  WifiTxVector ackTxVector;
};

struct WifiBlockAck : public WifiAcknowledgment {
  WifiBlockAck();

  std::unique_ptr<WifiAcknowledgment> Copy() const override;
  bool CheckQosAckPolicy(Mac48Address receiver, uint8_t tid,
                         WifiMacHeader::QosAckPolicy ackPolicy) const override;
  void Print(std::ostream &os) const override;

  WifiTxVector blockAckTxVector;
  BlockAckType baType;
};

struct WifiBarBlockAck : public WifiAcknowledgment {
  WifiBarBlockAck();

  std::unique_ptr<WifiAcknowledgment> Copy() const override;
  bool CheckQosAckPolicy(Mac48Address receiver, uint8_t tid,
                         WifiMacHeader::QosAckPolicy ackPolicy) const override;
  void Print(std::ostream &os) const override;

  WifiTxVector blockAckReqTxVector;
  WifiTxVector blockAckTxVector;
  BlockAckReqType barType;
  BlockAckType baType;
};

struct WifiDlMuBarBaSequence : public WifiAcknowledgment {
  WifiDlMuBarBaSequence();

  std::unique_ptr<WifiAcknowledgment> Copy() const override;
  bool CheckQosAckPolicy(Mac48Address receiver, uint8_t tid,
                         WifiMacHeader::QosAckPolicy ackPolicy) const override;
  void Print(std::ostream &os) const override;

  struct AckInfo {
    WifiTxVector ackTxVector;
  };

  struct BlockAckInfo {
    WifiTxVector blockAckTxVector;
    BlockAckType baType;
  };

  struct BlockAckReqInfo {
    WifiTxVector blockAckReqTxVector;
    BlockAckReqType barType;
    WifiTxVector blockAckTxVector;
    BlockAckType baType;
  };

  std::map<Mac48Address, AckInfo> stationsReplyingWithNormalAck;
  std::map<Mac48Address, BlockAckInfo> stationsReplyingWithBlockAck;
  std::map<Mac48Address, BlockAckReqInfo> stationsSendBlockAckReqTo;
};

struct WifiDlMuTfMuBar : public WifiAcknowledgment {
  WifiDlMuTfMuBar();

  std::unique_ptr<WifiAcknowledgment> Copy() const override;
  bool CheckQosAckPolicy(Mac48Address receiver, uint8_t tid,
                         WifiMacHeader::QosAckPolicy ackPolicy) const override;
  void Print(std::ostream &os) const override;

  struct BlockAckInfo {
    CtrlBAckRequestHeader barHeader;
    WifiTxVector blockAckTxVector;
    BlockAckType baType;
  };

  std::map<Mac48Address, BlockAckInfo> stationsReplyingWithBlockAck;
  std::list<BlockAckReqType> barTypes;
  uint16_t ulLength;
  WifiTxVector muBarTxVector;
};

struct WifiDlMuAggregateTf : public WifiAcknowledgment {
  WifiDlMuAggregateTf();

  std::unique_ptr<WifiAcknowledgment> Copy() const override;
  bool CheckQosAckPolicy(Mac48Address receiver, uint8_t tid,
                         WifiMacHeader::QosAckPolicy ackPolicy) const override;
  void Print(std::ostream &os) const override;

  struct BlockAckInfo {
    uint32_t muBarSize;
    CtrlBAckRequestHeader barHeader;
    WifiTxVector blockAckTxVector;
    BlockAckType baType;
  };

  std::map<Mac48Address, BlockAckInfo> stationsReplyingWithBlockAck;
  uint16_t ulLength;
};

struct WifiUlMuMultiStaBa : public WifiAcknowledgment {
  WifiUlMuMultiStaBa();

  std::unique_ptr<WifiAcknowledgment> Copy() const override;
  bool CheckQosAckPolicy(Mac48Address receiver, uint8_t tid,
                         WifiMacHeader::QosAckPolicy ackPolicy) const override;
  void Print(std::ostream &os) const override;

  std::map<std::pair<Mac48Address, uint8_t>, std::size_t>
      stationsReceivingMultiStaBa;
  BlockAckType baType;
  WifiTxVector tbPpduTxVector;
  WifiTxVector multiStaBaTxVector;
};

struct WifiAckAfterTbPpdu : public WifiAcknowledgment {
  WifiAckAfterTbPpdu();

  std::unique_ptr<WifiAcknowledgment> Copy() const override;
  bool CheckQosAckPolicy(Mac48Address receiver, uint8_t tid,
                         WifiMacHeader::QosAckPolicy ackPolicy) const override;
  void Print(std::ostream &os) const override;
};

std::ostream &operator<<(std::ostream &os,
                         const WifiAcknowledgment *acknowledgment);

} // namespace ns3

#endif
