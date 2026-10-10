
#include "ns2-mobility-helper.h"

#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/simulator.h"

#include <fstream>
#include <map>
#include <sstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ns2MobilityHelper");

#define NS2_AT "at"
#define NS2_X_COORD "X_"
#define NS2_Y_COORD "Y_"
#define NS2_Z_COORD "Z_"
#define NS2_SETDEST "setdest"
#define NS2_SET "set"
#define NS2_NODEID "$node_("
#define NS2_NS_SCH "$ns_"

struct ParseResult {
  std::vector<std::string> tokens;
  std::vector<int> ivals;
  std::vector<bool> has_ival;
  std::vector<double> dvals;
  std::vector<bool> has_dval;
  std::vector<std::string> svals;
};

struct DestinationPoint {
  Vector m_startPosition;
  Vector m_speed;
  Vector m_finalPosition;
  EventId m_stopEvent;
  double m_travelStartTime;
  double m_targetArrivalTime;
  DestinationPoint()
      : m_startPosition(Vector(0, 0, 0)), m_speed(Vector(0, 0, 0)),
        m_finalPosition(Vector(0, 0, 0)), m_travelStartTime(0),
        m_targetArrivalTime(0) {};
};

static ParseResult ParseNs2Line(const std::string &str);

static std::string TrimNs2Line(const std::string &str);

static bool IsNumber(const std::string &s);

template <class T> static bool IsVal(const std::string &str, T &ret);

static bool HasNodeIdNumber(std::string str);

static std::string GetNodeIdFromToken(std::string str);

static int GetNodeIdInt(ParseResult pr);

static std::string GetNodeIdString(ParseResult pr);

static Vector SetOneInitialCoord(Vector actPos, std::string &coord,
                                 double value);

static bool IsSetInitialPos(ParseResult pr);

static bool IsSchedSetPos(ParseResult pr);

static bool IsSchedMobilityPos(ParseResult pr);

static DestinationPoint SetMovement(Ptr<ConstantVelocityMobilityModel> model,
                                    Vector lastPos, double at,
                                    double xFinalPosition,
                                    double yFinalPosition, double speed);

static Vector SetInitialPosition(Ptr<ConstantVelocityMobilityModel> model,
                                 std::string coord, double coordVal);

static Vector SetSchedPosition(Ptr<ConstantVelocityMobilityModel> model,
                               double at, std::string coord, double coordVal);

Ns2MobilityHelper::Ns2MobilityHelper(std::string filename)
    : m_filename(filename) {
  std::ifstream file(m_filename, std::ios::in);
  if (!(file.is_open())) {
    NS_FATAL_ERROR("Could not open trace file "
                   << m_filename << " for reading, aborting here \n");
  }
}

Ptr<ConstantVelocityMobilityModel>
Ns2MobilityHelper::GetMobilityModel(std::string idString,
                                    const ObjectStore &store) const {
  std::istringstream iss;
  iss.str(idString);
  uint32_t id(0);
  iss >> id;
  Ptr<Object> object = store.Get(id);
  if (!object) {
    return nullptr;
  }
  Ptr<ConstantVelocityMobilityModel> model =
      object->GetObject<ConstantVelocityMobilityModel>();
  if (!model) {
    model = CreateObject<ConstantVelocityMobilityModel>();
    object->AggregateObject(model);
  }
  return model;
}

