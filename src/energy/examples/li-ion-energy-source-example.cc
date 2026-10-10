
#include "ns3/command-line.h"
#include "ns3/energy-source-container.h"
#include "ns3/li-ion-energy-source.h"
#include "ns3/log.h"
#include "ns3/simple-device-energy-model.h"
#include "ns3/simulator.h"

using namespace ns3;

static void PrintCellInfo(Ptr<LiIonEnergySource> es) {
  std::cout << "At " << Simulator::Now().As(Time::S)
            << " Cell voltage: " << es->GetSupplyVoltage()
            << " V Remaining Capacity: "
            << es->GetRemainingEnergy() / (3.6 * 3600) << " Ah" << std::endl;

  if (!Simulator::IsFinished()) {
    Simulator::Schedule(Seconds(20), &PrintCellInfo, es);
  }
}

int main(int argc, char **argv) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  Ptr<Node> node = CreateObject<Node>();

  Ptr<SimpleDeviceEnergyModel> sem = CreateObject<SimpleDeviceEnergyModel>();
  Ptr<EnergySourceContainer> esCont = CreateObject<EnergySourceContainer>();
  Ptr<LiIonEnergySource> es = CreateObject<LiIonEnergySource>();
  esCont->Add(es);
  es->SetNode(node);
  sem->SetEnergySource(es);
  es->AppendDeviceEnergyModel(sem);
  sem->SetNode(node);
  node->AggregateObject(esCont);

  Time now = Simulator::Now();

  sem->SetCurrentA(2.33);
  now += Seconds(1701);

  Simulator::Schedule(now, &SimpleDeviceEnergyModel::SetCurrentA, sem, 4.66);
  now += Seconds(600);

  PrintCellInfo(es);

  Simulator::Stop(now);
  Simulator::Run();
  Simulator::Destroy();

  DoubleValue v;
  es->GetAttribute("ThresholdVoltage", v);
  NS_ASSERT(es->GetSupplyVoltage() <= v.Get());

  return 0;
}
