
#include "tap-bridge.h"

#include "tap-encode-decode.h"

#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/channel.h"
#include "ns3/enum.h"
#include "ns3/ethernet-header.h"
#include "ns3/fd-reader.h"
#include "ns3/ipv4.h"
#include "ns3/llc-snap-header.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/realtime-simulator-impl.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TapBridge");

FdReader::Data TapBridgeFdReader::DoRead() {
  NS_LOG_FUNCTION(this);

  uint32_t bufferSize = 65536;
  auto buf = (uint8_t *)std::malloc(bufferSize);
  NS_ABORT_MSG_IF(buf == nullptr, "malloc() failed");

  NS_LOG_LOGIC("Calling read on tap device fd " << m_fd);
  ssize_t len = read(m_fd, buf, bufferSize);
  if (len <= 0) {
    NS_LOG_INFO("TapBridgeFdReader::DoRead(): done");
    std::free(buf);
    buf = nullptr;
    len = 0;
  }

  return FdReader::Data(buf, len);
}

#define TAP_MAGIC 95549

NS_OBJECT_ENSURE_REGISTERED(TapBridge);

TypeId TapBridge::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::TapBridge")
          .SetParent<NetDevice>()
          .SetGroupName("TapBridge")
          .AddConstructor<TapBridge>()
          .AddAttribute(
              "Mtu", "The MAC-level Maximum Transmission Unit",
              UintegerValue(0),
              MakeUintegerAccessor(&TapBridge::SetMtu, &TapBridge::GetMtu),
              MakeUintegerChecker<uint16_t>())
          .AddAttribute("DeviceName", "The name of the tap device to create.",
                        StringValue(""),
                        MakeStringAccessor(&TapBridge::m_tapDeviceName),
                        MakeStringChecker())
          .AddAttribute("Gateway",
                        "The IP address of the default gateway to assign to "
                        "the host machine, "
                        "when in ConfigureLocal mode.",
                        Ipv4AddressValue("255.255.255.255"),
                        MakeIpv4AddressAccessor(&TapBridge::m_tapGateway),
                        MakeIpv4AddressChecker())
          .AddAttribute("IpAddress",
                        "The IP address to assign to the tap device, when in "
                        "ConfigureLocal mode.  "
                        "This address will override the discovered IP address "
                        "of the simulated device.",
                        Ipv4AddressValue("255.255.255.255"),
                        MakeIpv4AddressAccessor(&TapBridge::m_tapIp),
                        MakeIpv4AddressChecker())
          .AddAttribute("MacAddress",
                        "The MAC address to assign to the tap device, when in "
                        "ConfigureLocal mode.  "
                        "This address will override the discovered MAC address "
                        "of the simulated device.",
                        Mac48AddressValue(Mac48Address("ff:ff:ff:ff:ff:ff")),
                        MakeMac48AddressAccessor(&TapBridge::m_tapMac),
                        MakeMac48AddressChecker())
          .AddAttribute("Netmask",
                        "The network mask to assign to the tap device, when in "
                        "ConfigureLocal mode.  "
                        "This address will override the discovered MAC address "
                        "of the simulated device.",
                        Ipv4MaskValue("255.255.255.255"),
                        MakeIpv4MaskAccessor(&TapBridge::m_tapNetmask),
                        MakeIpv4MaskChecker())
          .AddAttribute("Start",
                        "The simulation time at which to spin up the tap "
                        "device read thread.",
                        TimeValue(Seconds(0.)),
                        MakeTimeAccessor(&TapBridge::m_tStart),
                        MakeTimeChecker())
          .AddAttribute("Stop",
                        "The simulation time at which to tear down the tap "
                        "device read thread.",
                        TimeValue(Seconds(0.)),
                        MakeTimeAccessor(&TapBridge::m_tStop),
                        MakeTimeChecker())
          .AddAttribute(
              "Mode", "The operating and configuration mode to use.",
              EnumValue(USE_LOCAL), MakeEnumAccessor(&TapBridge::SetMode),
              MakeEnumChecker(CONFIGURE_LOCAL, "ConfigureLocal", USE_LOCAL,
                              "UseLocal", USE_BRIDGE, "UseBridge"))
          .AddAttribute(
              "Verbose", "Enable verbose output from tap-creator child process",
              BooleanValue(false), MakeBooleanAccessor(&TapBridge::m_verbose),
              MakeBooleanChecker());
  return tid;
}

