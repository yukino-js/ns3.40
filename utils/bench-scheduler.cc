
#include "ns3/core-module.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <vector>

using namespace ns3;

bool g_debug = false;

std::string g_me;
#define LOG(x) std::cout << x << std::endl
#define LOGME(x) LOG(g_me << x)
#define DEB(x)                                                                 \
  if (g_debug) {                                                               \
    LOGME(x);                                                                  \
  }

int g_fwidth = 6;

class Bench {
public:
  Bench(const uint64_t population, const uint64_t total)
      : m_population(population), m_total(total), m_count(0) {}

  void SetRandomStream(Ptr<RandomVariableStream> stream) { m_rand = stream; }

  void SetPopulation(const uint64_t population) { m_population = population; }

  void SetTotal(const uint64_t total) { m_total = total; }

  struct Result {
    double init;
    double simu;
    uint64_t pop;
    uint64_t events;
  };

  Result Run();

private:
  void Cb();

  Ptr<RandomVariableStream> m_rand;
  uint64_t m_population;
  uint64_t m_total;
  uint64_t m_count;
};

Bench::Result Bench::Run() {
  SystemWallClockMs timer;
  double init;
  double simu;

  DEB("initializing");
  m_count = 0;

  timer.Start();
  for (uint64_t i = 0; i < m_population; ++i) {
    Time at = NanoSeconds(m_rand->GetValue());
    Simulator::Schedule(at, &Bench::Cb, this);
  }
  init = timer.End() / 1000.0;
  DEB("initialization took " << init << "s");

  DEB("running");
  timer.Start();
  Simulator::Run();
  simu = timer.End() / 1000.0;
  DEB("run took " << simu << "s");

  Simulator::Destroy();

  return Result{init, simu, m_population, m_count};
}

void Bench::Cb() {
  if (m_count >= m_total) {
    Simulator::Stop();
    return;
  }
  DEB("event at " << Simulator::Now().GetSeconds() << "s");

  Time after = NanoSeconds(m_rand->GetValue());
  Simulator::Schedule(after, &Bench::Cb, this);
  ++m_count;
}

class BenchSuite {
public:
  BenchSuite(ObjectFactory &factory, uint64_t pop, uint64_t total,
             uint64_t runs, Ptr<RandomVariableStream> eventStream, bool calRev);

  void Log() const;

private:
  void Header() const;

  struct PhaseResult {
    double time;
    double rate;
    double period;
  };

  struct Result {
    PhaseResult init;
    PhaseResult run;
    static Result Bench(Bench::Result r);

    template <typename T> void Log(T label) const;
  };

  std::string m_scheduler;
  std::vector<Result> m_results;
};

BenchSuite::Result BenchSuite::Result::Bench(Bench::Result r) {
  return Result{{r.init, r.pop / r.init, r.init / r.pop},
                {r.simu, r.events / r.simu, r.simu / r.events}};
}

template <typename T> void BenchSuite::Result::Log(T label) const {

  LOG(std::left << std::setw(g_fwidth) << label << std::setw(g_fwidth)
                << init.time << std::setw(g_fwidth) << init.rate
                << std::setw(g_fwidth) << init.period << std::setw(g_fwidth)
                << run.time << std::setw(g_fwidth) << run.rate
                << std::setw(g_fwidth) << run.period);
}

BenchSuite::BenchSuite(ObjectFactory &factory, uint64_t pop, uint64_t total,
                       uint64_t runs, Ptr<RandomVariableStream> eventStream,
                       bool calRev) {
  Simulator::SetScheduler(factory);

  m_scheduler = factory.GetTypeId().GetName();
  if (m_scheduler == "ns3::CalendarScheduler") {
    m_scheduler +=
        ": insertion order: " + std::string(calRev ? "reverse" : "normal");
  }
  if (m_scheduler == "ns3::MapScheduler") {
    m_scheduler += " (default)";
  }

  Bench bench(pop, total);
  bench.SetRandomStream(eventStream);
  bench.SetPopulation(pop);
  bench.SetTotal(total);

  m_results.reserve(runs);
  Header();

  DEB("priming");
  auto prime = bench.Run();
  Result::Bench(prime).Log("prime");

  for (uint64_t i = 0; i < runs; i++) {
    auto run = bench.Run();
    m_results.push_back(Result::Bench(run));
    m_results.back().Log(i);
  }

  Simulator::Destroy();
}

