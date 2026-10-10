
#include "ns3/command-line.h"
#include "ns3/histogram.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"

#include <iomanip>
#include <iostream>
#include <map>

using namespace ns3;

void RunSingleSample(std::string mode, Ptr<EmpiricalRandomVariable> erv) {
  std::cout << "------------------------------" << std::endl;
  std::cout << "Sampling " << mode << std::endl;

  std::cout << std::endl;
  std::cout << "Binned sample" << std::endl;
  double value = erv->GetValue();
  std::cout << "Binned sample: " << value << std::endl;
  std::cout << std::endl;

  std::cout << "Interpolated sample" << std::endl;
  erv->SetInterpolate(true);
  value = erv->GetValue();
  std::cout << "Interpolated sample:" << value << std::endl;
  erv->SetInterpolate(false);
}

void PrintStatsLine(const double value, const long count, const long n,
                    const long sum) {
  std::cout << std::fixed << std::setprecision(3) << std::setw(10) << std::right
            << value << std::setw(10) << std::right << count << std::setw(10)
            << std::right << count / static_cast<double>(n) * 100.0
            << std::setw(10) << std::right
            << sum / static_cast<double>(n) * 100.0 << std::endl;
}

void PrintSummary(long sum, long n, double weighted, double expected) {
  std::cout << std::endl;
  std::cout << "                      --------" << std::endl;
  std::cout << "              Total " << std::setprecision(3) << std::fixed
            << std::setw(10) << std::right
            << sum / static_cast<double>(n) * 100.0 << std::endl;
  std::cout << "            Average " << std::setprecision(3) << std::fixed
            << std::setw(6) << std::right << weighted / n << std::endl;
  std::cout << "           Expected " << std::setprecision(3) << std::fixed
            << std::setw(6) << std::right << expected << std::endl
            << std::endl;
}

void RunBothModes(std::string mode, Ptr<EmpiricalRandomVariable> erv, long n) {
  std::cout << std::endl;
  std::cout << "Sampling " << mode << std::endl;
  std::map<double, int> counts;
  counts[0] = 0;
  for (long i = 0; i < n; ++i) {
    ++counts[erv->GetValue()];
  }
  long sum = 0;
  double weighted = 0;
  std::cout << std::endl;
  std::cout << "     Value    Counts         %     % CDF" << std::endl;
  std::cout << "----------  --------  --------  --------" << std::endl;
  for (auto c : counts) {
    long count = c.second;
    double value = c.first;
    sum += count;
    weighted += value * count;
    PrintStatsLine(value, count, n, sum);
  }
  PrintSummary(sum, n, weighted, 0.8);

  std::cout << "Interpolating " << mode << std::endl;
  erv->SetInterpolate(true);
  Histogram h(0.1);
  for (long i = 0; i < n; ++i) {
    h.AddValue(erv->GetValue());
  }
  erv->SetInterpolate(false);
  sum = 0;
  weighted = 0;
  std::cout << std::endl;
  std::cout << " Bin Start    Counts         %     % CDF" << std::endl;
  std::cout << "----------  --------  --------  --------" << std::endl;
  for (uint32_t i = 0; i < h.GetNBins(); ++i) {
    long count = h.GetBinCount(i);
    double start = h.GetBinStart(i);
    double value = start + h.GetBinWidth(i) / 2.;
    sum += count;
    weighted += count * value;
    PrintStatsLine(start, count, n, sum);
  }
  PrintSummary(sum, n, weighted, 0.760);
}

int main(int argc, char *argv[]) {
  long n = 1000000;
  bool disableAnti = false;
  bool single = false;
  CommandLine cmd;
  cmd.AddValue("count", "how many draws to make from the rng", n);
  cmd.AddValue("antithetic", "disable antithetic sampling", disableAnti);
  cmd.AddValue("single", "sample a single time", single);
  cmd.Parse(argc, argv);
  std::cout << std::endl;
  std::cout << cmd.GetName() << std::endl;
  if (!single) {
    std::cout << "Sample count: " << n << std::endl;
  } else {
    std::cout << "Sampling a single time" << std::endl;
  }
  if (disableAnti) {
    std::cout << "Antithetic sampling disabled" << std::endl;
  }

  Ptr<EmpiricalRandomVariable> erv = CreateObject<EmpiricalRandomVariable>();

  erv->CDF(0.0, 0.0 / 15.0);
  erv->CDF(0.2, 1.0 / 15.0);
  erv->CDF(0.4, 3.0 / 15.0);
  erv->CDF(0.6, 4.0 / 15.0);
  erv->CDF(0.8, 7.0 / 15.0);
  erv->CDF(1.0, 9.0 / 15.0);
  erv->CDF(1.0, 15.0 / 15.0);

  if (single) {
    RunSingleSample("normal", erv);
    if (!disableAnti) {
      std::cout << std::endl;
      std::cout << "Antithetic" << std::endl;
      erv->SetAntithetic(true);
      RunSingleSample("antithetic", erv);
      erv->SetAntithetic(false);
    }

    std::cout << std::endl;
    return 0;
  }

  RunBothModes("normal", erv, n);

  if (!disableAnti) {
    erv->SetAntithetic(true);
    RunBothModes("antithetic", erv, n);
    erv->SetAntithetic(false);
  }

  return 0;
}
