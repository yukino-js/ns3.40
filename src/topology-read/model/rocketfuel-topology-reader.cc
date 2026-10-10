
#include "rocketfuel-topology-reader.h"

#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/node-container.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RocketfuelTopologyReader");

NS_OBJECT_ENSURE_REGISTERED(RocketfuelTopologyReader);

TypeId RocketfuelTopologyReader::GetTypeId() {
  static TypeId tid = TypeId("ns3::RocketfuelTopologyReader")
                          .SetParent<TopologyReader>()
                          .SetGroupName("TopologyReader")
                          .AddConstructor<RocketfuelTopologyReader>();
  return tid;
}

RocketfuelTopologyReader::RocketfuelTopologyReader() {
  m_linksNumber = 0;
  m_nodesNumber = 0;
  NS_LOG_FUNCTION(this);
}

RocketfuelTopologyReader::~RocketfuelTopologyReader() { NS_LOG_FUNCTION(this); }

#define START "^"
#define END "$"
#define SPACE "[ \t]+"
#define MAYSPACE "[ \t]*"

#define ROCKETFUEL_MAPS_LINE                                                   \
  START "(-*[0-9]+)" SPACE "(@[?A-Za-z0-9,+-]+)" SPACE "(\\+)*" MAYSPACE       \
        "(bb)*" MAYSPACE "\\(([0-9]+)\\)" SPACE "(&[0-9]+)*" MAYSPACE          \
        "->" MAYSPACE "(<[0-9 \t<>]+>)*" MAYSPACE                              \
        "(\\{-[0-9\\{\\} \t-]+\\})*" SPACE "=([A-Za-z0-9.!-]+)" SPACE          \
        "r([0-9])" MAYSPACE END

#define ROCKETFUEL_WEIGHTS_LINE                                                \
  START "([^ \t]+)" SPACE "([^ \t]+)" SPACE "([0-9.]+)" MAYSPACE END

static const std::regex rocketfuel_maps_regex(ROCKETFUEL_MAPS_LINE);

static const std::regex rocketfuel_weights_regex(ROCKETFUEL_WEIGHTS_LINE);

static inline void
PrintNodeInfo(std::string &uid, std::string &loc, bool dns, bool bb,
              std::vector<std::string>::size_type neighListSize,
              std::string &name, int radius) {
  NS_LOG_INFO("Load Node[" << uid << "]: location: " << loc << " dns: " << dns
                           << " bb: " << bb << " neighbors: " << neighListSize
                           << "("
                           << "%d"
                           << ") externals: \"%s\"(%d) "
                           << "name: " << name << " radius: " << radius);
}

NodeContainer RocketfuelTopologyReader::GenerateFromMapsFile(
    const std::vector<std::string> &argv) {
  std::string uid;
  std::string loc;
  std::string ptr;
  std::string name;
  bool dns = false;
  bool bb = false;
  int num_neigh_s = 0;
  unsigned int num_neigh = 0;
  int radius = 0;
  std::vector<std::string> neigh_list;
  NodeContainer nodes;

  uid = argv[0];
  loc = argv[1];

  if (!argv[2].empty()) {
    dns = true;
  }

  if (!argv[3].empty()) {
    bb = true;
  }

  num_neigh_s = std::stoi(argv[4]);
  if (num_neigh_s < 0) {
    num_neigh = 0;
    NS_LOG_WARN("Negative number of neighbors given");
  } else {
    num_neigh = num_neigh_s;
  }

  if (!argv[6].empty()) {
    std::string temp;
    std::regex replace_regex("[<|>]");
    std::regex_replace(std::back_inserter(temp), argv[6].begin(), argv[6].end(),
                       replace_regex, "");

    std::regex split_regex("[ |\t]");
    std::sregex_token_iterator first{temp.begin(), temp.end(), split_regex, -1};
    std::sregex_token_iterator last;
    neigh_list = std::vector<std::string>{first, last};
  }
  if (num_neigh != neigh_list.size()) {
    NS_LOG_WARN("Given number of neighbors = "
                << num_neigh
                << " != size of neighbors list = " << neigh_list.size());
  }

  if (!argv[7].empty()) {
  }

  if (!argv[8].empty()) {
    name = argv[8];
  }

  radius = std::atoi(&argv[9][1]);
  if (radius > 0) {
    return nodes;
  }

  PrintNodeInfo(uid, loc, dns, bb, neigh_list.size(), name, radius);

  if (!uid.empty()) {
    if (!m_nodeMap[uid]) {
      Ptr<Node> tmpNode = CreateObject<Node>();
      std::string nodename = "RocketFuelTopology/NodeName/" + uid;
      Names::Add(nodename, tmpNode);
      m_nodeMap[uid] = tmpNode;
      nodes.Add(tmpNode);
      m_nodesNumber++;
    }

    for (auto &nuid : neigh_list) {
      if (nuid.empty()) {
        return nodes;
      }

      if (!m_nodeMap[nuid]) {
        Ptr<Node> tmpNode = CreateObject<Node>();
        std::string nodename = "RocketFuelTopology/NodeName/" + nuid;
        Names::Add(nodename, tmpNode);
        m_nodeMap[nuid] = tmpNode;
        nodes.Add(tmpNode);
        m_nodesNumber++;
      }
      NS_LOG_INFO(m_linksNumber << ":" << m_nodesNumber << " From: " << uid
                                << " to: " << nuid);
      Link link(m_nodeMap[uid], uid, m_nodeMap[nuid], nuid);
      AddLink(link);
      m_linksNumber++;
    }
  }

  NS_LOG_INFO("Rocketfuel topology created with "
              << m_nodesNumber << " nodes and " << m_linksNumber << " links");

  return nodes;
}

