
#ifndef SPECTRUM_TEST_H
#define SPECTRUM_TEST_H

#include <ns3/spectrum-value.h>
#include <ns3/test.h>

#define NS_TEST_ASSERT_MSG_SPECTRUM_MODEL_EQ_TOL(actual, expected, tol, msg)   \
  do {                                                                         \
    auto i = (actual).Begin();                                                 \
    auto j = (expected).Begin();                                               \
    uint32_t k = 0;                                                            \
    while (i != (actual).End() && j != (expected).End()) {                     \
      if ((i->fl > j->fl + (tol)) || (i->fl < j->fl - (tol)) ||                \
          (i->fc > j->fc + (tol)) || (i->fc < j->fc - (tol)) ||                \
          (i->fh > j->fh + (tol)) || (i->fh < j->fh - (tol))) {                \
        ASSERT_ON_FAILURE;                                                     \
        std::ostringstream indexStream;                                        \
        indexStream << "[" << k << "]";                                        \
        std::ostringstream msgStream;                                          \
        msgStream << (msg);                                                    \
        std::ostringstream actualStream;                                       \
        actualStream << i->fl << " <-- " << i->fc << " --> " << i->fh;         \
        std::ostringstream expectedStream;                                     \
        expectedStream << j->fl << " <-- " << j->fc << " --> " << j->fh;       \
        ReportTestFailure(std::string(#actual) + indexStream.str() + " == " +  \
                              std::string(#expected) + indexStream.str(),      \
                          actualStream.str(), expectedStream.str(),            \
                          msgStream.str(), (__FILE__), (__LINE__));            \
        CONTINUE_ON_FAILURE;                                                   \
      }                                                                        \
      ++i;                                                                     \
      ++j;                                                                     \
      ++k;                                                                     \
    }                                                                          \
    if (i != (actual).End() || j != (expected).End()) {                        \
      std::ostringstream msgStream;                                            \
      msgStream << (msg);                                                      \
      std::ostringstream actualStream;                                         \
      actualStream << (i != (actual).End());                                   \
      std::ostringstream expectedStream;                                       \
      expectedStream << (j != (expected).End());                               \
      ReportTestFailure("Bands::iterator == End ()", actualStream.str(),       \
                        expectedStream.str(), msgStream.str(), (__FILE__),     \
                        (__LINE__));                                           \
    }                                                                          \
  } while (false);

#define NS_TEST_ASSERT_MSG_SPECTRUM_VALUE_EQ_TOL(actual, expected, tol, msg)   \
  do {                                                                         \
    auto i = (actual).ConstValuesBegin();                                      \
    auto j = (expected).ConstValuesBegin();                                    \
    uint32_t k = 0;                                                            \
    while (i != (actual).ConstValuesEnd() &&                                   \
           j != (expected).ConstValuesEnd()) {                                 \
      if ((*i) > (*j) + (tol) || (*i) < (*j) - (tol)) {                        \
        ASSERT_ON_FAILURE;                                                     \
        std::ostringstream indexStream;                                        \
        indexStream << "[" << k << "]";                                        \
        std::ostringstream msgStream;                                          \
        msgStream << msg;                                                      \
        std::ostringstream actualStream;                                       \
        actualStream << actual;                                                \
        std::ostringstream expectedStream;                                     \
        expectedStream << expected;                                            \
        ReportTestFailure(std::string(#actual) + indexStream.str() + " == " +  \
                              std::string(#expected) + indexStream.str(),      \
                          actualStream.str(), expectedStream.str(),            \
                          msgStream.str(), __FILE__, __LINE__);                \
        CONTINUE_ON_FAILURE;                                                   \
      }                                                                        \
      ++i;                                                                     \
      ++j;                                                                     \
      ++k;                                                                     \
    }                                                                          \
    if (i != (actual).ConstValuesEnd() || j != (expected).ConstValuesEnd()) {  \
      std::ostringstream msgStream;                                            \
      msgStream << (msg);                                                      \
      std::ostringstream actualStream;                                         \
      actualStream << (i != (actual).ConstValuesEnd());                        \
      std::ostringstream expectedStream;                                       \
      expectedStream << (j != (expected).ConstValuesEnd());                    \
      ReportTestFailure("Values::const_iterator == ConstValuesEnd ()",         \
                        actualStream.str(), expectedStream.str(),              \
                        msgStream.str(), (__FILE__), (__LINE__));              \
    }                                                                          \
  } while (false);

#endif