void BenchSuite::Header() const {
  LOG("");
  LOG(m_scheduler);
  LOG(std::left << std::setw(g_fwidth) << "Run #" << std::left
                << std::setw(3 * g_fwidth) << "Initialization:" << std::left
                << "Simulation:");
  LOG(std::left << std::setw(g_fwidth) << "" << std::left << std::setw(g_fwidth)
                << "Time (s)" << std::left << std::setw(g_fwidth)
                << "Rate (ev/s)" << std::left << std::setw(g_fwidth)
                << "Per (s/ev)" << std::left << std::setw(g_fwidth)
                << "Time (s)" << std::left << std::setw(g_fwidth)
                << "Rate (ev/s)" << std::left << "Per (s/ev)");
  LOG(std::setfill('-') << std::right << std::setw(g_fwidth) << " "
                        << std::right << std::setw(g_fwidth) << " "
                        << std::right << std::setw(g_fwidth) << " "
                        << std::right << std::setw(g_fwidth) << " "
                        << std::right << std::setw(g_fwidth) << " "
                        << std::right << std::setw(g_fwidth) << " "
                        << std::right << std::setw(g_fwidth) << " "
                        << std::setfill(' '));
}

void BenchSuite::Log() const {
  if (m_results.size() < 2) {
    LOG("");
    return;
  }

  uint64_t n{0};
  Result average{m_results[0]};
  Result moment2{{0, 0, 0}, {0, 0, 0}};

  for (; n < m_results.size(); ++n) {
    double deltaPre;
    double deltaPost;
    const auto &run = m_results[n];
    uint64_t count = n + 1;

#define ACCUMULATE(phase, field)                                               \
  deltaPre = run.phase.field - average.phase.field;                            \
  average.phase.field += deltaPre / count;                                     \
  deltaPost = run.phase.field - average.phase.field;                           \
  moment2.phase.field += deltaPre * deltaPost

    ACCUMULATE(init, time);
    ACCUMULATE(init, rate);
    ACCUMULATE(init, period);
    ACCUMULATE(run, time);
    ACCUMULATE(run, rate);
    ACCUMULATE(run, period);

#undef ACCUMULATE
  }

  auto stdev = Result{
      {std::sqrt(moment2.init.time / n), std::sqrt(moment2.init.rate / n),
       std::sqrt(moment2.init.period / n)},
      {std::sqrt(moment2.run.time / n), std::sqrt(moment2.run.rate / n),
       std::sqrt(moment2.run.period / n)},
  };

  average.Log("average");
  stdev.Log("stdev");

  LOG("");
}

Ptr<RandomVariableStream> GetRandomStream(std::string filename) {
  Ptr<RandomVariableStream> stream = nullptr;

  if (filename.empty()) {
    LOG("  Event time distribution:      default exponential");
    auto erv = CreateObject<ExponentialRandomVariable>();
    erv->SetAttribute("Mean", DoubleValue(100));
    stream = erv;
  } else {
    std::istream *input;

    if (filename == "-") {
      LOG("  Event time distribution:      from stdin");
      input = &std::cin;
    } else {
      LOG("  Event time distribution:      from " << filename);
      input = new std::ifstream(filename);
    }

    double value;
    std::vector<double> nsValues;

    while (!input->eof()) {
      if (*input >> value) {
        auto ns = (uint64_t)(value * 1000000000);
        nsValues.push_back(ns);
      } else {
        input->clear();
        std::string line;
        *input >> line;
      }
    }
    LOG("    Found " << nsValues.size() << " entries");
    auto drv = CreateObject<DeterministicRandomVariable>();
    drv->SetValueArray(&nsValues[0], nsValues.size());
    stream = drv;
  }

  return stream;
}