void Ns2MobilityHelper::ConfigNodesMovements(const ObjectStore &store) const {
  std::map<int, DestinationPoint> last_pos;

  std::ifstream file(m_filename, std::ios::in);
  if (file.is_open()) {
    while (!file.eof()) {
      int iNodeId = 0;
      std::string nodeId;
      std::string line;

      getline(file, line);

      if (line.empty()) {
        continue;
      }

      ParseResult pr = ParseNs2Line(line);

      if (pr.tokens.size() != 4) {
        continue;
      }

      nodeId = GetNodeIdString(pr);
      iNodeId = GetNodeIdInt(pr);
      if (iNodeId == -1) {
        NS_LOG_ERROR("Node number couldn't be obtained (corrupted file?): "
                     << line << "\n");
        continue;
      }

      Ptr<ConstantVelocityMobilityModel> model =
          GetMobilityModel(nodeId, store);

      if (!model) {
        NS_LOG_ERROR("Unknown node ID (corrupted file?): " << nodeId << "\n");
        continue;
      }

      if (IsSetInitialPos(pr)) {
        DestinationPoint point;
        point.m_finalPosition =
            SetInitialPosition(model, pr.tokens[2], pr.dvals[3]);
        last_pos[iNodeId] = point;

        NS_LOG_DEBUG("Positions after parse for node "
                     << iNodeId << " " << nodeId
                     << " position = " << last_pos[iNodeId].m_finalPosition);
      }
    }
    file.close();
  }

  file.open(m_filename, std::ios::in);
  if (file.is_open()) {
    while (!file.eof()) {
      int iNodeId = 0;
      std::string nodeId;
      std::string line;

      getline(file, line);

      if (line.empty()) {
        continue;
      }

      ParseResult pr = ParseNs2Line(line);

      if (pr.tokens.size() != 4 && pr.tokens.size() != 7 &&
          pr.tokens.size() != 8) {
        NS_LOG_ERROR(
            "Line has not correct number of parameters (corrupted file?): "
            << line << "\n");
        continue;
      }

      nodeId = GetNodeIdString(pr);
      iNodeId = GetNodeIdInt(pr);
      if (iNodeId == -1) {
        NS_LOG_ERROR("Node number couldn't be obtained (corrupted file?): "
                     << line << "\n");
        continue;
      }

      Ptr<ConstantVelocityMobilityModel> model =
          GetMobilityModel(nodeId, store);

      if (!model) {
        NS_LOG_ERROR("Unknown node ID (corrupted file?): " << nodeId << "\n");
        continue;
      }

      if (IsSetInitialPos(pr)) {
        continue;
      }

      else {
        double at;

        if (!IsNumber(pr.tokens[2])) {
          NS_LOG_WARN("Time is not a number: " << pr.tokens[2]);
          continue;
        }

        at = pr.dvals[2];

        if (at < 0) {
          NS_LOG_WARN("Time is less than cero: " << at);
          continue;
        }

        if (IsSchedMobilityPos(pr)) {
          if (last_pos[iNodeId].m_targetArrivalTime > at) {
            NS_LOG_LOGIC("Did not reach a destination! stoptime = "
                         << last_pos[iNodeId].m_targetArrivalTime
                         << ", at = " << at);
            double actuallytraveled = at - last_pos[iNodeId].m_travelStartTime;
            Vector reached =
                Vector(last_pos[iNodeId].m_startPosition.x +
                           last_pos[iNodeId].m_speed.x * actuallytraveled,
                       last_pos[iNodeId].m_startPosition.y +
                           last_pos[iNodeId].m_speed.y * actuallytraveled,
                       0);
            NS_LOG_LOGIC("Final point = " << last_pos[iNodeId].m_finalPosition
                                          << ", actually reached = "
                                          << reached);
            last_pos[iNodeId].m_stopEvent.Cancel();
            last_pos[iNodeId].m_finalPosition = reached;
          }
          last_pos[iNodeId] =
              SetMovement(model, last_pos[iNodeId].m_finalPosition, at,
                          pr.dvals[5], pr.dvals[6], pr.dvals[7]);

          NS_LOG_DEBUG("Positions after parse for node "
                       << iNodeId << " " << nodeId
                       << " position =" << last_pos[iNodeId].m_finalPosition);
        }

        else if (IsSchedSetPos(pr)) {
          last_pos[iNodeId].m_finalPosition =
              SetSchedPosition(model, at, pr.tokens[5], pr.dvals[6]);
          if (last_pos[iNodeId].m_targetArrivalTime > at) {
            last_pos[iNodeId].m_stopEvent.Cancel();
          }
          last_pos[iNodeId].m_targetArrivalTime = at;
          last_pos[iNodeId].m_travelStartTime = at;
          NS_LOG_DEBUG("Positions after parse for node "
                       << iNodeId << " " << nodeId
                       << " position =" << last_pos[iNodeId].m_finalPosition);
        } else {
          NS_LOG_WARN("Format Line is not correct: " << line << "\n");
        }
      }
    }
    file.close();
  }
}

