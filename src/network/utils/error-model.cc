

#include "error-model.h"

#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ErrorModel");

NS_OBJECT_ENSURE_REGISTERED(ErrorModel);

TypeId ErrorModel::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ErrorModel")
          .SetParent<Object>()
          .SetGroupName("Network")
          .AddAttribute(
              "IsEnabled", "Whether this ErrorModel is enabled or not.",
              BooleanValue(true), MakeBooleanAccessor(&ErrorModel::m_enable),
              MakeBooleanChecker());
  return tid;
}

ErrorModel::ErrorModel() : m_enable(true) { NS_LOG_FUNCTION(this); }

ErrorModel::~ErrorModel() { NS_LOG_FUNCTION(this); }

bool ErrorModel::IsCorrupt(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this << p);
  bool result;
  result = DoCorrupt(p);
  return result;
}

void ErrorModel::Reset() {
  NS_LOG_FUNCTION(this);
  DoReset();
}

void ErrorModel::Enable() {
  NS_LOG_FUNCTION(this);
  m_enable = true;
}

void ErrorModel::Disable() {
  NS_LOG_FUNCTION(this);
  m_enable = false;
}

bool ErrorModel::IsEnabled() const {
  NS_LOG_FUNCTION(this);
  return m_enable;
}

NS_OBJECT_ENSURE_REGISTERED(RateErrorModel);

TypeId RateErrorModel::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::RateErrorModel")
          .SetParent<ErrorModel>()
          .SetGroupName("Network")
          .AddConstructor<RateErrorModel>()
          .AddAttribute("ErrorUnit", "The error unit",
                        EnumValue(ERROR_UNIT_BYTE),
                        MakeEnumAccessor(&RateErrorModel::m_unit),
                        MakeEnumChecker(ERROR_UNIT_BIT, "ERROR_UNIT_BIT",
                                        ERROR_UNIT_BYTE, "ERROR_UNIT_BYTE",
                                        ERROR_UNIT_PACKET, "ERROR_UNIT_PACKET"))
          .AddAttribute("ErrorRate", "The error rate.", DoubleValue(0.0),
                        MakeDoubleAccessor(&RateErrorModel::m_rate),
                        MakeDoubleChecker<double>())
          .AddAttribute(
              "RanVar", "The decision variable attached to this error model.",
              StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
              MakePointerAccessor(&RateErrorModel::m_ranvar),
              MakePointerChecker<RandomVariableStream>());
  return tid;
}

RateErrorModel::RateErrorModel() { NS_LOG_FUNCTION(this); }

RateErrorModel::~RateErrorModel() { NS_LOG_FUNCTION(this); }

RateErrorModel::ErrorUnit RateErrorModel::GetUnit() const {
  NS_LOG_FUNCTION(this);
  return m_unit;
}

void RateErrorModel::SetUnit(ErrorUnit error_unit) {
  NS_LOG_FUNCTION(this << error_unit);
  m_unit = error_unit;
}

double RateErrorModel::GetRate() const {
  NS_LOG_FUNCTION(this);
  return m_rate;
}

void RateErrorModel::SetRate(double rate) {
  NS_LOG_FUNCTION(this << rate);
  m_rate = rate;
}

void RateErrorModel::SetRandomVariable(Ptr<RandomVariableStream> ranvar) {
  NS_LOG_FUNCTION(this << ranvar);
  m_ranvar = ranvar;
}

int64_t RateErrorModel::AssignStreams(int64_t stream) {
  NS_LOG_FUNCTION(this << stream);
  m_ranvar->SetStream(stream);
  return 1;
}

bool RateErrorModel::DoCorrupt(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this << p);
  if (!IsEnabled()) {
    return false;
  }
  switch (m_unit) {
  case ERROR_UNIT_PACKET:
    return DoCorruptPkt(p);
  case ERROR_UNIT_BYTE:
    return DoCorruptByte(p);
  case ERROR_UNIT_BIT:
    return DoCorruptBit(p);
  default:
    NS_ASSERT_MSG(false, "m_unit not supported yet");
    break;
  }
  return false;
}

bool RateErrorModel::DoCorruptPkt(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this << p);
  return (m_ranvar->GetValue() < m_rate);
}

bool RateErrorModel::DoCorruptByte(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this << p);
  double per = 1 - std::pow(1.0 - m_rate, static_cast<double>(p->GetSize()));
  return (m_ranvar->GetValue() < per);
}

bool RateErrorModel::DoCorruptBit(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this << p);
  double per =
      1 - std::pow(1.0 - m_rate, static_cast<double>(8 * p->GetSize()));
  return (m_ranvar->GetValue() < per);
}

void RateErrorModel::DoReset() { NS_LOG_FUNCTION(this); }

NS_OBJECT_ENSURE_REGISTERED(BurstErrorModel);

TypeId BurstErrorModel::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::BurstErrorModel")
          .SetParent<ErrorModel>()
          .SetGroupName("Network")
          .AddConstructor<BurstErrorModel>()
          .AddAttribute("ErrorRate", "The burst error event.", DoubleValue(0.0),
                        MakeDoubleAccessor(&BurstErrorModel::m_burstRate),
                        MakeDoubleChecker<double>())
          .AddAttribute(
              "BurstStart",
              "The decision variable attached to this error model.",
              StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
              MakePointerAccessor(&BurstErrorModel::m_burstStart),
              MakePointerChecker<RandomVariableStream>())
          .AddAttribute("BurstSize",
                        "The number of packets being corrupted at one drop.",
                        StringValue("ns3::UniformRandomVariable[Min=1|Max=4]"),
                        MakePointerAccessor(&BurstErrorModel::m_burstSize),
                        MakePointerChecker<RandomVariableStream>());
  return tid;
}

