
#ifndef SNR_TO_BLOCK_ERROR_RATE_RECORD_H
#define SNR_TO_BLOCK_ERROR_RATE_RECORD_H

namespace ns3 {

class SNRToBlockErrorRateRecord {
public:
  SNRToBlockErrorRateRecord(double snrValue, double bitErrorRate,
                            double BlockErrorRate, double sigma2, double I1,
                            double I2);
  ~SNRToBlockErrorRateRecord();

  double GetSNRValue() const;
  double GetBitErrorRate() const;
  double GetBlockErrorRate() const;
  double GetSigma2() const;
  double GetI1() const;
  double GetI2() const;
  SNRToBlockErrorRateRecord *Copy() const;

  void SetSNRValue(double snrValue);
  void SetBitErrorRate(double bitErrorRate);
  void SetBlockErrorRate(double blockErrorRate);
  void SetI1(double i1);
  void SetI2(double i2);

private:
  double m_snrValue;
  double m_bitErrorRate;
  double m_blockErrorRate;
  double m_sigma2;
  double m_i1;
  double m_i2;
};

} // namespace ns3

#endif
