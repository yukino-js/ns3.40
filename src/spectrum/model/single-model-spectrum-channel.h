
#ifndef SINGLE_MODEL_SPECTRUM_CHANNEL_H
#define SINGLE_MODEL_SPECTRUM_CHANNEL_H

#include "spectrum-channel.h"
#include "spectrum-model.h"

#include <ns3/traced-callback.h>

namespace ns3 {

class SingleModelSpectrumChannel : public SpectrumChannel {
public:
  SingleModelSpectrumChannel();

  static TypeId GetTypeId();

  void RemoveRx(Ptr<SpectrumPhy> phy) override;
  void AddRx(Ptr<SpectrumPhy> phy) override;
  void StartTx(Ptr<SpectrumSignalParameters> params) override;

  std::size_t GetNDevices() const override;
  Ptr<NetDevice> GetDevice(std::size_t i) const override;

  typedef std::vector<Ptr<SpectrumPhy>> PhyList;

private:
  void DoDispose() override;

  void StartRx(Ptr<SpectrumSignalParameters> params, Ptr<SpectrumPhy> receiver);

  PhyList m_phyList;

  Ptr<const SpectrumModel> m_spectrumModel;
};

} // namespace ns3

#endif
