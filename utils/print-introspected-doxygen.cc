

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/global-value.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/object-vector.h"
#include "ns3/object.h"
#include "ns3/pointer.h"
#include "ns3/simple-channel.h"
#include "ns3/string.h"
#include "ns3/system-path.h"

#include <algorithm>
#include <climits>
#include <iomanip>
#include <iostream>
#include <map>
#include <utility>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("PrintIntrospectedDoxygen");

namespace {
bool outputText = false;

std::string anchor;
std::string argument;
std::string boldStart;
std::string boldStop;
std::string breakBoth;
std::string breakHtmlOnly;
std::string breakTextOnly;
std::string brief;
std::string classStart;
std::string classStop;
std::string codeWord;
std::string commentStart;
std::string commentStop;
std::string copyDoc;
std::string file;
std::string flagSpanStart;
std::string flagSpanStop;
std::string functionStart;
std::string functionStop;
std::string headingStart;
std::string headingStop;
std::string hrefStart;
std::string hrefMid;
std::string hrefStop;
std::string indentHtmlOnly;
std::string listLineStart;
std::string listLineStop;
std::string listStart;
std::string listStop;
std::string note;
std::string page;
std::string reference;
std::string referenceNo;
std::string returns;
std::string sectionStart;
std::string seeAlso;
std::string subSectionStart;
std::string templArgDeduced;
std::string templArgExplicit;
std::string templateArgument;
std::string variable;

std::map<std::string, ns3::TypeId::AttributeInformation>
SortedAttributeInfo(const TypeId tid) {
  std::map<std::string, ns3::TypeId::AttributeInformation> index;
  for (uint32_t j = 0; j < tid.GetAttributeN(); j++) {
    struct TypeId::AttributeInformation info = tid.GetAttribute(j);
    index[info.name] = info;
  }
  return index;
}

std::map<std::string, ns3::TypeId::TraceSourceInformation>
SortedTraceSourceInfo(const TypeId tid) {
  std::map<std::string, ns3::TypeId::TraceSourceInformation> index;
  for (uint32_t j = 0; j < tid.GetTraceSourceN(); j++) {
    struct TypeId::TraceSourceInformation info = tid.GetTraceSource(j);
    index[info.name] = info;
  }
  return index;
}

} // namespace

void SetMarkup() {
  NS_LOG_FUNCTION(outputText);
  if (outputText) {
    anchor = "";
    argument = "  Arg: ";
    boldStart = "";
    boldStop = "";
    breakBoth = "\n";
    breakHtmlOnly = "";
    breakTextOnly = "\n";
    brief = "";
    classStart = "";
    classStop = "\n\n";
    codeWord = " ";
    commentStart =
        "===============================================================\n";
    commentStop = "";
    copyDoc = "  See: ";
    file = "File: introspected-doxygen.txt";
    flagSpanStart = "";
    flagSpanStop = "";
    functionStart = "";
    functionStop = "\n\n";
    headingStart = "";
    headingStop = "";
    hrefStart = "";
    hrefMid = "(see ";
    hrefStop = ")";
    indentHtmlOnly = "";
    listLineStart = "    * ";
    listLineStop = "";
    listStart = "";
    listStop = "";
    note = "Note: ";
    page = "Page ";
    reference = " ";
    referenceNo = " ";
    returns = "  Returns: ";
    sectionStart = "Section:  ";
    seeAlso = "  See: ";
    subSectionStart = "Subsection ";
    templArgDeduced = "[deduced]  ";
    templArgExplicit = "[explicit] ";
    templateArgument = "Template Arg: ";
    variable = "Variable: ";
  } else {
    anchor = "\\anchor ";
    argument = "\\param ";
    boldStart = "<b>";
    boldStop = "</b>";
    breakBoth = "<br>";
    breakHtmlOnly = "<br>";
    breakTextOnly = "";
    brief = "\\brief ";
    classStart = "\\class ";
    classStop = "";
    codeWord = "\\p ";
    commentStart = "/*!\n";
    commentStop = "*/\n";
    copyDoc = "\\copydoc ";
    file = "\\file";
    flagSpanStart = "<span class=\"mlabel\">";
    flagSpanStop = "</span>";
    functionStart = "\\fn ";
    functionStop = "";
    headingStart = "<h3>";
    headingStop = "</h3>";
    hrefStart = "[";
    hrefMid = "](\\ref ";
    hrefStop = ")";
    indentHtmlOnly = "  ";
    listLineStart = "<li>";
    listLineStop = "</li>";
    listStart = "<ul>";
    listStop = "</ul>";
    note = "\\note ";
    page = "\\page ";
    reference = " \\ref ";
    referenceNo = " %";
    returns = "\\returns ";
    sectionStart = "\\ingroup ";
    seeAlso = "\\see ";
    subSectionStart = "\\addtogroup ";
    templArgDeduced = "\\deduced ";
    templArgExplicit = "\\explicit ";
    templateArgument = "\\tparam ";
    variable = "\\var ";
  }
}

