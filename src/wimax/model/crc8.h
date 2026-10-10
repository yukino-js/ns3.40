#ifndef CRC8_H
#define CRC8_H
#include <stdint.h>

namespace ns3 {

uint8_t CRC8Calculate(const uint8_t *data, int length);

}

#endif
