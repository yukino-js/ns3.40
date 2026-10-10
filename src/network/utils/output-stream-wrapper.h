
#ifndef OUTPUT_STREAM_WRAPPER_H
#define OUTPUT_STREAM_WRAPPER_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

#include <fstream>

namespace ns3 {

class OutputStreamWrapper : public SimpleRefCount<OutputStreamWrapper> {
public:
  OutputStreamWrapper(std::string filename, std::ios::openmode filemode);
  OutputStreamWrapper(std::ostream *os);
  ~OutputStreamWrapper();

  std::ostream *GetStream();

private:
  std::ostream *m_ostream;
  bool m_destroyable;
};

} // namespace ns3

#endif
