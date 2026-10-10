
#ifndef SPECTRUM_ERROR_MODEL_H
#define SPECTRUM_ERROR_MODEL_H

#include "spectrum-value.h"

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>

namespace ns3 {

class SpectrumErrorModel : public Object {
public:
  static TypeId GetTypeId();
  ~SpectrumErrorModel() override;

  virtual void StartRx(Ptr<const Packet> p) = 0;

  virtual void EvaluateChunk(const SpectrumValue &sinr, Time duration) = 0;

  virtual bool IsRxCorrect() = 0;
};

class ShannonSpectrumErrorModel : public SpectrumErrorModel {
protected:
  void DoDispose() override;

public:
  static TypeId GetTypeId();
  void StartRx(Ptr<const Packet> p) override;
  void EvaluateChunk(const SpectrumValue &sinr, Time duration) override;
  bool IsRxCorrect() override;

private:
  uint32_t m_bytes;
  uint32_t m_deliverableBytes;
};

} // namespace ns3

#endif
