
#ifndef ASCII_TEST_H
#define ASCII_TEST_H

#include "ascii-file.h"
#include "test.h"

#include <stdint.h>

#define NS_ASCII_TEST_EXPECT_EQ(gotFilename, expectedFilename)                 \
  do {                                                                         \
    uint64_t line(0);                                                          \
    bool diff = AsciiFile::Diff(gotFilename, expectedFilename, line);          \
    NS_TEST_EXPECT_MSG_EQ(diff, false,                                         \
                          "ASCII traces "                                      \
                              << gotFilename << " and " << expectedFilename    \
                              << " differ starting from line " << line);       \
  } while (false)

#endif
