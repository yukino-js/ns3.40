
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>

namespace ns3 {

std::string BufferToString(uint8_t *buffer, uint32_t len) {
  std::ostringstream oss;
  oss.setf(std::ios::hex, std::ios::basefield);
  oss.fill('0');

  for (uint32_t i = 0; i < len; i++) {
    oss << ":" << std::setw(2) << (uint32_t)buffer[i];
  }
  return oss.str();
}

bool StringToBuffer(std::string s, uint8_t *buffer, uint32_t *len) {
  if ((s.length() % 3) != 0) {
    return false;
  }

  std::istringstream iss;
  iss.str(s);

  uint8_t n = 0;

  while (iss.good()) {
    char c;
    iss.read(&c, 1);
    if (c != ':') {
      return false;
    }

    uint32_t tmp;
    iss >> std::hex >> tmp;
    buffer[n] = tmp;
    n++;
  }

  *len = n;
  return true;
}

} // namespace ns3
