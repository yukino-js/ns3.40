
#include "ns3/core-module.h"
#include "ns3/stats-module.h"

using namespace ns3;

namespace {

void Create2dPlot() {
  std::string fileNameWithoutExtension = "gnuplot-aggregator";
  std::string plotTitle = "Gnuplot Aggregator Plot";
  std::string plotXAxisHeading = "Time (seconds)";
  std::string plotYAxisHeading = "Double Values";
  std::string plotDatasetLabel = "Data Values";
  std::string datasetContext = "Dataset/Context/String";

  Ptr<GnuplotAggregator> aggregator =
      CreateObject<GnuplotAggregator>(fileNameWithoutExtension);

  aggregator->SetTerminal("png");
  aggregator->SetTitle(plotTitle);
  aggregator->SetLegend(plotXAxisHeading, plotYAxisHeading);

  aggregator->Add2dDataset(datasetContext, plotDatasetLabel);

  aggregator->Enable();

  double time;
  double value;

  for (time = -5.0; time <= +5.0; time += 1.0) {
    value = time * time;

    aggregator->Write2d(datasetContext, time, value);
  }

  aggregator->Disable();
}

} // namespace

int main(int argc, char *argv[]) {
  Create2dPlot();

  return 0;
}
