
#ifndef OMNET_DATA_OUTPUT_H
#define OMNET_DATA_OUTPUT_H

#include "data-output-interface.h"

#include "ns3/nstime.h"

namespace ns3 {

class OmnetDataOutput : public DataOutputInterface {
public:
  OmnetDataOutput();
  ~OmnetDataOutput() override;

  static TypeId GetTypeId();

  void Output(DataCollector &dc) override;

protected:
  void DoDispose() override;

private:
  class OmnetOutputCallback : public DataOutputCallback {
  public:
    OmnetOutputCallback(std::ostream *scalar);

    void OutputStatistic(std::string context, std::string name,
                         const StatisticalSummary *statSum) override;

    void OutputSingleton(std::string context, std::string name,
                         int val) override;

    void OutputSingleton(std::string context, std::string name,
                         uint32_t val) override;

    void OutputSingleton(std::string context, std::string name,
                         double val) override;

    void OutputSingleton(std::string context, std::string name,
                         std::string val) override;

    void OutputSingleton(std::string context, std::string name,
                         Time val) override;

  private:
    std::ostream *m_scalar;
  };
};

}; // namespace ns3

#endif