TapBridge::TapBridge()
    : m_node(nullptr), m_ifIndex(0), m_sock(-1), m_startEvent(), m_stopEvent(),
      m_fdReader(nullptr), m_ns3AddressRewritten(false) {
  NS_LOG_FUNCTION(this);
  m_packetBuffer = new uint8_t[65536];
  Start(m_tStart);
}

TapBridge::~TapBridge() {
  NS_LOG_FUNCTION(this);

  StopTapDevice();

  delete[] m_packetBuffer;
  m_packetBuffer = nullptr;

  m_bridgedDevice = nullptr;
}

void TapBridge::DoDispose() {
  NS_LOG_FUNCTION(this);
  NetDevice::DoDispose();
}

void TapBridge::Start(Time tStart) {
  NS_LOG_FUNCTION(this << tStart);

  Simulator::Cancel(m_startEvent);
  m_startEvent = Simulator::Schedule(tStart, &TapBridge::StartTapDevice, this);
}

void TapBridge::Stop(Time tStop) {
  NS_LOG_FUNCTION(this << tStop);
  Simulator::Cancel(m_stopEvent);
  m_startEvent = Simulator::Schedule(tStop, &TapBridge::StopTapDevice, this);
}

void TapBridge::StartTapDevice() {
  NS_LOG_FUNCTION(this);

  NS_ABORT_MSG_IF(m_sock != -1,
                  "TapBridge::StartTapDevice(): Tap is already started");

  m_nodeId = GetNode()->GetId();

  NS_LOG_LOGIC("Creating tap device");

  CreateTap();

  NotifyLinkUp();

  NS_ABORT_MSG_IF(
      m_fdReader,
      "TapBridge::StartTapDevice(): Receive thread is already running");
  NS_LOG_LOGIC("Spinning up read thread");

  m_fdReader = Create<TapBridgeFdReader>();
  m_fdReader->Start(m_sock, MakeCallback(&TapBridge::ReadCallback, this));
}

void TapBridge::StopTapDevice() {
  NS_LOG_FUNCTION(this);

  if (m_fdReader) {
    m_fdReader->Stop();
    m_fdReader = nullptr;
  }

  if (m_sock != -1) {
    close(m_sock);
    m_sock = -1;
  }
}

