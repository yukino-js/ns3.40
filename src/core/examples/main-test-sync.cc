
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/global-value.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/realtime-simulator-impl.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

#include <chrono>
#include <thread>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TestSync");

namespace {

bool gFirstRun = false;

void inserted_function() {
  NS_ASSERT(gFirstRun);
  NS_LOG_UNCOND("inserted_function() called at "
                << Simulator::Now().GetSeconds() << " s");
}

void background_function() {
  NS_ASSERT(gFirstRun);
  NS_LOG_UNCOND("background_function() called at "
                << Simulator::Now().GetSeconds() << " s");
}

void first_function() {
  NS_LOG_UNCOND("first_function() called at " << Simulator::Now().GetSeconds()
                                              << " s");
  gFirstRun = true;
}

class FakeNetDevice {
public:
  FakeNetDevice();
  void Doit3();
};

FakeNetDevice::FakeNetDevice() { NS_LOG_FUNCTION_NOARGS(); }

void FakeNetDevice::Doit3() {
  NS_LOG_FUNCTION_NOARGS();
  std::this_thread::sleep_for(std::chrono::seconds(1));

  for (uint32_t i = 0; i < 10000; ++i) {
    Simulator::ScheduleWithContext(Simulator::NO_CONTEXT, Seconds(0.0),
                                   MakeEvent(&inserted_function));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void test() {
  GlobalValue::Bind("SimulatorImplementationType",
                    StringValue("ns3::RealtimeSimulatorImpl"));

  FakeNetDevice fnd;

  Simulator::ScheduleWithContext(0xffffffff, Seconds(0.0),
                                 MakeEvent(&first_function));

  for (double d = 0.; d < 14.999; d += 0.01) {
    Simulator::Schedule(Seconds(d), &background_function);
  }

  std::thread st3 = std::thread(&FakeNetDevice::Doit3, &fnd);

  Simulator::Stop(Seconds(15.0));
  Simulator::Run();

  if (st3.joinable()) {
    st3.join();
  }

  Simulator::Destroy();
}

} // namespace

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  while (true) {
    test();
  }

  return 0;
}
