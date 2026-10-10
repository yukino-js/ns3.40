#ifndef DOPPLER_PROCESS_H
#define DOPPLER_PROCESS_H

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

#include <complex>

namespace ns3 {
class PropagationLossModel;
class JakesPropagationLossModel;

class JakesProcess : public Object {
public:
  static TypeId GetTypeId();
  JakesProcess();
  ~JakesProcess() override;

  std::complex<double> GetComplexGain() const;
  double GetChannelGainDb() const;

  void SetPropagationLossModel(Ptr<const PropagationLossModel> model);

protected:
  void DoDispose() override;

private:
  struct Oscillator {
    Oscillator(std::complex<double> amplitude, double initialPhase,
               double omega);
    std::complex<double> GetValueAt(Time t) const;

    std::complex<double> m_amplitude;
    double m_phase;
    double m_omega;
  };

private:
  void SetNOscillators(unsigned int nOscillators);

  void SetDopplerFrequencyHz(double dopplerFrequencyHz);

  void ConstructOscillators();

private:
  std::vector<Oscillator> m_oscillators;
  double m_omegaDopplerMax;
  unsigned int m_nOscillators;
  Ptr<UniformRandomVariable> m_uniformVariable;
  Ptr<const JakesPropagationLossModel> m_jakes;
};
} // namespace ns3
#endif
