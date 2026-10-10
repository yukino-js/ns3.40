
#include "gnuplot-aggregator.h"

#include "ns3/abort.h"
#include "ns3/log.h"

#include <fstream>
#include <iostream>
#include <string>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GnuplotAggregator");

NS_OBJECT_ENSURE_REGISTERED(GnuplotAggregator);

TypeId GnuplotAggregator::GetTypeId() {
  static TypeId tid = TypeId("ns3::GnuplotAggregator")
                          .SetParent<DataCollectionObject>()
                          .SetGroupName("Stats");

  return tid;
}

GnuplotAggregator::GnuplotAggregator(
    const std::string &outputFileNameWithoutExtension)
    : m_outputFileNameWithoutExtension(outputFileNameWithoutExtension),
      m_graphicsFileName(m_outputFileNameWithoutExtension + ".png"),
      m_title("Data Values"), m_xLegend("X Values"), m_yLegend("Y Values"),
      m_titleSet(false), m_xAndYLegendsSet(false),
      m_gnuplot(m_graphicsFileName) {
  NS_LOG_FUNCTION(this);
}

GnuplotAggregator::~GnuplotAggregator() {
  NS_LOG_FUNCTION(this);
  if (!m_titleSet) {
    NS_LOG_WARN(
        "Warning: The plot title was not set for the gnuplot aggregator");
  }
  if (!m_xAndYLegendsSet) {
    NS_LOG_WARN(
        "Warning: The axis legends were not set for the gnuplot aggregator");
  }

  std::string dataFileName = m_outputFileNameWithoutExtension + ".dat";
  std::string plotFileName = m_outputFileNameWithoutExtension + ".plt";
  std::string scriptFileName = m_outputFileNameWithoutExtension + ".sh";

  std::ofstream plotFile;
  plotFile.open(plotFileName);
  std::ofstream dataFile;
  dataFile.open(dataFileName);

  m_gnuplot.AppendExtra("set datafile missing \"-nan\"");

  m_gnuplot.GenerateOutput(plotFile, dataFile, dataFileName);

  plotFile.close();
  dataFile.close();

  std::ofstream scriptFile;
  scriptFile.open(scriptFileName);

  scriptFile << "#!/bin/sh" << std::endl;
  scriptFile << std::endl;
  scriptFile << "gnuplot " << plotFileName << std::endl;

  scriptFile.close();
}

void GnuplotAggregator::Write2d(std::string context, double x, double y) {
  NS_LOG_FUNCTION(this << context << x << y);

  if (m_2dDatasetMap.count(context) == 0) {
    NS_ABORT_MSG("Dataset " << context << " has not been added");
  }

  if (m_enabled) {
    m_2dDatasetMap[context].Add(x, y);
  }
}

void GnuplotAggregator::Write2dWithXErrorDelta(std::string context, double x,
                                               double y, double errorDelta) {
  NS_LOG_FUNCTION(this << context << x << y << errorDelta);

  if (m_2dDatasetMap.count(context) == 0) {
    NS_ABORT_MSG("Dataset " << context << " has not been added");
  }

  if (m_enabled) {
    m_2dDatasetMap[context].Add(x, y, errorDelta);
  }
}

void GnuplotAggregator::Write2dWithYErrorDelta(std::string context, double x,
                                               double y, double errorDelta) {
  NS_LOG_FUNCTION(this << context << x << y << errorDelta);

  if (m_2dDatasetMap.count(context) == 0) {
    NS_ABORT_MSG("Dataset " << context << " has not been added");
  }

  if (m_enabled) {
    m_2dDatasetMap[context].Add(x, y, errorDelta);
  }
}

void GnuplotAggregator::Write2dWithXYErrorDelta(std::string context, double x,
                                                double y, double xErrorDelta,
                                                double yErrorDelta) {
  NS_LOG_FUNCTION(this << context << x << y << xErrorDelta << yErrorDelta);

  if (m_2dDatasetMap.count(context) == 0) {
    NS_ABORT_MSG("Dataset " << context << " has not been added");
  }

  if (m_enabled) {
    m_2dDatasetMap[context].Add(x, y, xErrorDelta, yErrorDelta);
  }
}

void GnuplotAggregator::SetTerminal(const std::string &terminal) {
  m_graphicsFileName = m_outputFileNameWithoutExtension + "." + terminal;

  m_gnuplot.SetTerminal(terminal);
  m_gnuplot.SetOutputFilename(m_graphicsFileName);
}