class StaticInformation {
public:
  void RecordAggregationInfo(std::string a, std::string b);
  void Gather(TypeId tid);
  void Print() const;

  std::vector<std::string> Get(TypeId tid) const;

  std::vector<std::string> GetNoTypeIds() const;

private:
  std::string GetCurrentPath() const;
  void DoGather(TypeId tid);
  void RecordOutput(TypeId tid);
  bool HasAlreadyBeenProcessed(TypeId tid) const;
  std::vector<std::pair<TypeId, std::string>> m_output;
  std::vector<std::string> m_currentPath;
  std::vector<TypeId> m_alreadyProcessed;
  std::vector<std::pair<TypeId, TypeId>> m_aggregates;
  mutable std::vector<std::string> m_noTids;
};

void StaticInformation::RecordAggregationInfo(std::string a, std::string b) {
  NS_LOG_FUNCTION(this << a << b);
  TypeId aTid;
  bool found = TypeId::LookupByNameFailSafe(a, &aTid);
  if (!found) {
    m_noTids.push_back(a);
    return;
  }
  TypeId bTid;
  found = TypeId::LookupByNameFailSafe(b, &bTid);
  if (!found) {
    m_noTids.push_back(b);
    return;
  }

  m_aggregates.emplace_back(aTid, bTid);
}

void StaticInformation::Print() const {
  NS_LOG_FUNCTION(this);
  for (const auto &item : m_output) {
    std::cout << item.first.GetName() << " -> " << item.second << std::endl;
  }
}

std::string StaticInformation::GetCurrentPath() const {
  NS_LOG_FUNCTION(this);
  std::ostringstream oss;
  for (const auto &item : m_currentPath) {
    oss << "/" << item;
  }
  return oss.str();
}

void StaticInformation::RecordOutput(TypeId tid) {
  NS_LOG_FUNCTION(this << tid);
  m_output.emplace_back(tid, GetCurrentPath());
}

bool StaticInformation::HasAlreadyBeenProcessed(TypeId tid) const {
  NS_LOG_FUNCTION(this << tid);
  for (const auto &it : m_alreadyProcessed) {
    if (it == tid) {
      return true;
    }
  }
  return false;
}

std::vector<std::string> StaticInformation::Get(TypeId tid) const {
  NS_LOG_FUNCTION(this << tid);
  std::vector<std::string> paths;
  for (const auto &item : m_output) {
    if (item.first == tid) {
      paths.push_back(item.second);
    }
  }
  return paths;
}

template <typename T> void Uniquefy(T t) {
  std::sort(t.begin(), t.end());
  t.erase(std::unique(t.begin(), t.end()), t.end());
}

std::vector<std::string> StaticInformation::GetNoTypeIds() const {
  NS_LOG_FUNCTION(this);
  Uniquefy(m_noTids);
  return m_noTids;
}

void StaticInformation::Gather(TypeId tid) {
  NS_LOG_FUNCTION(this << tid);
  DoGather(tid);
  Uniquefy(m_output);
}

void StaticInformation::DoGather(TypeId tid) {
  NS_LOG_FUNCTION(this << tid);
  if (HasAlreadyBeenProcessed(tid)) {
    return;
  }
  RecordOutput(tid);
  for (uint32_t i = 0; i < tid.GetAttributeN(); ++i) {
    struct TypeId::AttributeInformation info = tid.GetAttribute(i);
    const auto ptrChecker =
        dynamic_cast<const PointerChecker *>(PeekPointer(info.checker));
    if (ptrChecker != nullptr) {
      TypeId pointee = ptrChecker->GetPointeeTypeId();

      Ptr<Object> object = CreateObject<Object>();
      TypeId objectTypeId = object->GetTypeId();
      if (objectTypeId == pointee) {
        continue;
      }

      m_currentPath.push_back(info.name);
      m_alreadyProcessed.push_back(tid);
      DoGather(pointee);
      m_alreadyProcessed.pop_back();
      m_currentPath.pop_back();
      continue;
    }
    const auto vectorChecker = dynamic_cast<const ObjectPtrContainerChecker *>(
        PeekPointer(info.checker));
    if (vectorChecker != nullptr) {
      TypeId item = vectorChecker->GetItemTypeId();
      m_currentPath.push_back(info.name + "/[i]");
      m_alreadyProcessed.push_back(tid);
      DoGather(item);
      m_alreadyProcessed.pop_back();
      m_currentPath.pop_back();
      continue;
    }
  }
  for (uint32_t j = 0; j < TypeId::GetRegisteredN(); j++) {
    TypeId child = TypeId::GetRegistered(j);
    if (child.IsChildOf(tid)) {
      std::string childName = "$" + child.GetName();
      m_currentPath.push_back(childName);
      m_alreadyProcessed.push_back(tid);
      DoGather(child);
      m_alreadyProcessed.pop_back();
      m_currentPath.pop_back();
    }
  }
  for (const auto &item : m_aggregates) {
    if (item.first == tid || item.second == tid) {
      TypeId other;
      if (item.first == tid) {
        other = item.second;
      }
      if (item.second == tid) {
        other = item.first;
      }
      std::string name = "$" + other.GetName();
      m_currentPath.push_back(name);
      m_alreadyProcessed.push_back(tid);
      DoGather(other);
      m_alreadyProcessed.pop_back();
      m_currentPath.pop_back();
    }
  }
}

