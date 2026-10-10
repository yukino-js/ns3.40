
#ifndef MATRIX_BASED_CHANNEL_H
#define MATRIX_BASED_CHANNEL_H

#include <ns3/matrix-array.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/phased-array-model.h>
#include <ns3/vector.h>

#include <tuple>

namespace ns3 {

class MobilityModel;

class MatrixBasedChannelModel : public Object {
public:
  ~MatrixBasedChannelModel() override;

  using DoubleVector = std::vector<double>;

  using Double2DVector = std::vector<DoubleVector>;

  using Double3DVector = std::vector<Double2DVector>;

  using Complex2DVector = ComplexMatrixArray;
  using Complex3DVector = ComplexMatrixArray;

  struct ChannelMatrix : public SimpleRefCount<ChannelMatrix> {
    Complex3DVector m_channel;

    Time m_generatedTime;

    std::pair<uint32_t, uint32_t> m_antennaPair;

    std::pair<uint32_t, uint32_t> m_nodeIds;

    virtual ~ChannelMatrix() = default;

    bool IsReverse(uint32_t aAntennaId, uint32_t bAntennaId) const {
      uint32_t sAntennaId;
      uint32_t uAntennaId;
      std::tie(sAntennaId, uAntennaId) = m_antennaPair;
      NS_ASSERT_MSG((sAntennaId == aAntennaId && uAntennaId == bAntennaId) ||
                        (sAntennaId == bAntennaId && uAntennaId == aAntennaId),
                    "This channel matrix does not represent the channel among "
                    "the antenna "
                    "arrays for which are provided IDs.");
      return (sAntennaId == bAntennaId && uAntennaId == aAntennaId);
    }
  };

  struct ChannelParams : public SimpleRefCount<ChannelParams> {
    Time m_generatedTime;

    DoubleVector m_delay;

    Double2DVector m_angle;

    DoubleVector m_alpha;

    DoubleVector m_D;

    std::pair<uint32_t, uint32_t> m_nodeIds;

    virtual ~ChannelParams() = default;
  };

  virtual Ptr<const ChannelMatrix>
  GetChannel(Ptr<const MobilityModel> aMob, Ptr<const MobilityModel> bMob,
             Ptr<const PhasedArrayModel> aAntenna,
             Ptr<const PhasedArrayModel> bAntenna) = 0;

  virtual Ptr<const ChannelParams>
  GetParams(Ptr<const MobilityModel> aMob,
            Ptr<const MobilityModel> bMob) const = 0;

  static uint64_t GetKey(uint32_t a, uint32_t b) {
    return (uint64_t)std::min(a, b) << 32 | std::max(a, b);
  }

  static const uint8_t AOA_INDEX = 0;
  static const uint8_t ZOA_INDEX = 1;
  static const uint8_t AOD_INDEX = 2;
  static const uint8_t ZOD_INDEX = 3;
};

}; // namespace ns3

#endif