int main(int argc, char *argv[]) {
  bool allSched = false;
  bool schedCal = false;
  bool schedHeap = false;
  bool schedList = false;
  bool schedMap = false;
  bool schedPQ = false;

  uint64_t pop = 100000;
  uint64_t total = 1000000;
  uint64_t runs = 1;
  std::string filename = "";
  bool calRev = false;

  CommandLine cmd(__FILE__);
  cmd.Usage("Benchmark the simulator scheduler.\n"
            "\n"
            "Event intervals are taken from one of:\n"
            "  an exponential distribution, with mean 100 ns,\n"
            "  an ascii file, given by the --file=\"<filename>\" argument,\n"
            "  or standard input, by the argument --file=\"-\"\n"
            "In the case of either --file form, the input is expected\n"
            "to be ascii, giving the relative event times in ns.\n"
            "\n"
            "If no scheduler is specified the MapScheduler will be run.");
  cmd.AddValue("all", "use all schedulers", allSched);
  cmd.AddValue("cal", "use CalendarSheduler", schedCal);
  cmd.AddValue("calrev", "reverse ordering in the CalendarScheduler", calRev);
  cmd.AddValue("heap", "use HeapScheduler", schedHeap);
  cmd.AddValue("list", "use ListSheduler", schedList);
  cmd.AddValue("map", "use MapScheduler (default)", schedMap);
  cmd.AddValue("pri", "use PriorityQueue", schedPQ);
  cmd.AddValue("debug", "enable debugging output", g_debug);
  cmd.AddValue("pop", "event population size", pop);
  cmd.AddValue("total", "total number of events to run", total);
  cmd.AddValue("runs", "number of runs", runs);
  cmd.AddValue("file", "file of relative event times", filename);
  cmd.AddValue("prec", "printed output precision", g_fwidth);
  cmd.Parse(argc, argv);

  g_me = cmd.GetName() + ": ";
  g_fwidth += 6;

  LOG(std::setprecision(g_fwidth - 6));
  LOGME(" Benchmark the simulator scheduler");
  LOG("  Event population size:        " << pop);
  LOG("  Total events per run:         " << total);
  LOG("  Number of runs per scheduler: " << runs);
  DEB("debugging is ON");

  if (allSched) {
    schedCal = schedHeap = schedList = schedMap = schedPQ = true;
  }
  if (!(schedCal || schedHeap || schedList || schedMap || schedPQ)) {
    schedMap = true;
  }

  auto eventStream = GetRandomStream(filename);

  ObjectFactory factory("ns3::MapScheduler");
  if (schedCal) {
    factory.SetTypeId("ns3::CalendarScheduler");
    factory.Set("Reverse", BooleanValue(calRev));
    BenchSuite(factory, pop, total, runs, eventStream, calRev).Log();
    if (allSched) {
      factory.Set("Reverse", BooleanValue(!calRev));
      BenchSuite(factory, pop, total, runs, eventStream, !calRev).Log();
    }
  }
  if (schedHeap) {
    factory.SetTypeId("ns3::HeapScheduler");
    BenchSuite(factory, pop, total, runs, eventStream, calRev).Log();
  }
  if (schedList) {
    factory.SetTypeId("ns3::ListScheduler");
    auto listTotal = total;
    if (allSched) {
      LOG("Running List scheduler with 1/10 total events");
      listTotal /= 10;
    }
    BenchSuite(factory, pop, listTotal, runs, eventStream, calRev).Log();
  }
  if (schedMap) {
    factory.SetTypeId("ns3::MapScheduler");
    BenchSuite(factory, pop, total, runs, eventStream, calRev).Log();
  }
  if (schedPQ) {
    factory.SetTypeId("ns3::PriorityQueueScheduler");
    BenchSuite(factory, pop, total, runs, eventStream, calRev).Log();
  }

  return 0;
}
