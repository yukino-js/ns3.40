
#ifndef LTE_GLOBAL_PATHLOSS_DATABASE_H
#define LTE_GLOBAL_PATHLOSS_DATABASE_H

#include <ns3/log.h>
#include <ns3/ptr.h>

#include <map>
#include <string>

namespace ns3 {

class SpectrumPhy;

class LteGlobalPathlossDatabase {
public:
  virtual ~LteGlobalPathlossDatabase();

  virtual void UpdatePathloss(std::string context, Ptr<const SpectrumPhy> txPhy,
                              Ptr<const SpectrumPhy> rxPhy, double lossDb) = 0;

  double GetPathloss(uint16_t cellId, uint64_t imsi);

  void Print();

protected:
  std::map<uint16_t, std::map<uint64_t, double>> m_pathlossMap;
};

class DownlinkLteGlobalPathlossDatabase : public LteGlobalPathlossDatabase {
public:
  void UpdatePathloss(std::string context, Ptr<const SpectrumPhy> txPhy,
                      Ptr<const SpectrumPhy> rxPhy, double lossDb) override;
};

class UplinkLteGlobalPathlossDatabase : public LteGlobalPathlossDatabase {
public:
  void UpdatePathloss(std::string context, Ptr<const SpectrumPhy> txPhy,
                      Ptr<const SpectrumPhy> rxPhy, double lossDb) override;
};

} // namespace ns3

#endif
