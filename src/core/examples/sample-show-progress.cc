

#include "ns3/core-module.h"

#include <chrono>
#include <iomanip>
#include <string>
#include <thread>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SampleShowProgress");

namespace {

class Hold : public SimpleRefCount<Hold> {
public:
  Hold(Time wait, Time interval) {
    m_wait = wait;
    m_interval = interval;

    m_rng = CreateObject<ExponentialRandomVariable>();
    m_rng->SetAttribute("Mean", DoubleValue(m_wait.GetSeconds()));
  }

  Hold(Ptr<RandomVariableStream> rng) : m_rng(rng) {}

  void Event() {
    double delta = m_rng->GetValue();
    Time delay = Seconds(delta);
    NS_LOG_LOGIC("event delay: " << delay);

    Simulator::Schedule(delay, &Hold::Event, this);

    int64x64_t ratio = (Simulator::Now() / m_interval) / 10;
    bool even = (ratio.GetHigh() % 2);
    Time work = m_wait * (even ? 3 : 1);
    std::this_thread::sleep_for(
        std::chrono::nanoseconds(work.GetNanoSeconds()));
  }

private:
  Ptr<RandomVariableStream> m_rng;
  Time m_wait;
  Time m_interval;
};

} // namespace

int main(int argc, char **argv) {
  Time stop = Seconds(100);
  Time interval = Seconds(10);
  Time wait = MilliSeconds(10);
  bool verbose = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("stop", "Simulation duration in virtual time.", stop);
  cmd.AddValue("interval",
               "Approximate reporting interval, in wall clock time.", interval);
  cmd.AddValue("wait", "Wallclock time to burn on each event.", wait);
  cmd.AddValue("verbose", "Turn on verbose progress message.", verbose);
  cmd.Parse(argc, argv);

  std::cout << "\n"
            << cmd.GetName() << ":\n"
            << "\n"
            << "verbose progress message:  " << (verbose ? "on\n" : "off\n")
            << "target reporting interval: " << interval.As(Time::S) << "\n"
            << "average event sleep time:  " << wait.As(Time::MS) << "\n"
            << "total simulation run time: " << stop.As(Time::S) << std::endl;

  Ptr<Hold> h = Create<Hold>(wait, interval);
  h->Event();

  Simulator::Stop(stop);
  ShowProgress spinner(interval);
  spinner.SetVerbose(verbose);

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