StaticInformation GetTypicalAggregations() {
  NS_LOG_FUNCTION_NOARGS();

  static StaticInformation info;
  static bool mapped = false;

  if (mapped) {
    return info;
  }

  mapped = true;

  info.RecordAggregationInfo("ns3::Node", "ns3::TcpSocketFactory");
  info.RecordAggregationInfo("ns3::Node", "ns3::UdpSocketFactory");
  info.RecordAggregationInfo("ns3::Node", "ns3::PacketSocketFactory");
  info.RecordAggregationInfo("ns3::Node", "ns3::MobilityModel");
  info.RecordAggregationInfo("ns3::Node", "ns3::Ipv4L3Protocol");
  info.RecordAggregationInfo("ns3::Node", "ns3::Ipv4NixVectorRouting");
  info.RecordAggregationInfo("ns3::Node", "ns3::Icmpv4L4Protocol");
  info.RecordAggregationInfo("ns3::Node", "ns3::ArpL3Protocol");
  info.RecordAggregationInfo("ns3::Node", "ns3::Icmpv4L4Protocol");
  info.RecordAggregationInfo("ns3::Node", "ns3::UdpL4Protocol");
  info.RecordAggregationInfo("ns3::Node", "ns3::Ipv6L3Protocol");
  info.RecordAggregationInfo("ns3::Node", "ns3::Icmpv6L4Protocol");
  info.RecordAggregationInfo("ns3::Node", "ns3::TcpL4Protocol");
  info.RecordAggregationInfo("ns3::Node", "ns3::RipNg");
  info.RecordAggregationInfo("ns3::Node", "ns3::GlobalRouter");
  info.RecordAggregationInfo("ns3::Node", "ns3::aodv::RoutingProtocol");
  info.RecordAggregationInfo("ns3::Node", "ns3::dsdv::RoutingProtocol");
  info.RecordAggregationInfo("ns3::Node", "ns3::dsr::DsrRouting");
  info.RecordAggregationInfo("ns3::Node", "ns3::olsr::RoutingProtocol");
  info.RecordAggregationInfo("ns3::Node", "ns3::EnergyHarvesterContainer");
  info.RecordAggregationInfo("ns3::Node", "ns3::EnergySourceContainer");

  Ptr<SimpleChannel> simpleChannel;
  simpleChannel = CreateObject<SimpleChannel>();

  for (uint32_t i = 0; i < Config::GetRootNamespaceObjectN(); ++i) {
    Ptr<Object> object = Config::GetRootNamespaceObject(i);
    info.Gather(object->GetInstanceTypeId());
  }

  return info;
}

typedef std::map<std::string, int32_t> NameMap;
typedef NameMap::const_iterator NameMapIterator;

NameMap GetNameMap() {
  NS_LOG_FUNCTION_NOARGS();

  static NameMap nameMap;
  static bool mapped = false;

  if (mapped) {
    return nameMap;
  }

  mapped = true;

  StaticInformation info = GetTypicalAggregations();

  for (uint32_t i = 0; i < TypeId::GetRegisteredN(); i++) {
    TypeId tid = TypeId::GetRegistered(i);
    if (tid.MustHideFromDocumentation()) {
      continue;
    }

    std::string name = tid.GetName();
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

    nameMap[name] = i;
  }

  std::vector<std::string> noTids = info.GetNoTypeIds();
  for (const auto &item : noTids) {
    nameMap[item] = -1;
  }

  return nameMap;
}

