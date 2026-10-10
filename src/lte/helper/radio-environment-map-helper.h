
#ifndef RADIO_ENVIRONMENT_MAP_HELPER_H
#define RADIO_ENVIRONMENT_MAP_HELPER_H

#include <ns3/object.h>

#include <fstream>

namespace ns3 {

class RemSpectrumPhy;
class Node;
class NetDevice;
class SpectrumChannel;
class MobilityModel;

class RadioEnvironmentMapHelper : public Object {
public:
  RadioEnvironmentMapHelper();
  ~RadioEnvironmentMapHelper() override;

  void DoDispose() override;
  static TypeId GetTypeId();

  uint16_t GetBandwidth() const;

  void SetBandwidth(uint16_t bw);

  void Install();

private:
  void DelayedInstall();

  void RunOneIteration(double xMin, double xMax, double yMin, double yMax);

  void PrintAndReset();

  void Finalize();

  struct RemPoint {
    Ptr<RemSpectrumPhy> phy;
    Ptr<MobilityModel> bmm;
  };

  std::list<RemPoint> m_rem;

  double m_xMin;
  double m_xMax;
  uint16_t m_xRes;
  double m_xStep;

  double m_yMin;
  double m_yMax;
  uint16_t m_yRes;
  double m_yStep;

  uint32_t m_maxPointsPerIteration;

  uint16_t m_earfcn;
  uint16_t m_bandwidth;

  double m_z;

  std::string m_channelPath;

  std::string m_outputFile;

  bool m_stopWhenDone;

  Ptr<SpectrumChannel> m_channel;

  double m_noisePower;

  std::ofstream m_outFile;

  bool m_useDataChannel;
  int32_t m_rbId;
};

} // namespace ns3

#endif