void TapBridge::CreateTap() {
  NS_LOG_FUNCTION(this);

  int sock = socket(PF_UNIX, SOCK_DGRAM, 0);
  NS_ABORT_MSG_IF(sock == -1,
                  "TapBridge::CreateTap(): Unix socket creation error, errno = "
                      << std::strerror(errno));

  struct sockaddr_un un;
  memset(&un, 0, sizeof(un));
  un.sun_family = AF_UNIX;
  int status = bind(sock, (struct sockaddr *)&un, sizeof(sa_family_t));
  NS_ABORT_MSG_IF(status == -1,
                  "TapBridge::CreateTap(): Could not bind(): errno = "
                      << std::strerror(errno));
  NS_LOG_INFO("Created Unix socket");
  NS_LOG_INFO("sun_family = " << un.sun_family);
  NS_LOG_INFO("sun_path = " << un.sun_path);

  socklen_t len = sizeof(un);
  status = getsockname(sock, (struct sockaddr *)&un, &len);
  NS_ABORT_MSG_IF(status == -1,
                  "TapBridge::CreateTap(): Could not getsockname(): errno = "
                      << std::strerror(errno));

  std::string path = TapBufferToString((uint8_t *)&un, len);
  NS_LOG_INFO("Encoded Unix socket as \"" << path << "\"");

  Ptr<NetDevice> nd = GetBridgedNetDevice();
  Ptr<Node> n = nd->GetNode();
  Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();

  pid_t pid = ::fork();
  if (pid == 0) {
    NS_LOG_DEBUG("Child process");

    bool wantIp = (m_mode == CONFIGURE_LOCAL);

    if (wantIp && (!ipv4) && m_tapIp.IsBroadcast() &&
        m_tapNetmask == Ipv4Mask::GetOnes()) {
      NS_FATAL_ERROR(
          "TapBridge::CreateTap(): Tap device IP configuration requested but "
          "neither IP address nor IP netmask is provided");
    }

    Ipv4Address ipv4Address("255.255.255.255");
    Ipv4Mask ipv4Mask("255.255.255.255");

    if (ipv4) {
      uint32_t index = ipv4->GetInterfaceForDevice(nd);
      if (ipv4->GetNAddresses(index) > 1) {
        NS_LOG_WARN("Underlying bridged NetDevice has multiple IP addresses; "
                    "using first one.");
      }
      ipv4Address = ipv4->GetAddress(index, 0).GetLocal();

      ipv4Mask = ipv4->GetAddress(index, 0).GetMask();
    }

    Address address = nd->GetAddress();
    Mac48Address mac48Address = Mac48Address::ConvertFrom(address);

    std::ostringstream ossDeviceName;
    ossDeviceName << "-d" << m_tapDeviceName;

    std::ostringstream ossGateway;
    ossGateway << "-g" << m_tapGateway;

    std::ostringstream ossIp;
    if (m_tapIp.IsBroadcast()) {
      ossIp << "-i" << ipv4Address;
    } else {
      ossIp << "-i" << m_tapIp;
    }

    std::ostringstream ossMac;
    if (m_tapMac.IsBroadcast()) {
      ossMac << "-m" << mac48Address;
    } else {
      ossMac << "-m" << m_tapMac;
    }

    std::ostringstream ossNetmask;
    if (m_tapNetmask == Ipv4Mask::GetOnes()) {
      ossNetmask << "-n" << ipv4Mask;
    } else {
      ossNetmask << "-n" << m_tapNetmask;
    }

    std::ostringstream ossMode;
    ossMode << "-o";
    if (m_mode == CONFIGURE_LOCAL) {
      ossMode << "1";
    } else if (m_mode == USE_LOCAL) {
      ossMode << "2";
    } else {
      ossMode << "3";
    }

    std::ostringstream ossVerbose;
    if (m_verbose) {
      ossVerbose << "-v";
    }

    std::ostringstream ossPath;
    ossPath << "-p" << path;

    NS_LOG_DEBUG("Executing: " << TAP_CREATOR << " " << ossDeviceName.str()
                               << " " << ossGateway.str() << " " << ossIp.str()
                               << " " << ossMac.str() << " " << ossNetmask.str()
                               << " " << ossMode.str() << " " << ossPath.str()
                               << " " << ossVerbose.str());

    status = ::execlp(TAP_CREATOR, TAP_CREATOR, ossDeviceName.str().c_str(),
                      ossGateway.str().c_str(), ossIp.str().c_str(),
                      ossMac.str().c_str(), ossNetmask.str().c_str(),
                      ossMode.str().c_str(), ossPath.str().c_str(),
                      ossVerbose.str().c_str(), (char *)nullptr);

    NS_FATAL_ERROR("TapBridge::CreateTap(): Back from execlp(), status = "
                   << status << " errno = " << ::strerror(errno));
  } else {
    NS_LOG_DEBUG("Parent process");
    int st;
    pid_t waited = waitpid(pid, &st, 0);
    NS_ABORT_MSG_IF(waited == -1,
                    "TapBridge::CreateTap(): waitpid() fails, errno = "
                        << std::strerror(errno));
    NS_ASSERT_MSG(pid == waited, "TapBridge::CreateTap(): pid mismatch");

    if (WIFEXITED(st)) {
      int exitStatus = WEXITSTATUS(st);
      NS_ABORT_MSG_IF(
          exitStatus != 0,
          "TapBridge::CreateTap(): socket creator exited normally with status "
              << exitStatus);
    } else if (WIFSIGNALED(st)) {
      NS_FATAL_ERROR(
          "TapBridge::CreateTap(): socket creator exited with signal "
          << WTERMSIG(st));
    } else {
      NS_FATAL_ERROR(
          "TapBridge::CreateTap(): socket creator exited abnormally");
    }

    struct iovec iov;
    uint32_t magic;
    iov.iov_base = &magic;
    iov.iov_len = sizeof(magic);

    size_t msg_size = sizeof(int);
    char control[CMSG_SPACE(msg_size)];

    struct msghdr msg;
    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);
    msg.msg_flags = 0;

    ssize_t bytesRead = recvmsg(sock, &msg, 0);
    NS_ABORT_MSG_IF(
        bytesRead != sizeof(int),
        "TapBridge::CreateTap(): Wrong byte count from socket creator");

    struct cmsghdr *cmsg;
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr;
         cmsg = CMSG_NXTHDR(&msg, cmsg)) {
      if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        if (magic == TAP_MAGIC) {
          NS_LOG_INFO("Got SCM_RIGHTS with correct magic " << magic);
          int *rawSocket = (int *)CMSG_DATA(cmsg);
          NS_LOG_INFO(
              "Got the socket from the socket creator = " << *rawSocket);
          m_sock = *rawSocket;
          break;
        } else {
          NS_LOG_INFO("Got SCM_RIGHTS, but with bad magic " << magic);
        }
      }
    }
    if (cmsg == nullptr) {
      NS_FATAL_ERROR("Did not get the raw socket from the socket creator");
    }

    if (m_mode == USE_BRIDGE) {
      struct ifreq s;
      memset(&s, 0, sizeof(struct ifreq));
      strncpy(s.ifr_name, m_tapDeviceName.c_str(), IFNAMSIZ - 1);

      NS_LOG_INFO("Trying to get MacAddr of " << m_tapDeviceName);
      int ioctlResult = ioctl(sock, SIOCGIFHWADDR, &s);
      if (ioctlResult == 0) {
        Mac48Address learnedMac;
        learnedMac.CopyFrom((uint8_t *)s.ifr_hwaddr.sa_data);
        NS_LOG_INFO("Learned Tap device MacAddr is "
                    << learnedMac
                    << ": setting ns-3 device to use this address");
        m_bridgedDevice->SetAddress(learnedMac);
        m_ns3AddressRewritten = true;
      }

      if (!m_ns3AddressRewritten) {
        NS_LOG_INFO("Cannot get MacAddr of Tap device: "
                    << m_tapDeviceName
                    << " while in USE_LOCAL/USE_BRIDGE mode: "
                    << std::strerror(errno));
        NS_LOG_INFO("Underlying ns-3 device will continue to use default "
                    "address, what can "
                    "lead to connectivity errors");
      }
    }
  }

  close(sock);
}