void PrintConfigPaths(std::ostream &os, const TypeId tid) {
  NS_LOG_FUNCTION(tid);
  std::vector<std::string> paths = GetTypicalAggregations().Get(tid);

  if (paths.empty()) {
    os << "Introspection did not find any typical Config paths." << breakBoth
       << std::endl;
  } else {
    os << headingStart << "Config Paths" << headingStop << std::endl;
    os << std::endl;
    os << tid.GetName() << " is accessible through the following paths"
       << " with Config::Set and Config::Connect:" << std::endl;
    os << listStart << std::endl;
    for (const auto &path : paths) {
      os << listLineStart << "\"" << path << "\"" << listLineStop
         << breakTextOnly << std::endl;
    }
    os << listStop << std::endl;
  }
}

void PrintAttributesTid(std::ostream &os, const TypeId tid) {
  NS_LOG_FUNCTION(tid);

  auto index = SortedAttributeInfo(tid);

  os << listStart << std::endl;
  for (const auto &[name, info] : index) {
    os << listLineStart << boldStart << name << boldStop << ": " << info.help
       << std::endl;
    os << indentHtmlOnly << listStart << std::endl;
    os << "    " << listLineStart << "Set with class: " << reference
       << info.checker->GetValueTypeName() << listLineStop << std::endl;

    std::string underType;
    if (info.checker->HasUnderlyingTypeInformation()) {
      os << "    " << listLineStart << "Underlying type: ";

      std::string valType = info.checker->GetValueTypeName();
      underType = info.checker->GetUnderlyingTypeInformation();
      bool handled = false;
      if ((valType != "ns3::EnumValue") && (underType != "std::string")) {
        if (valType == "ns3::PointerValue") {
          const auto ptrChecker =
              dynamic_cast<const PointerChecker *>(PeekPointer(info.checker));
          if (ptrChecker != nullptr) {
            os << reference << "ns3::Ptr"
               << "< " << reference << ptrChecker->GetPointeeTypeId().GetName()
               << ">";
            handled = true;
          }
        } else if (valType == "ns3::ObjectPtrContainerValue") {
          const auto ptrChecker =
              dynamic_cast<const ObjectPtrContainerChecker *>(
                  PeekPointer(info.checker));
          if (ptrChecker != nullptr) {
            os << reference << "ns3::Ptr"
               << "< " << reference << ptrChecker->GetItemTypeId().GetName()
               << ">";
            handled = true;
          }
        }

        auto match = [&uType = std::as_const(underType)](const std::string &s) {
          return uType.rfind(s, 0) == 0;
        };

        if (match("bool") || match("double") || match("int8_t") ||
            match("uint8_t") || match("int16_t") || match("uint16_t") ||
            match("int32_t") || match("uint32_t") || match("int64_t") ||
            match("uint64_t")) {
          os << underType;
          handled = true;
        }
      }
      if (!handled) {
        os << codeWord << underType;
      }
      os << listLineStop << std::endl;
    }
    if (info.flags & TypeId::ATTR_CONSTRUCT && info.accessor->HasSetter()) {
      std::string value = info.initialValue->SerializeToString(info.checker);
      if (underType == "std::string" && value.empty()) {
        value = "\"\"";
      }
      os << "    " << listLineStart << "Initial value: " << value
         << listLineStop << std::endl;
    }
    bool moreFlags{false};
    os << "    " << listLineStart << "Flags: ";
    if (info.flags & TypeId::ATTR_CONSTRUCT && info.accessor->HasSetter()) {
      os << flagSpanStart << "construct" << flagSpanStop;
      moreFlags = true;
    }
    if (info.flags & TypeId::ATTR_SET && info.accessor->HasSetter()) {
      os << (outputText && moreFlags ? ", " : "") << flagSpanStart << "write"
         << flagSpanStop;
      moreFlags = true;
    }
    if (info.flags & TypeId::ATTR_GET && info.accessor->HasGetter()) {
      os << (outputText && moreFlags ? ", " : "") << flagSpanStart << "read"
         << flagSpanStop;
      moreFlags = true;
    }
    os << listLineStop << std::endl;
    os << indentHtmlOnly << listStop << std::endl;
  }
  os << listStop << std::endl;
}

void PrintAttributes(std::ostream &os, const TypeId tid) {
  NS_LOG_FUNCTION(tid);
  if (tid.GetAttributeN() == 0) {
    os << "No Attributes are defined for this type." << breakBoth << std::endl;
  } else {
    os << headingStart << "Attributes" << headingStop << std::endl;
    PrintAttributesTid(os, tid);
  }

  TypeId tmp = tid.GetParent();
  while (tmp.GetParent() != tmp) {
    if (tmp.GetAttributeN() != 0) {
      os << headingStart << "Attributes defined in parent class "
         << tmp.GetName() << headingStop << std::endl;
      PrintAttributesTid(os, tmp);
    }
    tmp = tmp.GetParent();
  }
}