void GnuplotAggregator::SetTitle(const std::string &title) {
  NS_LOG_FUNCTION(this << title);
  m_gnuplot.SetTitle(title);
  m_titleSet = true;
}

void GnuplotAggregator::SetLegend(const std::string &xLegend,
                                  const std::string &yLegend) {
  NS_LOG_FUNCTION(this << xLegend << yLegend);
  m_gnuplot.SetLegend(xLegend, yLegend);
  m_xAndYLegendsSet = true;
}

void GnuplotAggregator::SetExtra(const std::string &extra) {
  NS_LOG_FUNCTION(this << extra);
  m_gnuplot.SetExtra(extra);
}

void GnuplotAggregator::AppendExtra(const std::string &extra) {
  NS_LOG_FUNCTION(this << extra);
  m_gnuplot.AppendExtra(extra);
}

void GnuplotAggregator::Add2dDataset(const std::string &dataset,
                                     const std::string &title) {
  NS_LOG_FUNCTION(this << dataset << title);

  if (m_2dDatasetMap.count(dataset) > 0) {
    NS_ABORT_MSG("Dataset " << dataset << " has already been added");
  }

  Gnuplot2dDataset gnuplot2dDataset(title);
  m_2dDatasetMap[dataset] = gnuplot2dDataset;

  m_gnuplot.AddDataset(m_2dDatasetMap[dataset]);
}

void GnuplotAggregator::Set2dDatasetDefaultExtra(const std::string &extra) {
  NS_LOG_FUNCTION(extra);
  Gnuplot2dDataset::SetDefaultExtra(extra);
}

void GnuplotAggregator::Set2dDatasetExtra(const std::string &dataset,
                                          const std::string &extra) {
  NS_LOG_FUNCTION(this << dataset << extra);
  if (m_2dDatasetMap.count(dataset) == 0) {
    NS_ABORT_MSG("Dataset " << dataset << " has not been added");
  }

  m_2dDatasetMap[dataset].SetExtra(extra);
}

void GnuplotAggregator::Write2dDatasetEmptyLine(const std::string &dataset) {
  NS_LOG_FUNCTION(this << dataset);
  if (m_2dDatasetMap.count(dataset) == 0) {
    NS_ABORT_MSG("Dataset " << dataset << " has not been added");
  }

  if (m_enabled) {
    m_2dDatasetMap[dataset].AddEmptyLine();
  }
}

void GnuplotAggregator::Set2dDatasetDefaultStyle(
    Gnuplot2dDataset::Style style) {
  NS_LOG_FUNCTION(style);
  Gnuplot2dDataset::SetDefaultStyle(style);
}

void GnuplotAggregator::Set2dDatasetStyle(const std::string &dataset,
                                          Gnuplot2dDataset::Style style) {
  NS_LOG_FUNCTION(this << dataset << style);
  if (m_2dDatasetMap.count(dataset) == 0) {
    NS_ABORT_MSG("Dataset " << dataset << " has not been added");
  }

  m_2dDatasetMap[dataset].SetStyle(style);
}

void GnuplotAggregator::Set2dDatasetDefaultErrorBars(
    Gnuplot2dDataset::ErrorBars errorBars) {
  NS_LOG_FUNCTION(errorBars);
  Gnuplot2dDataset::SetDefaultErrorBars(errorBars);
}

void GnuplotAggregator::Set2dDatasetErrorBars(
    const std::string &dataset, Gnuplot2dDataset::ErrorBars errorBars) {
  NS_LOG_FUNCTION(this << dataset << errorBars);
  if (m_2dDatasetMap.count(dataset) == 0) {
    NS_ABORT_MSG("Dataset " << dataset << " has not been added");
  }

  m_2dDatasetMap[dataset].SetErrorBars(errorBars);
}

void GnuplotAggregator::SetKeyLocation(
    GnuplotAggregator::KeyLocation keyLocation) {
  NS_LOG_FUNCTION(this << keyLocation);
  switch (keyLocation) {
  case NO_KEY:
    m_gnuplot.AppendExtra("set key off");
    break;
  case KEY_ABOVE:
    m_gnuplot.AppendExtra("set key outside center above");
    break;
  case KEY_BELOW:
    m_gnuplot.AppendExtra("set key outside center below");
    break;
  default:
    m_gnuplot.AppendExtra("set key inside");
    break;
  }
}

} // namespace ns3
