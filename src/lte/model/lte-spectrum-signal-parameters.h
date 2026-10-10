
#ifndef LTE_SPECTRUM_SIGNAL_PARAMETERS_H
#define LTE_SPECTRUM_SIGNAL_PARAMETERS_H

#include <ns3/spectrum-signal-parameters.h>

#include <list>

namespace ns3 {

class PacketBurst;
class LteControlMessage;

struct LteSpectrumSignalParameters : public SpectrumSignalParameters {
  Ptr<SpectrumSignalParameters> Copy() const override;

  LteSpectrumSignalParameters();

  LteSpectrumSignalParameters(const LteSpectrumSignalParameters &p);

  Ptr<PacketBurst> packetBurst;
};

struct LteSpectrumSignalParametersDataFrame : public SpectrumSignalParameters {
  Ptr<SpectrumSignalParameters> Copy() const override;

  LteSpectrumSignalParametersDataFrame();

  LteSpectrumSignalParametersDataFrame(
      const LteSpectrumSignalParametersDataFrame &p);

  Ptr<PacketBurst> packetBurst;

  std::list<Ptr<LteControlMessage>> ctrlMsgList;

  uint16_t cellId;
};

struct LteSpectrumSignalParametersDlCtrlFrame
    : public SpectrumSignalParameters {
  Ptr<SpectrumSignalParameters> Copy() const override;

  LteSpectrumSignalParametersDlCtrlFrame();

  LteSpectrumSignalParametersDlCtrlFrame(
      const LteSpectrumSignalParametersDlCtrlFrame &p);

  std::list<Ptr<LteControlMessage>> ctrlMsgList;

  uint16_t cellId;
  bool pss;
};

struct LteSpectrumSignalParametersUlSrsFrame : public SpectrumSignalParameters {
  Ptr<SpectrumSignalParameters> Copy() const override;

  LteSpectrumSignalParametersUlSrsFrame();

  LteSpectrumSignalParametersUlSrsFrame(
      const LteSpectrumSignalParametersUlSrsFrame &p);

  uint16_t cellId;
};

} // namespace ns3

#endif