void PrintTraceSourcesTid(std::ostream &os, const TypeId tid) {
  NS_LOG_FUNCTION(tid);

  auto index = SortedTraceSourceInfo(tid);

  os << listStart << std::endl;
  for (const auto &[name, info] : index) {
    os << listLineStart << boldStart << name << boldStop << ": " << info.help
       << breakBoth;
    if (!outputText) {
      os << "%";
    }
    os << "Callback signature: " << info.callback << std::endl;
    os << listLineStop << std::endl;
  }
  os << listStop << std::endl;
}

void PrintTraceSources(std::ostream &os, const TypeId tid) {
  NS_LOG_FUNCTION(tid);
  if (tid.GetTraceSourceN() == 0) {
    os << "No TraceSources are defined for this type." << breakBoth
       << std::endl;
  } else {
    os << headingStart << "TraceSources" << headingStop << std::endl;
    PrintTraceSourcesTid(os, tid);
  }

  TypeId tmp = tid.GetParent();
  while (tmp.GetParent() != tmp) {
    if (tmp.GetTraceSourceN() != 0) {
      os << headingStart << "TraceSources defined in parent class "
         << tmp.GetName() << headingStop << std::endl;
      PrintTraceSourcesTid(os, tmp);
    }
    tmp = tmp.GetParent();
  }
}

void PrintSize(std::ostream &os, const TypeId tid) {
  NS_LOG_FUNCTION(tid);
  NS_ASSERT_MSG(CHAR_BIT != 0, "CHAR_BIT is zero");

  std::size_t arch = (sizeof(void *) * CHAR_BIT);

  os << boldStart << "Size" << boldStop << " of this type is " << tid.GetSize()
     << " bytes (on a " << arch << "-bit architecture)." << std::endl;
}

void PrintTypeIdBlocks(std::ostream &os) {
  NS_LOG_FUNCTION_NOARGS();

  NameMap nameMap = GetNameMap();

  for (const auto &item : nameMap) {
    if (item.second < 0) {
      continue;
    }
    TypeId tid = TypeId::GetRegistered(item.second);
    std::string name = tid.GetName();

    std::cout << commentStart << std::endl;

    std::cout << classStart << name << std::endl;
    std::cout << std::endl;

    PrintConfigPaths(std::cout, tid);
    PrintAttributes(std::cout, tid);
    PrintTraceSources(std::cout, tid);
    PrintSize(std::cout, tid);

    std::cout << commentStop << std::endl;
  }
}

void PrintAllTypeIds(std::ostream &os) {
  NS_LOG_FUNCTION_NOARGS();
  os << commentStart << page << "TypeIdList All ns3::TypeId's\n" << std::endl;
  os << "This is a list of all" << reference << "ns3::TypeId's.\n"
     << "For more information see the" << reference << "ns3::TypeId "
     << "section of this API documentation and the" << referenceNo
     << "TypeId section "
     << "in the Configuration and " << referenceNo
     << "Attributes chapter of the Manual.\n"
     << std::endl;

  os << listStart << std::endl;

  NameMap nameMap = GetNameMap();
  for (const auto &item : nameMap) {
    if (item.second < 0) {
      continue;
    }
    TypeId tid = TypeId::GetRegistered(item.second);

    os << indentHtmlOnly << listLineStart << boldStart << tid.GetName()
       << boldStop << listLineStop << std::endl;
  }
  os << listStop << std::endl;
  os << commentStop << std::endl;
}

void PrintAllAttributes(std::ostream &os) {
  NS_LOG_FUNCTION_NOARGS();
  os << commentStart << page << "AttributeList All Attributes\n" << std::endl;
  os << "This is a list of all" << reference << "attributes classes.  "
     << "For more information see the" << reference << "attributes "
     << "section of this API documentation and the Attributes sections "
     << "in the Tutorial and Manual.\n"
     << std::endl;

  NameMap nameMap = GetNameMap();
  for (const auto &item : nameMap) {
    if (item.second < 0) {
      continue;
    }
    TypeId tid = TypeId::GetRegistered(item.second);

    if (tid.GetAttributeN() == 0) {
      continue;
    }

    auto index = SortedAttributeInfo(tid);

    os << boldStart << tid.GetName() << boldStop << breakHtmlOnly << std::endl;
    os << listStart << std::endl;
    for (const auto &[name, info] : index) {
      os << listLineStart << boldStart << name << boldStop << ": " << info.help
         << listLineStop << std::endl;
    }
    os << listStop << std::endl;
  }
  os << commentStop << std::endl;
}