NodeContainer RocketfuelTopologyReader::GenerateFromWeightsFile(
    const std::vector<std::string> &argv) {
  std::string sname;
  std::string tname;
  std::string::size_type endptr;
  NodeContainer nodes;

  sname = argv[0];
  tname = argv[1];
  std::stod(argv[2], &endptr);

  if (argv[2].size() != endptr) {
    NS_LOG_WARN("invalid weight: " << argv[2]);
    return nodes;
  }

  if (!sname.empty() && !tname.empty()) {
    if (!m_nodeMap[sname]) {
      Ptr<Node> tmpNode = CreateObject<Node>();
      std::string nodename = "RocketFuelTopology/NodeName/" + sname;
      Names::Add(nodename, tmpNode);
      m_nodeMap[sname] = tmpNode;
      nodes.Add(tmpNode);
      m_nodesNumber++;
    }

    if (!m_nodeMap[tname]) {
      Ptr<Node> tmpNode = CreateObject<Node>();
      std::string nodename = "RocketFuelTopology/NodeName/" + tname;
      Names::Add(nodename, tmpNode);
      m_nodeMap[tname] = tmpNode;
      nodes.Add(tmpNode);
      m_nodesNumber++;
    }
    NS_LOG_INFO(m_linksNumber << ":" << m_nodesNumber << " From: " << sname
                              << " to: " << tname);
    TopologyReader::ConstLinksIterator iter;
    bool found = false;
    for (iter = LinksBegin(); iter != LinksEnd(); iter++) {
      if ((iter->GetFromNode() == m_nodeMap[tname]) &&
          (iter->GetToNode() == m_nodeMap[sname])) {
        found = true;
        break;
      }
    }

    if (!found) {
      Link link(m_nodeMap[sname], sname, m_nodeMap[tname], tname);
      AddLink(link);
      m_linksNumber++;
    }
  }

  NS_LOG_INFO("Rocketfuel topology created with "
              << m_nodesNumber << " nodes and " << m_linksNumber << " links");

  return nodes;
}

RocketfuelTopologyReader::RF_FileType
RocketfuelTopologyReader::GetFileType(const std::string &line) {
  std::smatch matches;
  if (std::regex_match(line, matches, rocketfuel_maps_regex)) {
    return RF_MAPS;
  }

  if (std::regex_match(line, matches, rocketfuel_weights_regex)) {
    return RF_WEIGHTS;
  }

  return RF_UNKNOWN;
}

NodeContainer RocketfuelTopologyReader::Read() {
  std::ifstream topgen;
  topgen.open(GetFileName());
  NodeContainer nodes;

  std::istringstream lineBuffer;
  std::string line;
  int lineNumber = 0;
  RF_FileType ftype = RF_UNKNOWN;

  if (!topgen.is_open()) {
    NS_LOG_WARN("Couldn't open the file " << GetFileName());
    return nodes;
  }

  while (!topgen.eof()) {
    std::vector<std::string> argv;

    lineNumber++;
    line.clear();
    lineBuffer.clear();

    getline(topgen, line);

    if (lineNumber == 1) {
      ftype = GetFileType(line);
      if (ftype == RF_UNKNOWN) {
        NS_LOG_INFO("Unknown File Format (" << GetFileName() << ")");
        break;
      }
    }

    std::smatch matches;

    if (ftype == RF_MAPS) {
      bool ret = std::regex_match(line, matches, rocketfuel_maps_regex);
      if (!ret || matches.empty()) {
        NS_LOG_WARN("match failed (maps file): %s" << line);
        break;
      }
    } else if (ftype == RF_WEIGHTS) {
      bool ret = std::regex_match(line, matches, rocketfuel_weights_regex);
      if (!ret || matches.empty()) {
        NS_LOG_WARN("match failed (weights file): %s" << line);
        break;
      }
    }

    std::string matched_string;

    for (auto it = matches.begin() + 1; it != matches.end(); it++) {
      if (it->matched) {
        matched_string = it->str();
      } else {
        matched_string = "";
      }
      argv.push_back(matched_string);
    }

    if (ftype == RF_MAPS) {
      nodes.Add(GenerateFromMapsFile(argv));
    } else if (ftype == RF_WEIGHTS) {
      nodes.Add(GenerateFromWeightsFile(argv));
    } else {
      NS_LOG_WARN("Unsupported file format (only Maps/Weights are supported)");
    }
  }

  topgen.close();

  return nodes;
}

} // namespace ns3