ParseResult ParseNs2Line(const std::string &str) {
  ParseResult ret;
  std::istringstream s;
  std::string line;

  size_t pos_sharp = str.find_first_of('#');
  if (pos_sharp != std::string::npos) {
    line = str.substr(0, pos_sharp);
  } else {
    line = str;
  }

  line = TrimNs2Line(line);

  if (!HasNodeIdNumber(line)) {
    NS_LOG_WARN("Line has no node Id: " << line);
    return ret;
  }

  s.str(line);

  while (!s.eof()) {
    std::string x;
    s >> x;
    if (x.empty()) {
      continue;
    }
    ret.tokens.push_back(x);
    int ii(0);
    double d(0);
    if (HasNodeIdNumber(x)) {
      x = GetNodeIdFromToken(x);
    }
    ret.has_ival.push_back(IsVal<int>(x, ii));
    ret.ivals.push_back(ii);
    ret.has_dval.push_back(IsVal<double>(x, d));
    ret.dvals.push_back(d);
    ret.svals.push_back(x);
  }

  size_t tokensLength = ret.tokens.size();
  size_t lasTokenLength = ret.tokens[tokensLength - 1].size();

  if ((tokensLength == 7 || tokensLength == 8) &&
      (ret.tokens[tokensLength - 1][lasTokenLength - 1] == '"')) {
    ret.tokens[tokensLength - 1] =
        ret.tokens[tokensLength - 1].substr(0, lasTokenLength - 1);

    std::string x;
    x = ret.tokens[tokensLength - 1];

    if (HasNodeIdNumber(x)) {
      x = GetNodeIdFromToken(x);
    }

    int ii(0);
    double d(0);
    ret.has_ival[tokensLength - 1] = IsVal<int>(x, ii);
    ret.ivals[tokensLength - 1] = ii;
    ret.has_dval[tokensLength - 1] = IsVal<double>(x, d);
    ret.dvals[tokensLength - 1] = d;
    ret.svals[tokensLength - 1] = x;
  } else if ((tokensLength == 9 && ret.tokens[tokensLength - 1] == "\"") ||
             (tokensLength == 8 && ret.tokens[tokensLength - 1] == "\"")) {

    ret.tokens.erase(ret.tokens.begin() + tokensLength - 1);
    ret.has_ival.erase(ret.has_ival.begin() + tokensLength - 1);
    ret.ivals.erase(ret.ivals.begin() + tokensLength - 1);
    ret.has_dval.erase(ret.has_dval.begin() + tokensLength - 1);
    ret.dvals.erase(ret.dvals.begin() + tokensLength - 1);
    ret.svals.erase(ret.svals.begin() + tokensLength - 1);
  }

  return ret;
}

std::string TrimNs2Line(const std::string &s) {
  std::string ret = s;

  while (!ret.empty() && isblank(ret[0])) {
    ret.erase(0, 1);
  }

  while (!ret.empty() &&
         (isblank(ret[ret.size() - 1]) || (ret[ret.size() - 1] == ';'))) {
    ret.erase(ret.size() - 1, 1);
  }

  return ret;
}

bool IsNumber(const std::string &s) {
  char *endp;
  strtod(s.c_str(), &endp);
  return endp == s.c_str() + s.size();
}

template <class T> bool IsVal(const std::string &str, T &ret) {
  if (str.empty()) {
    return false;
  } else if (IsNumber(str)) {
    std::istringstream s(str);
    s >> ret;
    return true;
  } else {
    return false;
  }
}

bool HasNodeIdNumber(std::string str) {
  std::string::size_type startNodeId = str.find_first_of('(');
  std::string::size_type endNodeId = str.find_first_of(')');

  std::string nodeId;

  if (startNodeId == std::string::npos || endNodeId == std::string::npos) {
    return false;
  }

  nodeId = str.substr(startNodeId + 1, endNodeId - (startNodeId + 1));

  return IsNumber(nodeId) && nodeId.find_first_of('.') == std::string::npos &&
         nodeId[0] != '-';
}

std::string GetNodeIdFromToken(std::string str) {
  if (HasNodeIdNumber(str)) {
    std::string::size_type startNodeId = str.find_first_of('(');
    std::string::size_type endNodeId = str.find_first_of(')');

    return str.substr(startNodeId + 1, endNodeId - (startNodeId + 1));
  } else {
    return "";
  }
}

int GetNodeIdInt(ParseResult pr) {
  int result = -1;
  switch (pr.tokens.size()) {
  case 4:
    result = pr.ivals[0];
    break;
  case 7:
    result = pr.ivals[3];
    break;
  case 8:
    result = pr.ivals[3];
    break;
  default:
    result = -1;
  }
  return result;
}

std::string GetNodeIdString(ParseResult pr) {
  switch (pr.tokens.size()) {
  case 4:
    return pr.svals[0];
  case 7:
    return pr.svals[3];
  case 8:
    return pr.svals[3];
  default:
    return "";
  }
}

Vector SetOneInitialCoord(Vector position, std::string &coord, double value) {
  if (coord == NS2_X_COORD) {
    position.x = value;
    NS_LOG_DEBUG("X=" << value);
  } else if (coord == NS2_Y_COORD) {
    position.y = value;
    NS_LOG_DEBUG("Y=" << value);
  } else if (coord == NS2_Z_COORD) {
    position.z = value;
    NS_LOG_DEBUG("Z=" << value);
  }
  return position;
}

