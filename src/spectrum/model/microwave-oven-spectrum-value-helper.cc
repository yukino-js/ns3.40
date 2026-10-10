
#include "microwave-oven-spectrum-value-helper.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MicrowaveOvenSpectrumValue");

static Ptr<SpectrumModel> g_MicrowaveOvenSpectrumModel5Mhz;
static Ptr<SpectrumModel> g_MicrowaveOvenSpectrumModel6Mhz;

static class MicrowaveOvenSpectrumModel5MhzInitializer {
public:
  MicrowaveOvenSpectrumModel5MhzInitializer() {
    NS_LOG_FUNCTION(this);
    Bands bands;
    for (double fl = 2400e6; fl < 2499e6; fl += 5e6) {
      BandInfo bi;
      bi.fl = fl;
      bi.fc = fl + 5e6;
      bi.fh = fl + 10e6;
      bands.push_back(bi);
    }
    NS_LOG_LOGIC("bands.size () :" << bands.size());
    g_MicrowaveOvenSpectrumModel5Mhz = Create<SpectrumModel>(bands);
  }
} g_MicrowaveOvenSpectrumModel5MhzInitializerInstance;

static class MicrowaveOvenSpectrumModel6MhzInitializer {
public:
  MicrowaveOvenSpectrumModel6MhzInitializer() {
    NS_LOG_FUNCTION(this);
    Bands bands;
    for (double fl = 2360e6; fl < 2479e6; fl += 6e6) {
      BandInfo bi;
      bi.fl = fl;
      bi.fc = fl + 6e6;
      bi.fh = fl + 12e6;
      bands.push_back(bi);
    }
    NS_LOG_LOGIC("bands.size () :" << bands.size());
    g_MicrowaveOvenSpectrumModel6Mhz = Create<SpectrumModel>(bands);
  }
} g_MicrowaveOvenSpectrumModel6MhzInitializerInstance;

Ptr<SpectrumValue>
MicrowaveOvenSpectrumValueHelper::CreatePowerSpectralDensityMwo1() {
  Ptr<SpectrumValue> psd =
      Create<SpectrumValue>(g_MicrowaveOvenSpectrumModel6Mhz);

  (*psd)[0] = -67.5;
  (*psd)[1] = -67.5;
  (*psd)[2] = -67.5;
  (*psd)[3] = -67.5;
  (*psd)[4] = -67.5;
  (*psd)[5] = -66;
  (*psd)[6] = -64;
  (*psd)[7] = -63;
  (*psd)[8] = -62.5;
  (*psd)[9] = -63;
  (*psd)[10] = -62.5;
  (*psd)[11] = -62.5;
  (*psd)[12] = -58;
  (*psd)[13] = -53.5;
  (*psd)[14] = -44;
  (*psd)[15] = -38;
  (*psd)[16] = -45;
  (*psd)[17] = -65;
  (*psd)[18] = -67.5;
  (*psd)[19] = -67.5;

  (*psd) = Pow(10.0, ((*psd) - 30) / 10.0);

  return psd;
}

Ptr<SpectrumValue>
MicrowaveOvenSpectrumValueHelper::CreatePowerSpectralDensityMwo2() {

  Ptr<SpectrumValue> psd =
      Create<SpectrumValue>(g_MicrowaveOvenSpectrumModel5Mhz);

  (*psd)[0] = -68;
  (*psd)[1] = -68;
  (*psd)[2] = -68;
  (*psd)[3] = -68;
  (*psd)[4] = -65;
  (*psd)[5] = -62;
  (*psd)[6] = -56;
  (*psd)[7] = -55;
  (*psd)[8] = -47;
  (*psd)[9] = -40;
  (*psd)[10] = -37;
  (*psd)[11] = -33;
  (*psd)[12] = -45;
  (*psd)[13] = -67;
  (*psd)[14] = -68;
  (*psd)[15] = -68;
  (*psd)[16] = -68;
  (*psd)[17] = -68;
  (*psd)[18] = -68;
  (*psd)[19] = -68;

  (*psd) = Pow(10.0, ((*psd) - 30) / 10.0);

  return psd;
}

} // namespace ns3