void TapBridge::ReadCallback(uint8_t *buf, ssize_t len) {
  NS_LOG_FUNCTION(this << buf << len);

  NS_ASSERT_MSG(buf != nullptr, "invalid buf argument");
  NS_ASSERT_MSG(len > 0, "invalid len argument");

  NS_LOG_INFO("TapBridge::ReadCallback(): Received packet on node "
              << m_nodeId);
  NS_LOG_INFO("TapBridge::ReadCallback(): Scheduling handler");
  Simulator::ScheduleWithContext(
      m_nodeId, Seconds(0.0),
      MakeEvent(&TapBridge::ForwardToBridgedDevice, this, buf, len));
}

void TapBridge::ForwardToBridgedDevice(uint8_t *buf, ssize_t len) {
  NS_LOG_FUNCTION(this << buf << len);

  Ptr<Packet> packet =
      Create<Packet>(reinterpret_cast<const uint8_t *>(buf), len);
  std::free(buf);
  buf = nullptr;

  Address src;
  Address dst;
  uint16_t type;

  NS_LOG_LOGIC("Received packet from tap device");

  Ptr<Packet> p = Filter(packet, &src, &dst, &type);
  if (!p) {
    NS_LOG_LOGIC("TapBridge::ForwardToBridgedDevice:  Discarding packet as "
                 "unfit for ns-3 consumption");
    return;
  }

  NS_LOG_LOGIC("Pkt source is " << src);
  NS_LOG_LOGIC("Pkt destination is " << dst);
  NS_LOG_LOGIC("Pkt LengthType is " << type);
  if (m_mode == USE_LOCAL) {
    NS_ASSERT_MSG(
        Mac48Address::ConvertFrom(src) != Mac48Address("ff:ff:ff:ff:ff:ff"),
        "TapBridge::ForwardToBridgedDevice:  Source addr is broadcast");
    if (!m_ns3AddressRewritten) {
      Mac48Address learnedMac = Mac48Address::ConvertFrom(src);
      NS_LOG_LOGIC("Learned MacAddr is "
                   << learnedMac
                   << ": setting ns-3 device to use this address");
      m_bridgedDevice->SetAddress(Mac48Address::ConvertFrom(learnedMac));
      m_ns3AddressRewritten = true;
    }
    NS_LOG_LOGIC("Forwarding packet to ns-3 device via Send()");
    m_bridgedDevice->Send(packet, dst, type);
    return;
  }

  NS_LOG_LOGIC("Forwarding packet");

  if (m_mode == USE_BRIDGE) {
    m_bridgedDevice->SendFrom(packet, src, dst, type);
  } else {
    NS_ASSERT_MSG(m_mode == CONFIGURE_LOCAL,
                  "TapBridge::ForwardToBridgedDevice(): Internal error");
    m_bridgedDevice->Send(packet, dst, type);
  }
}

