
#include "ns3/assert.h"
#include "ns3/callback.h"
#include "ns3/command-line.h"

#include <iostream>

using namespace ns3;

namespace {

double CbOne(double a, double b) {
  std::cout << "invoke cbOne a=" << a << ", b=" << b << std::endl;
  return a;
}

class MyCb {
public:
  int CbTwo(double a) {
    std::cout << "invoke cbTwo a=" << a << std::endl;
    return -5;
  }
};

} // namespace

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  Callback<double, double, double> one;
  one = MakeCallback(&CbOne);
  NS_ASSERT(!one.IsNull());
  double retOne;
  retOne = one(10.0, 20.0);
  NS_ASSERT(retOne == 10.0);

  Callback<int, double> two;
  MyCb cb;
  two = MakeCallback(&MyCb::CbTwo, &cb);
  NS_ASSERT(!two.IsNull());
  int retTwo;
  retTwo = two(10.0);
  NS_ASSERT(retTwo == -5);

  two = MakeNullCallback<int, double>();
  NS_ASSERT(two.IsNull());

#if 0
  two = MakeCallback (&CbOne);
#endif

#if 0
  Callback<void, float> three;
  three.Assign (MakeCallback (&CbOne));
#endif

  return 0;
}
