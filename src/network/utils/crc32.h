#ifndef CRC32_H
#define CRC32_H
#include <stdint.h>

namespace ns3 {

uint32_t CRC32Calculate(const uint8_t *data, int length);

}

#endif