Ptr<Packet> TapBridge::Filter(Ptr<Packet> p, Address *src, Address *dst,
                              uint16_t *type) {
  NS_LOG_FUNCTION(this << p);
  uint32_t pktSize;

  pktSize = p->GetSize();
  EthernetHeader header(false);
  if (pktSize < header.GetSerializedSize()) {
    return nullptr;
  }

  uint32_t headerSize = p->PeekHeader(header);
  p->RemoveAtStart(headerSize);

  NS_LOG_LOGIC("Pkt source is " << header.GetSource());
  NS_LOG_LOGIC("Pkt destination is " << header.GetDestination());
  NS_LOG_LOGIC("Pkt LengthType is " << header.GetLengthType());

  if (header.GetLengthType() <= 1500) {
    *src = header.GetSource();
    *dst = header.GetDestination();

    pktSize = p->GetSize();
    LlcSnapHeader llc;
    if (pktSize < llc.GetSerializedSize()) {
      return nullptr;
    }

    p->RemoveHeader(llc);
    *type = llc.GetType();
  } else {
    *src = header.GetSource();
    *dst = header.GetDestination();
    *type = header.GetLengthType();
  }

  return p;
}

Ptr<NetDevice> TapBridge::GetBridgedNetDevice() {
  NS_LOG_FUNCTION(this);
  return m_bridgedDevice;
}

