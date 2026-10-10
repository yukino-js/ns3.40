
#ifndef MULTI_MODEL_SPECTRUM_CHANNEL_H
#define MULTI_MODEL_SPECTRUM_CHANNEL_H

#include "spectrum-channel.h"
#include "spectrum-converter.h"
#include "spectrum-propagation-loss-model.h"
#include "spectrum-value.h"

#include <ns3/propagation-delay-model.h>

#include <map>
#include <set>

namespace ns3 {

typedef std::map<SpectrumModelUid_t, SpectrumConverter> SpectrumConverterMap_t;

class TxSpectrumModelInfo {
public:
  TxSpectrumModelInfo(Ptr<const SpectrumModel> txSpectrumModel);

  Ptr<const SpectrumModel> m_txSpectrumModel;
  SpectrumConverterMap_t m_spectrumConverterMap;
};

typedef std::map<SpectrumModelUid_t, TxSpectrumModelInfo>
    TxSpectrumModelInfoMap_t;

class RxSpectrumModelInfo {
public:
  RxSpectrumModelInfo(Ptr<const SpectrumModel> rxSpectrumModel);

  Ptr<const SpectrumModel> m_rxSpectrumModel;
  std::vector<Ptr<SpectrumPhy>> m_rxPhys;
};

typedef std::map<SpectrumModelUid_t, RxSpectrumModelInfo>
    RxSpectrumModelInfoMap_t;

class MultiModelSpectrumChannel : public SpectrumChannel {
public:
  MultiModelSpectrumChannel();

  static TypeId GetTypeId();

  void RemoveRx(Ptr<SpectrumPhy> phy) override;
  void AddRx(Ptr<SpectrumPhy> phy) override;
  void StartTx(Ptr<SpectrumSignalParameters> params) override;

  std::size_t GetNDevices() const override;
  Ptr<NetDevice> GetDevice(std::size_t i) const override;

protected:
  void DoDispose() override;

private:
  TxSpectrumModelInfoMap_t::const_iterator
  FindAndEventuallyAddTxSpectrumModel(Ptr<const SpectrumModel> txSpectrumModel);

  virtual void StartRx(Ptr<SpectrumSignalParameters> params,
                       Ptr<SpectrumPhy> receiver);

  TxSpectrumModelInfoMap_t m_txSpectrumModelInfoMap;

  RxSpectrumModelInfoMap_t m_rxSpectrumModelInfoMap;

  std::size_t m_numDevices;
};

} // namespace ns3

#endif
