
#include "ns3/command-line.h"
#include "ns3/double.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"

#include <iostream>

using namespace ns3;

namespace {

class MyModel {
public:
  void Start();

private:
  void HandleEvent(double eventValue);
};

void MyModel::Start() {
  Simulator::Schedule(Seconds(10.0), &MyModel::HandleEvent, this,
                      Simulator::Now().GetSeconds());
}

void MyModel::HandleEvent(double value) {
  std::cout << "Member method received event at "
            << Simulator::Now().GetSeconds() << "s started at " << value << "s"
            << std::endl;
}

void ExampleFunction(MyModel *model) {
  std::cout << "ExampleFunction received event at "
            << Simulator::Now().GetSeconds() << "s" << std::endl;
  model->Start();
}

void RandomFunction() {
  std::cout << "RandomFunction received event at "
            << Simulator::Now().GetSeconds() << "s" << std::endl;
}

void CancelledEvent() {
  std::cout << "I should never be called... " << std::endl;
}

} // namespace

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  MyModel model;
  Ptr<UniformRandomVariable> v = CreateObject<UniformRandomVariable>();
  v->SetAttribute("Min", DoubleValue(10));
  v->SetAttribute("Max", DoubleValue(20));

  Simulator::Schedule(Seconds(10.0), &ExampleFunction, &model);

  Simulator::Schedule(Seconds(v->GetValue()), &RandomFunction);

  EventId id = Simulator::Schedule(Seconds(30.0), &CancelledEvent);
  Simulator::Cancel(id);

  Simulator::Schedule(Seconds(25.0), []() {
    std::cout << "Code within a lambda expression at time "
              << Simulator::Now().As(Time::S) << std::endl;
  });

  Simulator::Run();

  Simulator::Destroy();

  return 0;
}
