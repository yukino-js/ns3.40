
#include "ns3/bit-serializer.h"

#include "ns3/bit-deserializer.h"

#include <iostream>

using namespace ns3;

int main() {
  BitSerializer testBitSerializer1;

  testBitSerializer1.PushBits(0x55, 7);
  testBitSerializer1.PushBits(0x7, 3);
  testBitSerializer1.PushBits(0x0, 2);

  std::vector<uint8_t> result = testBitSerializer1.GetBytes();

  std::cout << "Result:    ";
  for (std::size_t i = 0; i < result.size(); i++) {
    std::cout << std::hex << int(result[i]) << " ";
  }
  std::cout << std::endl;
  std::cout << "Expecting: ab c0" << std::endl;

  BitSerializer testBitSerializer2;

  testBitSerializer2.PushBits(0x55, 7);
  testBitSerializer2.PushBits(0x7, 3);
  testBitSerializer2.PushBits(0x0, 2);

  testBitSerializer2.InsertPaddingAtEnd(false);

  result = testBitSerializer2.GetBytes();

  std::cout << "Result:    ";
  for (std::size_t i = 0; i < result.size(); i++) {
    std::cout << std::hex << int(result[i]) << " ";
  }
  std::cout << std::endl;
  std::cout << "Expecting: a bc" << std::endl;

  BitDeserializer testBitDeserializer;
  uint8_t test[2];
  test[0] = 0xab;
  test[1] = 0xc0;

  testBitDeserializer.PushBytes(test, 2);
  uint16_t nibble1 = testBitDeserializer.GetBits(7);
  uint8_t nibble2 = testBitDeserializer.GetBits(3);
  uint8_t nibble3 = testBitDeserializer.GetBits(2);

  std::cout << "Result:    " << std::hex << nibble1 << " " << +nibble2 << " "
            << +nibble3 << " " << std::endl;
  std::cout << "Expecting: 55 7 0" << std::endl;

  return 0;
}