bool IsSetInitialPos(ParseResult pr) {
  return pr.tokens.size() == 4 && HasNodeIdNumber(pr.tokens[0]) &&
         pr.tokens[1] == NS2_SET && pr.has_dval[3] &&
         (pr.tokens[2] == NS2_X_COORD || pr.tokens[2] == NS2_Y_COORD ||
          pr.tokens[2] == NS2_Z_COORD);
}

bool IsSchedSetPos(ParseResult pr) {
  return pr.tokens.size() == 7 && pr.tokens[0] == NS2_NS_SCH &&
         pr.tokens[1] == NS2_AT && pr.tokens[4] == NS2_SET && pr.has_dval[2] &&
         pr.has_dval[3] &&
         (pr.tokens[5] == NS2_X_COORD || pr.tokens[5] == NS2_Y_COORD ||
          pr.tokens[5] == NS2_Z_COORD) &&
         pr.has_dval[2];
}

bool IsSchedMobilityPos(ParseResult pr) {
  return pr.tokens.size() == 8 && pr.tokens[0] == NS2_NS_SCH &&
         pr.tokens[1] == NS2_AT && pr.has_dval[2] && pr.has_dval[5] &&
         pr.has_dval[6] && pr.has_dval[7] && pr.tokens[4] == NS2_SETDEST;
}

DestinationPoint SetMovement(Ptr<ConstantVelocityMobilityModel> model,
                             Vector last_pos, double at, double xFinalPosition,
                             double yFinalPosition, double speed) {
  DestinationPoint retval;
  retval.m_startPosition = last_pos;
  retval.m_finalPosition = last_pos;
  retval.m_travelStartTime = at;
  retval.m_targetArrivalTime = at;

  if (speed == 0) {
    retval.m_stopEvent = Simulator::Schedule(
        Seconds(at), &ConstantVelocityMobilityModel::SetVelocity, model,
        Vector(0, 0, 0));
    return retval;
  }
  if (speed > 0) {
    double time =
        std::sqrt(std::pow(xFinalPosition - retval.m_finalPosition.x, 2) +
                  std::pow(yFinalPosition - retval.m_finalPosition.y, 2)) /
        speed;
    NS_LOG_DEBUG("at=" << at << " time=" << time);
    if (time == 0) {
      return retval;
    }
    double xSpeed = (xFinalPosition - retval.m_finalPosition.x) / time;
    double ySpeed = (yFinalPosition - retval.m_finalPosition.y) / time;
    retval.m_speed = Vector(xSpeed, ySpeed, 0);

    double zSpeed = 0;

    NS_LOG_DEBUG("Calculated Speed: X=" << xSpeed << " Y=" << ySpeed
                                        << " Z=" << zSpeed);

    Simulator::Schedule(Seconds(at),
                        &ConstantVelocityMobilityModel::SetVelocity, model,
                        Vector(xSpeed, ySpeed, zSpeed));
    retval.m_stopEvent = Simulator::Schedule(
        Seconds(at + time), &ConstantVelocityMobilityModel::SetVelocity, model,
        Vector(0, 0, 0));
    retval.m_finalPosition.x += xSpeed * time;
    retval.m_finalPosition.y += ySpeed * time;
    retval.m_targetArrivalTime += time;
  }
  return retval;
}

Vector SetInitialPosition(Ptr<ConstantVelocityMobilityModel> model,
                          std::string coord, double coordVal) {
  model->SetPosition(SetOneInitialCoord(model->GetPosition(), coord, coordVal));

  Vector position;
  position.x = model->GetPosition().x;
  position.y = model->GetPosition().y;
  position.z = model->GetPosition().z;

  return position;
}

Vector SetSchedPosition(Ptr<ConstantVelocityMobilityModel> model, double at,
                        std::string coord, double coordVal) {
  model->SetPosition(SetOneInitialCoord(model->GetPosition(), coord, coordVal));

  Vector position;
  position.x = model->GetPosition().x;
  position.y = model->GetPosition().y;
  position.z = model->GetPosition().z;

  Simulator::Schedule(Seconds(at), &ConstantVelocityMobilityModel::SetPosition,
                      model, position);

  return position;
}

void Ns2MobilityHelper::Install() const {
  Install(NodeList::Begin(), NodeList::End());
}

} // namespace ns3
