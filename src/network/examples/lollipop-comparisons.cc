
#include "ns3/lollipop-counter.h"

using namespace ns3;

int main(int argc, char *argv[]) {
  LollipopCounter8 counterA;
  LollipopCounter8 counterB;

  for (uint8_t i = 0; i < std::numeric_limits<uint8_t>::max(); i++) {
    std::cout << +i << " -- ";
    for (uint8_t j = 0; j < std::numeric_limits<uint8_t>::max(); j++) {
      counterA = i;
      counterB = j;
      if (counterA < counterB) {
        std::cout << "<";
      } else if (counterA == counterB) {
        std::cout << "=";
      } else if (counterA > counterB) {
        std::cout << ">";
      } else {
        std::cout << ".";
      }
    }
    std::cout << std::endl;
  }

  return 0;
}
