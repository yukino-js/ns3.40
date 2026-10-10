
#ifndef SPECTRUM_INTERFERENCE_H
#define SPECTRUM_INTERFERENCE_H

#include "spectrum-value.h"

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>

namespace ns3 {

class SpectrumErrorModel;

class SpectrumInterference : public Object {
public:
  SpectrumInterference();
  ~SpectrumInterference() override;

  static TypeId GetTypeId();

  void SetErrorModel(Ptr<SpectrumErrorModel> e);

  void StartRx(Ptr<const Packet> p, Ptr<const SpectrumValue> rxPsd);

  void AbortRx();

  bool EndRx();

  void AddSignal(Ptr<const SpectrumValue> spd, const Time duration);

  void SetNoisePowerSpectralDensity(Ptr<const SpectrumValue> noisePsd);

protected:
  void DoDispose() override;

private:
  void ConditionallyEvaluateChunk();
  void DoAddSignal(Ptr<const SpectrumValue> spd);
  void DoSubtractSignal(Ptr<const SpectrumValue> spd);

  bool m_receiving;

  Ptr<const SpectrumValue> m_rxSignal;

  Ptr<SpectrumValue> m_allSignals;

  Ptr<const SpectrumValue> m_noise;

  Time m_lastChangeTime;

  Ptr<SpectrumErrorModel> m_errorModel;
};

} // namespace ns3

#endif