void TapBridge::SetBridgedNetDevice(Ptr<NetDevice> bridgedDevice) {
  NS_LOG_FUNCTION(this << bridgedDevice);

  NS_ASSERT_MSG(m_node,
                "TapBridge::SetBridgedDevice:  Bridge not installed in a node");
  NS_ASSERT_MSG(bridgedDevice != this,
                "TapBridge::SetBridgedDevice:  Cannot bridge to self");
  NS_ASSERT_MSG(!m_bridgedDevice,
                "TapBridge::SetBridgedDevice:  Already bridged");

  if (!Mac48Address::IsMatchingType(bridgedDevice->GetAddress())) {
    NS_FATAL_ERROR("TapBridge::SetBridgedDevice: Device does not support eui "
                   "48 addresses: "
                   "cannot be added to bridge.");
  }

  if (m_mode == USE_BRIDGE && !bridgedDevice->SupportsSendFrom()) {
    NS_FATAL_ERROR("TapBridge::SetBridgedDevice: Device does not support "
                   "SendFrom: cannot be "
                   "added to bridge.");
  }

  bridgedDevice->SetReceiveCallback(
      MakeCallback(&TapBridge::DiscardFromBridgedDevice, this));
  bridgedDevice->SetPromiscReceiveCallback(
      MakeCallback(&TapBridge::ReceiveFromBridgedDevice, this));
  m_bridgedDevice = bridgedDevice;
}

bool TapBridge::DiscardFromBridgedDevice(Ptr<NetDevice> device,
                                         Ptr<const Packet> packet,
                                         uint16_t protocol,
                                         const Address &src) {
  NS_LOG_FUNCTION(this << device << packet << protocol << src);
  NS_LOG_LOGIC("Discarding packet stolen from bridged device " << device);
  return true;
}

bool TapBridge::ReceiveFromBridgedDevice(Ptr<NetDevice> device,
                                         Ptr<const Packet> packet,
                                         uint16_t protocol, const Address &src,
                                         const Address &dst,
                                         PacketType packetType) {
  NS_LOG_FUNCTION(this << device << packet << protocol << src << dst
                       << packetType);
  NS_ASSERT_MSG(
      device == m_bridgedDevice,
      "TapBridge::SetBridgedDevice:  Received packet from unexpected device");
  NS_LOG_DEBUG("Packet UID is " << packet->GetUid());

  if (m_mode == CONFIGURE_LOCAL && packetType == PACKET_OTHERHOST) {
    return true;
  }

  Mac48Address from = Mac48Address::ConvertFrom(src);
  Mac48Address to = Mac48Address::ConvertFrom(dst);

  Ptr<Packet> p = packet->Copy();
  EthernetHeader header = EthernetHeader(false);
  header.SetSource(from);
  header.SetDestination(to);

  header.SetLengthType(protocol);
  p->AddHeader(header);

  NS_LOG_LOGIC("Writing packet to Linux host");
  NS_LOG_LOGIC("Pkt source is " << header.GetSource());
  NS_LOG_LOGIC("Pkt destination is " << header.GetDestination());
  NS_LOG_LOGIC("Pkt LengthType is " << header.GetLengthType());
  NS_LOG_LOGIC("Pkt size is " << p->GetSize());

  NS_ASSERT_MSG(p->GetSize() <= 65536,
                "TapBridge::ReceiveFromBridgedDevice: Packet too big "
                    << p->GetSize());
  p->CopyData(m_packetBuffer, p->GetSize());

  uint32_t bytesWritten = write(m_sock, m_packetBuffer, p->GetSize());
  NS_ABORT_MSG_IF(bytesWritten != p->GetSize(),
                  "TapBridge::ReceiveFromBridgedDevice(): Write error.");

  NS_LOG_LOGIC("End of receive packet handling on node " << m_node->GetId());
  return true;
}

void TapBridge::SetIfIndex(const uint32_t index) {
  NS_LOG_FUNCTION(this << index);
  m_ifIndex = index;
}

uint32_t TapBridge::GetIfIndex() const {
  NS_LOG_FUNCTION(this);
  return m_ifIndex;
}

Ptr<Channel> TapBridge::GetChannel() const {
  NS_LOG_FUNCTION(this);
  return nullptr;
}

