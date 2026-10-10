
#ifndef PCAP_TEST_H
#define PCAP_TEST_H

#include "pcap-file.h"

#include "ns3/test.h"

#include <sstream>
#include <stdint.h>
#include <string>

#define NS_PCAP_TEST_EXPECT_EQ(filename)                                       \
  do {                                                                         \
    std::ostringstream oss;                                                    \
    oss << filename;                                                           \
    std::string expected = CreateDataDirFilename(oss.str());                   \
    std::string got = CreateTempDirFilename(oss.str());                        \
    uint32_t sec{0};                                                           \
    uint32_t usec{0};                                                          \
    uint32_t packets{0};                                                       \
                                                                               \
    bool diff = PcapFile::Diff(got, expected, sec, usec, packets);             \
    NS_TEST_EXPECT_MSG_EQ(diff, false,                                         \
                          "PCAP traces " << got << " and " << expected         \
                                         << " differ starting from packet "    \
                                         << packets << " at " << sec << " s "  \
                                         << usec << " us");                    \
  } while (false)

#endif
