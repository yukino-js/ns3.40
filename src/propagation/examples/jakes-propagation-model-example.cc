#include "ns3/core-module.h"
#include "ns3/jakes-propagation-loss-model.h"
#include "ns3/mobility-module.h"

#include <cmath>
#include <vector>

using namespace ns3;

class JakesPropagationExample {
public:
  JakesPropagationExample();
  ~JakesPropagationExample();

private:
  Ptr<PropagationLossModel> m_loss;
  Ptr<MobilityModel> m_firstMobility;
  Ptr<MobilityModel> m_secondMobility;
  Time m_step;
  EventId m_nextEvent;
  void Next();
};

JakesPropagationExample::JakesPropagationExample() : m_step(Seconds(0.0002)) {
  m_loss = CreateObject<JakesPropagationLossModel>();
  m_firstMobility = CreateObject<ConstantPositionMobilityModel>();
  m_secondMobility = CreateObject<ConstantPositionMobilityModel>();
  m_firstMobility->SetPosition(Vector(0, 0, 0));
  m_secondMobility->SetPosition(Vector(10, 0, 0));
  m_nextEvent =
      Simulator::Schedule(m_step, &JakesPropagationExample::Next, this);
}

JakesPropagationExample::~JakesPropagationExample() {}

void JakesPropagationExample::Next() {
  m_nextEvent =
      Simulator::Schedule(m_step, &JakesPropagationExample::Next, this);
  std::cout << Simulator::Now().As(Time::MS) << " "
            << m_loss->CalcRxPower(0, m_firstMobility, m_secondMobility)
            << std::endl;
}

int main(int argc, char *argv[]) {
  Config::SetDefault("ns3::JakesProcess::NumberOfOscillators",
                     UintegerValue(100));
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);
  JakesPropagationExample example;
  Simulator::Stop(Seconds(1000));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
