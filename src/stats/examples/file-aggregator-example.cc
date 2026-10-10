
#include "ns3/core-module.h"
#include "ns3/stats-module.h"

using namespace ns3;

namespace {

void CreateCommaSeparatedFile() {
  std::string fileName = "file-aggregator-comma-separated.txt";
  std::string datasetContext = "Dataset/Context/String";

  Ptr<FileAggregator> aggregator =
      CreateObject<FileAggregator>(fileName, FileAggregator::COMMA_SEPARATED);

  aggregator->Enable();

  double time;
  double value;

  for (time = -5.0; time <= +5.0; time += 1.0) {
    value = time * time;

    aggregator->Write2d(datasetContext, time, value);
  }

  aggregator->Disable();
}

void CreateSpaceSeparatedFile() {
  std::string fileName = "file-aggregator-space-separated.txt";
  std::string datasetContext = "Dataset/Context/String";

  Ptr<FileAggregator> aggregator = CreateObject<FileAggregator>(fileName);

  aggregator->Enable();

  double time;
  double value;

  for (time = -5.0; time <= +5.0; time += 1.0) {
    value = time * time;

    aggregator->Write2d(datasetContext, time, value);
  }

  aggregator->Disable();
}

void CreateFormattedFile() {
  std::string fileName = "file-aggregator-formatted-values.txt";
  std::string datasetContext = "Dataset/Context/String";

  Ptr<FileAggregator> aggregator =
      CreateObject<FileAggregator>(fileName, FileAggregator::FORMATTED);

  aggregator->Set2dFormat("Time = %.3e\tValue = %.0f");

  aggregator->Enable();

  double time;
  double value;

  for (time = -5.0; time < 5.5; time += 1.0) {
    value = time * time;

    aggregator->Write2d(datasetContext, time, value);
  }

  aggregator->Disable();
}

} // namespace

int main(int argc, char *argv[]) {
  CreateCommaSeparatedFile();
  CreateSpaceSeparatedFile();
  CreateFormattedFile();

  return 0;
}