void PrintAllGlobals(std::ostream &os) {
  NS_LOG_FUNCTION_NOARGS();
  os << commentStart << page << "GlobalValueList All GlobalValues\n"
     << std::endl;
  os << "This is a list of all" << reference << "ns3::GlobalValue instances.\n"
     << "See ns3::GlobalValue for how to set these." << std::endl;

  os << listStart << std::endl;
  for (auto i = GlobalValue::Begin(); i != GlobalValue::End(); ++i) {
    StringValue val;
    (*i)->GetValue(val);
    os << indentHtmlOnly << listLineStart << boldStart << hrefStart
       << (*i)->GetName() << hrefMid << "GlobalValue" << (*i)->GetName()
       << hrefStop << boldStop << ": " << (*i)->GetHelp()
       << ".  Default value: " << val.Get() << "." << listLineStop << std::endl;
  }
  os << listStop << std::endl;
  os << commentStop << std::endl;
}

void PrintAllLogComponents(std::ostream &os) {
  NS_LOG_FUNCTION_NOARGS();
  os << commentStart << page << "LogComponentList All LogComponents\n"
     << std::endl;
  os << "This is a list of all" << reference << "ns3::LogComponent instances.\n"
     << std::endl;

  LogComponent::ComponentList *logs = LogComponent::GetComponentList();
  std::size_t widthL = std::string("Log Component").size();
  std::size_t widthR = std::string("file").size();
  for (const auto &it : (*logs)) {
    widthL = std::max(widthL, it.first.size());
    std::string file = it.second->File();
    while (file.find("../") == 0) {
      file = file.substr(3);
    }
    widthR = std::max(widthR, file.size());
  }
  const std::string tLeft("| ");
  const std::string tMid(" | ");
  const std::string tRight(" |");

  os << tLeft << std::setw(widthL) << std::left << "Log Component" << tMid
     << std::setw(widthR) << std::left << "File" << tRight << std::endl;
  os << tLeft << ":" << std::string(widthL - 1, '-') << tMid << ":"
     << std::string(widthR - 1, '-') << tRight << std::endl;

  for (const auto &it : (*logs)) {
    std::string file = it.second->File();
    while (file.find("../") == 0) {
      file = file.substr(3);
    }

    os << tLeft << std::setw(widthL) << std::left << it.first << tMid
       << std::setw(widthR) << file << tRight << std::endl;
  }
  os << std::right << std::endl;
  os << commentStop << std::endl;
}

void PrintAllTraceSources(std::ostream &os) {
  NS_LOG_FUNCTION_NOARGS();
  os << commentStart << page << "TraceSourceList All TraceSources\n"
     << std::endl;
  os << "This is a list of all" << reference << "tracing sources.  "
     << "For more information see the " << reference << "tracing "
     << "section of this API documentation and the Tracing sections "
     << "in the Tutorial and Manual.\n"
     << std::endl;

  NameMap nameMap = GetNameMap();

  for (const auto &item : nameMap) {
    if (item.second < 0) {
      continue;
    }
    TypeId tid = TypeId::GetRegistered(item.second);

    if (tid.GetTraceSourceN() == 0) {
      continue;
    }

    auto index = SortedTraceSourceInfo(tid);

    os << boldStart << tid.GetName() << boldStop << breakHtmlOnly << std::endl;

    os << listStart << std::endl;
    for (const auto &[name, info] : index) {
      os << listLineStart << boldStart << name << boldStop << ": " << info.help
         << listLineStop << std::endl;
    }
    os << listStop << std::endl;
  }
  os << commentStop << std::endl;
}

void PrintAttributeValueSection(std::ostream &os, const std::string &name,
                                const bool seeBase = true) {
  NS_LOG_FUNCTION(name);
  std::string section = "attribute_" + name;

  os << commentStart << sectionStart << "attributes\n"
     << subSectionStart << "attribute_" << name << " " << name << " Attribute\n"
     << "AttributeValue implementation for " << name << "\n";
  if (seeBase) {
    if (name != "IeMeshId") {
      os << seeAlso << "ns3::" << name << "\n";
    } else {
      os << seeAlso << "ns3::dot11s::" << name << "\n";
    }
  }
  os << commentStop;
}

