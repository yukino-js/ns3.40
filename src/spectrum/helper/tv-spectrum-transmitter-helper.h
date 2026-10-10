
#ifndef TV_SPECTRUM_TRANSMITTER_HELPER_H
#define TV_SPECTRUM_TRANSMITTER_HELPER_H

#include "ns3/object-factory.h"
#include <ns3/antenna-model.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device-container.h>
#include <ns3/net-device.h>
#include <ns3/node-container.h>
#include <ns3/non-communicating-net-device.h>
#include <ns3/random-variable-stream.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-phy.h>
#include <ns3/spectrum-signal-parameters.h>
#include <ns3/spectrum-value.h>
#include <ns3/tv-spectrum-transmitter.h>

class TvHelperDistributionTestCase;

namespace ns3 {

class TvSpectrumTransmitterHelper {
public:
  friend class ::TvHelperDistributionTestCase;

  enum Region { REGION_NORTH_AMERICA, REGION_JAPAN, REGION_EUROPE };

  enum Density { DENSITY_LOW, DENSITY_MEDIUM, DENSITY_HIGH };

  TvSpectrumTransmitterHelper();
  virtual ~TvSpectrumTransmitterHelper();

  void SetChannel(Ptr<SpectrumChannel> c);

  void SetAttribute(std::string name, const AttributeValue &val);

  NetDeviceContainer Install(NodeContainer nodes);

  NetDeviceContainer Install(NodeContainer nodes, Region region,
                             uint16_t channelNumber);

  NetDeviceContainer InstallAdjacent(NodeContainer nodes);

  NetDeviceContainer InstallAdjacent(NodeContainer nodes, Region region,
                                     uint16_t channelNumber);

  int64_t AssignStreams(int64_t streamNum);

  void CreateRegionalTvTransmitters(Region region, Density density,
                                    double originLatitude,
                                    double originLongitude, double maxAltitude,
                                    double maxRadius);

private:
  Ptr<SpectrumChannel> m_channel;

  std::list<int>
  GenerateRegionalTransmitterIndices(const double startFrequencies[],
                                     const int startFrequenciesLength,
                                     Density density);

  int GetRandomNumTransmitters(Density density, uint32_t numChannels);

  void
  InstallRandomRegionalTransmitters(Region region,
                                    std::list<int> transmitterIndicesToCreate,
                                    std::list<Vector> transmitterLocations);

  ObjectFactory m_factory;
  Ptr<UniformRandomVariable> m_uniRand;
};

} // namespace ns3

#endif