void TapBridge::SetAddress(Address address) {
  NS_LOG_FUNCTION(this << address);
  m_address = Mac48Address::ConvertFrom(address);
}

Address TapBridge::GetAddress() const {
  NS_LOG_FUNCTION(this);
  return m_address;
}

void TapBridge::SetMode(Mode mode) {
  NS_LOG_FUNCTION(this << mode);
  m_mode = mode;
}

TapBridge::Mode TapBridge::GetMode() {
  NS_LOG_FUNCTION(this);
  return m_mode;
}

bool TapBridge::SetMtu(const uint16_t mtu) {
  NS_LOG_FUNCTION(this << mtu);
  m_mtu = mtu;
  return true;
}

uint16_t TapBridge::GetMtu() const {
  NS_LOG_FUNCTION(this);
  return m_mtu;
}

void TapBridge::NotifyLinkUp() {
  NS_LOG_FUNCTION(this);
  if (!m_linkUp) {
    m_linkUp = true;
    m_linkChangeCallbacks();
  }
}

bool TapBridge::IsLinkUp() const {
  NS_LOG_FUNCTION(this);
  return m_linkUp;
}

void TapBridge::AddLinkChangeCallback(Callback<void> callback) {
  NS_LOG_FUNCTION(this);
  m_linkChangeCallbacks.ConnectWithoutContext(callback);
}

bool TapBridge::IsBroadcast() const {
  NS_LOG_FUNCTION(this);
  return true;
}

Address TapBridge::GetBroadcast() const {
  NS_LOG_FUNCTION(this);
  return Mac48Address("ff:ff:ff:ff:ff:ff");
}

bool TapBridge::IsMulticast() const {
  NS_LOG_FUNCTION(this);
  return true;
}

Address TapBridge::GetMulticast(Ipv4Address multicastGroup) const {
  NS_LOG_FUNCTION(this << multicastGroup);
  Mac48Address multicast = Mac48Address::GetMulticast(multicastGroup);
  return multicast;
}

bool TapBridge::IsPointToPoint() const {
  NS_LOG_FUNCTION(this);
  return false;
}

bool TapBridge::IsBridge() const {
  NS_LOG_FUNCTION(this);
  return false;
}

bool TapBridge::Send(Ptr<Packet> packet, const Address &dst,
                     uint16_t protocol) {
  NS_LOG_FUNCTION(this << packet << dst << protocol);
  NS_FATAL_ERROR(
      "TapBridge::Send: You may not call Send on a TapBridge directly");
  return false;
}

bool TapBridge::SendFrom(Ptr<Packet> packet, const Address &src,
                         const Address &dst, uint16_t protocol) {
  NS_LOG_FUNCTION(this << packet << src << dst << protocol);
  NS_FATAL_ERROR(
      "TapBridge::Send: You may not call SendFrom on a TapBridge directly");
  return false;
}

Ptr<Node> TapBridge::GetNode() const {
  NS_LOG_FUNCTION(this);
  return m_node;
}

void TapBridge::SetNode(Ptr<Node> node) {
  NS_LOG_FUNCTION(this);
  m_node = node;
}

bool TapBridge::NeedsArp() const {
  NS_LOG_FUNCTION(this);
  return true;
}

void TapBridge::SetReceiveCallback(NetDevice::ReceiveCallback cb) {
  NS_LOG_FUNCTION(this);
  m_rxCallback = cb;
}

void TapBridge::SetPromiscReceiveCallback(
    NetDevice::PromiscReceiveCallback cb) {
  NS_LOG_FUNCTION(this);
  m_promiscRxCallback = cb;
}

bool TapBridge::SupportsSendFrom() const {
  NS_LOG_FUNCTION(this);
  return true;
}

Address TapBridge::GetMulticast(Ipv6Address addr) const {
  NS_LOG_FUNCTION(this << addr);
  return Mac48Address::GetMulticast(addr);
}

} // namespace ns3