void PrintAttributeValueWithName(std::ostream &os, const std::string &name,
                                 const std::string &type,
                                 const std::string &header) {
  NS_LOG_FUNCTION(name << type << header);
  std::string sectAttr = sectionStart + "attribute_" + name;

  std::string valClass = name + "Value";
  std::string qualClass = " ns3::" + valClass;

  os << commentStart << sectAttr << std::endl;
  os << classStart << qualClass << " \"" << header << "\"" << std::endl;
  os << "AttributeValue implementation for " << name << "." << std::endl;
  os << seeAlso << "AttributeValue" << std::endl;
  os << commentStop;

  os << commentStart << functionStart << name << qualClass << "::" << valClass;
  if ((name == "EmptyAttribute") || (name == "ObjectPtrContainer")) {
    os << "()\n";
  } else {
    os << "(const " << type << " & value)\n"
       << "Copy constructor.\n"
       << argument << "[in] value The " << name << " value to copy.\n";
  }
  os << commentStop;

  os << commentStart << functionStart << type << qualClass << "::Get () const\n"
     << returns << "The " << name << " value.\n"
     << commentStop;

  os << commentStart << functionStart << "bool" << qualClass
     << "::GetAccessor (T & value) const\n"
     << "Access the " << name << " value as type " << codeWord << "T.\n"
     << templateArgument << "T " << templArgExplicit << "The type to cast to.\n"
     << argument << "[out] value The " << name << " value, as type " << codeWord
     << "T.\n"
     << returns << "true.\n"
     << commentStop;

  if (type != "Callback") {
    os << commentStart << functionStart << "void" << qualClass
       << "::Set (const " << type << " & value)\n"
       << "Set the value.\n"
       << argument << "[in] value The value to adopt.\n"
       << commentStop;
  }

  os << commentStart << variable << type << qualClass << "::m_value\n"
     << "The stored " << name << " instance.\n"
     << commentStop << std::endl;
}

void PrintMakeAccessors(std::ostream &os, const std::string &name) {
  NS_LOG_FUNCTION(name);
  std::string sectAttr = sectionStart + "attribute_" + name + "\n";
  std::string make = "ns3::Make" + name + "Accessor ";

  os << commentStart << sectAttr << functionStart
     << "ns3::Ptr<const ns3::AttributeAccessor> " << make << "(T1 a1)\n"
     << copyDoc << "ns3::MakeAccessorHelper(T1)\n"
     << seeAlso << "AttributeAccessor\n"
     << commentStop;

  os << commentStart << sectAttr << functionStart
     << "ns3::Ptr<const ns3::AttributeAccessor> " << make << "(T1 a1, T2 a2)\n"
     << copyDoc << "ns3::MakeAccessorHelper(T1,T2)\n"
     << seeAlso << "AttributeAccessor\n"
     << commentStop;
}

void PrintMakeChecker(std::ostream &os, const std::string &name,
                      const std::string &header) {
  NS_LOG_FUNCTION(name << header);
  std::string sectAttr = sectionStart + "attribute_" + name + "\n";
  std::string make = "ns3::Make" + name + "Checker ";

  os << commentStart << sectAttr << std::endl;
  os << classStart << " ns3::" << name << "Checker"
     << " \"" << header << "\"" << std::endl;
  os << "AttributeChecker implementation for " << name << "Value." << std::endl;
  os << seeAlso << "AttributeChecker" << std::endl;
  os << commentStop;

  os << commentStart << sectAttr << functionStart
     << "ns3::Ptr<const ns3::AttributeChecker> " << make << "()\n"
     << returns << "The AttributeChecker.\n"
     << seeAlso << "AttributeChecker\n"
     << commentStop;
}

struct AttributeDescriptor {
  const std::string m_name;
  const std::string m_type;
  const bool m_seeBase;
  const std::string m_header;
};

void PrintAttributeHelper(std::ostream &os, const AttributeDescriptor &attr) {
  NS_LOG_FUNCTION(attr.m_name << attr.m_type << attr.m_seeBase
                              << attr.m_header);
  PrintAttributeValueSection(os, attr.m_name, attr.m_seeBase);
  PrintAttributeValueWithName(os, attr.m_name, attr.m_type, attr.m_header);
  PrintMakeAccessors(os, attr.m_name);
  PrintMakeChecker(os, attr.m_name, attr.m_header);
}

