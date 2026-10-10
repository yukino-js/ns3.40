#ifndef LR_WPAN_INTERFERENCE_HELPER_H
#define LR_WPAN_INTERFERENCE_HELPER_H

#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>

#include <set>

namespace ns3 {

class SpectrumValue;
class SpectrumModel;

class LrWpanInterferenceHelper
    : public SimpleRefCount<LrWpanInterferenceHelper> {
public:
  LrWpanInterferenceHelper(Ptr<const SpectrumModel> spectrumModel);

  ~LrWpanInterferenceHelper();

  bool AddSignal(Ptr<const SpectrumValue> signal);

  bool RemoveSignal(Ptr<const SpectrumValue> signal);

  void ClearSignals();

  Ptr<SpectrumValue> GetSignalPsd() const;

  Ptr<const SpectrumModel> GetSpectrumModel() const;

private:
  LrWpanInterferenceHelper(const LrWpanInterferenceHelper &);
  LrWpanInterferenceHelper &operator=(const LrWpanInterferenceHelper &);
  Ptr<const SpectrumModel> m_spectrumModel;

  std::set<Ptr<const SpectrumValue>> m_signals;

  mutable Ptr<SpectrumValue> m_signal;

  mutable bool m_dirty;
};

} // namespace ns3

#endif
