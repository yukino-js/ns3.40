
#ifndef BUILDINGS_HELPER_H
#define BUILDINGS_HELPER_H

#include <ns3/attribute.h>
#include <ns3/node-container.h>
#include <ns3/object-factory.h>
#include <ns3/ptr.h>

#include <string>

namespace ns3 {

class MobilityModel;
class Building;

class BuildingsHelper {
public:
  static void Install(Ptr<Node> node);
  static void Install(NodeContainer c);
};

} // namespace ns3

#endif