void PrintAttributeImplementations(std::ostream &os) {
  NS_LOG_FUNCTION_NOARGS();

  // clang-format off
  const AttributeDescriptor attributes [] =
    {
      { "Address",        "Address",        true,  "address.h"          },
      { "Box",            "Box",            true,  "box.h"              },
      { "DataRate",       "DataRate",       true,  "data-rate.h"        },
      { "Length",         "Length",         true,  "length.h"           },
      { "IeMeshId",       "IeMeshId",       true,  "ie-dot11s-id.h"     },
      { "Ipv4Address",    "Ipv4Address",    true,  "ipv4-address.h"     },
      { "Ipv4Mask",       "Ipv4Mask",       true,  "ipv4-address.h"     },
      { "Ipv6Address",    "Ipv6Address",    true,  "ipv6-address.h"     },
      { "Ipv6Prefix",     "Ipv6Prefix",     true,  "ipv6-address.h"     },
      { "Mac16Address",   "Mac16Address",   true,  "mac16-address.h"    },
      { "Mac48Address",   "Mac48Address",   true,  "mac48-address.h"    },
      { "Mac64Address",   "Mac64Address",   true,  "mac64-address.h"    },
      { "ObjectFactory",  "ObjectFactory",  true,  "object-factory.h"   },
      { "OrganizationIdentifier",
                          "OrganizationIdentifier",
                                            true,  "vendor-specific-action.h" },
      { "Priomap",        "Priomap",        true,  "prio-queue-disc.h"  },
      { "QueueSize",      "QueueSize",      true,  "queue-size.h"       },
      { "Rectangle",      "Rectangle",      true,  "rectangle.h"        },
      { "Ssid",           "Ssid",           true,  "ssid.h"             },
      { "TypeId",         "TypeId",         true,  "type-id.h"          },
      { "UanModesList",   "UanModesList",   true,  "uan-tx-mode.h"      },
      { "ValueClassTest", "ValueClassTest", false, "attribute-test-suite.cc"  },
      { "Vector",         "Vector",         true,  "vector.h"           },
      { "Vector2D",       "Vector2D",       true,  "vector.h"           },
      { "Vector3D",       "Vector3D",       true,  "vector.h"           },
      { "Waypoint",       "Waypoint",       true,  "waypoint.h"         },
      { "WifiMode",       "WifiMode",       true,  "wifi-mode.h"        },

      { "Boolean",        "bool",           false, "boolean.h"          },
      { "Callback",       "Callback",       true,  "callback.h"         },
      { "Double",         "double",         false, "double.h"           },
      { "Enum",           "int",            false, "enum.h"             },
      { "Integer",        "int64_t",        false, "integer.h"          },
      { "Pointer",        "Pointer",        false, "pointer.h"          },
      { "String",         "std::string",    false, "string.h"           },
      { "Time",           "Time",           true,  "nstime.h"           },
      { "Uinteger",       "uint64_t",       false, "uinteger.h"         },
      { "",               "",               false, "last placeholder"   }
    };
  // clang-format on

  int i = 0;
  while (!attributes[i].m_name.empty()) {
    PrintAttributeHelper(os, attributes[i]);
    ++i;
  }

  PrintAttributeValueSection(os, "EmptyAttribute", false);
  PrintAttributeValueWithName(os, "EmptyAttribute", "EmptyAttribute",
                              "attribute.h");

  PrintAttributeValueSection(os, "ObjectPtrContainer", false);
  PrintAttributeValueWithName(os, "ObjectPtrContainer", "ObjectPtrContainer",
                              "object-ptr-container.h");
  PrintMakeChecker(os, "ObjectPtrContainer", "object-ptr-container.h");

  PrintAttributeValueSection(os, "ObjectVector", false);
  PrintMakeAccessors(os, "ObjectVector");
  PrintMakeChecker(os, "ObjectVector", "object-vector.h");

  PrintAttributeValueSection(os, "ObjectMap", false);
  PrintMakeAccessors(os, "ObjectMap");
  PrintMakeChecker(os, "ObjectMap", "object-map.h");

  PrintAttributeValueSection(os, "Pair", false);
  PrintAttributeValueWithName(os, "Pair", "std::pair<A, B>", "pair.h");
  PrintMakeChecker(os, "Pair", "pair.h");

  PrintAttributeValueSection(os, "Tuple", false);
  PrintAttributeValueWithName(os, "Tuple", "std::tuple<Args...>", "tuple.h");
  PrintMakeChecker(os, "Tuple", "tuple.h");

  PrintMakeChecker(os, "AttributeContainer", "attribute-container.h");
}

int main(int argc, char *argv[]) {
  NS_LOG_FUNCTION_NOARGS();

  CommandLine cmd(__FILE__);
  cmd.Usage("Generate documentation for all ns-3 registered types, "
            "trace sources, attributes and global variables.");
  cmd.AddValue("output-text", "format output as plain text", outputText);
  cmd.Parse(argc, argv);

  SetMarkup();

  NodeContainer c;
  c.Create(1);

  std::cout << std::endl;
  std::cout << commentStart << file << "\n"
            << sectionStart << "utils\n"
            << "Doxygen docs generated from the TypeId database.\n"
            << note << "This file is automatically generated by " << codeWord
            << "print-introspected-doxygen.cc. Do not edit this file! "
            << "Edit that file instead.\n"
            << commentStop << std::endl;

  PrintTypeIdBlocks(std::cout);

  PrintAllTypeIds(std::cout);
  PrintAllAttributes(std::cout);
  PrintAllGlobals(std::cout);
  PrintAllLogComponents(std::cout);
  PrintAllTraceSources(std::cout);
  PrintAttributeImplementations(std::cout);

  return 0;
}