BurstErrorModel::BurstErrorModel() : m_counter(0), m_currentBurstSz(0) {}

BurstErrorModel::~BurstErrorModel() { NS_LOG_FUNCTION(this); }

double BurstErrorModel::GetBurstRate() const {
  NS_LOG_FUNCTION(this);
  return m_burstRate;
}

void BurstErrorModel::SetBurstRate(double rate) {
  NS_LOG_FUNCTION(this << rate);
  m_burstRate = rate;
}

void BurstErrorModel::SetRandomVariable(Ptr<RandomVariableStream> ranVar) {
  NS_LOG_FUNCTION(this << ranVar);
  m_burstStart = ranVar;
}

void BurstErrorModel::SetRandomBurstSize(Ptr<RandomVariableStream> burstSz) {
  NS_LOG_FUNCTION(this << burstSz);
  m_burstSize = burstSz;
}

int64_t BurstErrorModel::AssignStreams(int64_t stream) {
  NS_LOG_FUNCTION(this << stream);
  m_burstStart->SetStream(stream);
  m_burstSize->SetStream(stream);
  return 2;
}

bool BurstErrorModel::DoCorrupt(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this);
  if (!IsEnabled()) {
    return false;
  }
  double ranVar = m_burstStart->GetValue();

  if (ranVar < m_burstRate) {
    m_currentBurstSz = m_burstSize->GetInteger();
    NS_LOG_DEBUG("new burst size selected: " << m_currentBurstSz);
    if (m_currentBurstSz == 0) {
      NS_LOG_WARN("Burst size == 0; shouldn't happen");
      return false;
    }
    m_counter = 1;
    return true;
  } else {
    if (m_counter < m_currentBurstSz) {
      m_counter++;
      return true;
    } else {
      return false;
    }
  }
}

void BurstErrorModel::DoReset() {
  NS_LOG_FUNCTION(this);
  m_counter = 0;
  m_currentBurstSz = 0;
}

NS_OBJECT_ENSURE_REGISTERED(ListErrorModel);

TypeId ListErrorModel::GetTypeId() {
  static TypeId tid = TypeId("ns3::ListErrorModel")
                          .SetParent<ErrorModel>()
                          .SetGroupName("Network")
                          .AddConstructor<ListErrorModel>();
  return tid;
}

ListErrorModel::ListErrorModel() { NS_LOG_FUNCTION(this); }

ListErrorModel::~ListErrorModel() { NS_LOG_FUNCTION(this); }

std::list<uint64_t> ListErrorModel::GetList() const {
  NS_LOG_FUNCTION(this);
  return m_packetList;
}

void ListErrorModel::SetList(const std::list<uint64_t> &packetlist) {
  NS_LOG_FUNCTION(this << &packetlist);
  m_packetList = packetlist;
}

bool ListErrorModel::DoCorrupt(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this << p);
  if (!IsEnabled()) {
    return false;
  }
  auto uid = p->GetUid();
  for (auto i = m_packetList.begin(); i != m_packetList.end(); i++) {
    if (uid == *i) {
      return true;
    }
  }
  return false;
}

void ListErrorModel::DoReset() {
  NS_LOG_FUNCTION(this);
  m_packetList.clear();
}

NS_OBJECT_ENSURE_REGISTERED(ReceiveListErrorModel);

TypeId ReceiveListErrorModel::GetTypeId() {
  static TypeId tid = TypeId("ns3::ReceiveListErrorModel")
                          .SetParent<ErrorModel>()
                          .SetGroupName("Network")
                          .AddConstructor<ReceiveListErrorModel>();
  return tid;
}

ReceiveListErrorModel::ReceiveListErrorModel() : m_timesInvoked(0) {
  NS_LOG_FUNCTION(this);
}

ReceiveListErrorModel::~ReceiveListErrorModel() { NS_LOG_FUNCTION(this); }

std::list<uint32_t> ReceiveListErrorModel::GetList() const {
  NS_LOG_FUNCTION(this);
  return m_packetList;
}

void ReceiveListErrorModel::SetList(const std::list<uint32_t> &packetlist) {
  NS_LOG_FUNCTION(this << &packetlist);
  m_packetList = packetlist;
}

bool ReceiveListErrorModel::DoCorrupt(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this << p);
  if (!IsEnabled()) {
    return false;
  }
  m_timesInvoked += 1;
  for (auto i = m_packetList.begin(); i != m_packetList.end(); i++) {
    if (m_timesInvoked - 1 == *i) {
      return true;
    }
  }
  return false;
}

void ReceiveListErrorModel::DoReset() {
  NS_LOG_FUNCTION(this);
  m_packetList.clear();
}

NS_OBJECT_ENSURE_REGISTERED(BinaryErrorModel);

TypeId BinaryErrorModel::GetTypeId() {
  static TypeId tid = TypeId("ns3::BinaryErrorModel")
                          .SetParent<ErrorModel>()
                          .AddConstructor<BinaryErrorModel>();
  return tid;
}

BinaryErrorModel::BinaryErrorModel() {
  NS_LOG_FUNCTION(this);
  m_counter = 0;
}

BinaryErrorModel::~BinaryErrorModel() { NS_LOG_FUNCTION(this); }

bool BinaryErrorModel::DoCorrupt(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this);
  if (!IsEnabled()) {
    return false;
  }
  bool ret = m_counter % 2;
  m_counter++;
  return ret;
}

void BinaryErrorModel::DoReset() {
  NS_LOG_FUNCTION(this);
  m_counter = 0;
}

} // namespace ns3
