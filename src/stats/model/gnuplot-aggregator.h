
#ifndef GNUPLOT_AGGREGATOR_H
#define GNUPLOT_AGGREGATOR_H

#include "data-collection-object.h"
#include "gnuplot.h"

#include <map>
#include <string>

namespace ns3 {

class GnuplotAggregator : public DataCollectionObject {
public:
  enum KeyLocation { NO_KEY, KEY_INSIDE, KEY_ABOVE, KEY_BELOW };

  static TypeId GetTypeId();

  GnuplotAggregator(const std::string &outputFileNameWithoutExtension);

  ~GnuplotAggregator() override;

  void Write2d(std::string context, double x, double y);

  void Write2dWithXErrorDelta(std::string context, double x, double y,
                              double errorDelta);

  void Write2dWithYErrorDelta(std::string context, double x, double y,
                              double errorDelta);

  void Write2dWithXYErrorDelta(std::string context, double x, double y,
                               double xErrorDelta, double yErrorDelta);

  void SetTerminal(const std::string &terminal);

  void SetTitle(const std::string &title);

  void SetLegend(const std::string &xLegend, const std::string &yLegend);

  void SetExtra(const std::string &extra);

  void AppendExtra(const std::string &extra);

  void Add2dDataset(const std::string &dataset, const std::string &title);

  static void Set2dDatasetDefaultExtra(const std::string &extra);

  void Set2dDatasetExtra(const std::string &dataset, const std::string &extra);

  void Write2dDatasetEmptyLine(const std::string &dataset);

  static void Set2dDatasetDefaultStyle(Gnuplot2dDataset::Style style);

  void Set2dDatasetStyle(const std::string &dataset,
                         Gnuplot2dDataset::Style style);

  static void
  Set2dDatasetDefaultErrorBars(Gnuplot2dDataset::ErrorBars errorBars);

  void Set2dDatasetErrorBars(const std::string &dataset,
                             Gnuplot2dDataset::ErrorBars errorBars);

  void SetKeyLocation(KeyLocation keyLocation);

private:
  std::string m_outputFileNameWithoutExtension;

  std::string m_graphicsFileName;

  std::string m_title;

  std::string m_terminal;

  std::string m_xLegend;

  std::string m_yLegend;

  std::string m_extra;

  bool m_titleSet;

  bool m_xAndYLegendsSet;

  Gnuplot m_gnuplot;

  std::map<std::string, Gnuplot2dDataset> m_2dDatasetMap;
};

} // namespace ns3

#endif
