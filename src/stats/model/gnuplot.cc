#include "gnuplot.h"

#include "ns3/assert.h"

#include <ostream>
#include <stdexcept>

namespace ns3 {

struct GnuplotDataset::Data {

  unsigned int m_references;

  std::string m_title;
  std::string m_extra;

  Data(const std::string &title);

  virtual ~Data();

  virtual std::string GetCommand() const = 0;

  virtual void PrintExpression(std::ostream &os, bool generateOneOutputFile,
                               unsigned int dataFileDatasetIndex,
                               std::string &dataFileName) const = 0;

  virtual void PrintDataFile(std::ostream &os,
                             bool generateOneOutputFile) const = 0;

  virtual bool IsEmpty() const = 0;
};

GnuplotDataset::Data::Data(const std::string &title)
    : m_references(1), m_title(title), m_extra(m_defaultExtra) {}

GnuplotDataset::Data::~Data() {}

std::string GnuplotDataset::m_defaultExtra = "";

GnuplotDataset::GnuplotDataset(Data *data) : m_data(data) {}

GnuplotDataset::GnuplotDataset(const GnuplotDataset &original)
    : m_data(original.m_data) {
  ++m_data->m_references;
}

GnuplotDataset::~GnuplotDataset() {
  if (--m_data->m_references == 0) {
    delete m_data;
  }
}

GnuplotDataset &GnuplotDataset::operator=(const GnuplotDataset &original) {
  if (this != &original) {
    if (--m_data->m_references == 0) {
      delete m_data;
    }

    m_data = original.m_data;
    ++m_data->m_references;
  }
  return *this;
}

void GnuplotDataset::SetTitle(const std::string &title) {
  m_data->m_title = title;
}

void GnuplotDataset::SetDefaultExtra(const std::string &extra) {
  m_defaultExtra = extra;
}

void GnuplotDataset::SetExtra(const std::string &extra) {
  m_data->m_extra = extra;
}

struct Gnuplot2dDataset::Data2d : public GnuplotDataset::Data {

  Style m_style;
  ErrorBars m_errorBars;

  PointSet m_pointset;

  Data2d(const std::string &title);

  std::string GetCommand() const override;
  void PrintExpression(std::ostream &os, bool generateOneOutputFile,
                       unsigned int dataFileDatasetIndex,
                       std::string &dataFileName) const override;
  void PrintDataFile(std::ostream &os,
                     bool generateOneOutputFile) const override;
  bool IsEmpty() const override;
};

Gnuplot2dDataset::Data2d::Data2d(const std::string &title)
    : Data(title), m_style(m_defaultStyle), m_errorBars(m_defaultErrorBars) {}

std::string Gnuplot2dDataset::Data2d::GetCommand() const { return "plot"; }

void Gnuplot2dDataset::Data2d::PrintExpression(
    std::ostream &os, bool generateOneOutputFile,
    unsigned int dataFileDatasetIndex, std::string &dataFileName) const {
  if (generateOneOutputFile) {
    os << "\"-\" ";
  } else {
    os << "\"" << dataFileName << "\" index " << dataFileDatasetIndex;
  }

  if (!m_title.empty()) {
    os << " title \"" << m_title << "\"";
  }

  switch (m_style) {
  case LINES:
    os << " with lines";
    break;
  case POINTS:
    switch (m_errorBars) {
    case NONE:
      os << " with points";
      break;
    case X:
      os << " with xerrorbars";
      break;
    case Y:
      os << " with yerrorbars";
      break;
    case XY:
      os << " with xyerrorbars";
      break;
    }
    break;
  case LINES_POINTS:
    switch (m_errorBars) {
    case NONE:
      os << " with linespoints";
      break;
    case X:
      os << " with errorlines";
      break;
    case Y:
      os << " with yerrorlines";
      break;
    case XY:
      os << " with xyerrorlines";
      break;
    }
    break;
  case DOTS:
    os << " with dots";
    break;
  case IMPULSES:
    os << " with impulses";
    break;
  case STEPS:
    os << " with steps";
    break;
  case FSTEPS:
    os << " with fsteps";
    break;
  case HISTEPS:
    os << " with histeps";
    break;
  }

  if (!m_extra.empty()) {
    os << " " << m_extra;
  }
}

void Gnuplot2dDataset::Data2d::PrintDataFile(std::ostream &os,
                                             bool generateOneOutputFile) const {
  for (auto i = m_pointset.begin(); i != m_pointset.end(); ++i) {
    if (i->empty) {
      os << std::endl;
      continue;
    }

    switch (m_errorBars) {
    case NONE:
      os << i->x << " " << i->y << std::endl;
      break;
    case X:
      os << i->x << " " << i->y << " " << i->dx << std::endl;
      break;
    case Y:
      os << i->x << " " << i->y << " " << i->dy << std::endl;
      break;
    case XY:
      os << i->x << " " << i->y << " " << i->dx << " " << i->dy << std::endl;
      break;
    }
  }

  if (generateOneOutputFile) {
    os << "e" << std::endl;
  } else {
    os << std::endl;
    os << std::endl;
  }
}

bool Gnuplot2dDataset::Data2d::IsEmpty() const { return m_pointset.empty(); }

Gnuplot2dDataset::Style Gnuplot2dDataset::m_defaultStyle = LINES;
Gnuplot2dDataset::ErrorBars Gnuplot2dDataset::m_defaultErrorBars = NONE;

Gnuplot2dDataset::Gnuplot2dDataset(const std::string &title)
    : GnuplotDataset(new Data2d(title)) {}

void Gnuplot2dDataset::SetDefaultStyle(Style style) { m_defaultStyle = style; }

void Gnuplot2dDataset::SetStyle(Style style) {
  reinterpret_cast<Data2d *>(m_data)->m_style = style;
}

void Gnuplot2dDataset::SetDefaultErrorBars(ErrorBars errorBars) {
  m_defaultErrorBars = errorBars;
}

void Gnuplot2dDataset::SetErrorBars(ErrorBars errorBars) {
  reinterpret_cast<Data2d *>(m_data)->m_errorBars = errorBars;
}

void Gnuplot2dDataset::Add(double x, double y) {
  NS_ASSERT(reinterpret_cast<Data2d *>(m_data)->m_errorBars == NONE);

  Point data;
  data.empty = false;
  data.x = x;
  data.y = y;
  data.dx = 0.0;
  data.dy = 0.0;
  reinterpret_cast<Data2d *>(m_data)->m_pointset.push_back(data);
}

void Gnuplot2dDataset::Add(double x, double y, double errorDelta) {
  NS_ASSERT(reinterpret_cast<Data2d *>(m_data)->m_errorBars == X ||
            reinterpret_cast<Data2d *>(m_data)->m_errorBars == Y);

  Point data;
  data.empty = false;
  data.x = x;
  data.y = y;
  data.dx = errorDelta;
  data.dy = errorDelta;
  reinterpret_cast<Data2d *>(m_data)->m_pointset.push_back(data);
}

void Gnuplot2dDataset::Add(double x, double y, double xErrorDelta,
                           double yErrorDelta) {
  NS_ASSERT(reinterpret_cast<Data2d *>(m_data)->m_errorBars == XY);

  Point data;
  data.empty = false;
  data.x = x;
  data.y = y;
  data.dx = xErrorDelta;
  data.dy = yErrorDelta;
  reinterpret_cast<Data2d *>(m_data)->m_pointset.push_back(data);
}

void Gnuplot2dDataset::AddEmptyLine() {
  Point data;
  data.empty = true;
  reinterpret_cast<Data2d *>(m_data)->m_pointset.push_back(data);
}

struct Gnuplot2dFunction::Function2d : public GnuplotDataset::Data {

  std::string m_function;

  Function2d(const std::string &title, const std::string &function);

  std::string GetCommand() const override;
  void PrintExpression(std::ostream &os, bool generateOneOutputFile,
                       unsigned int dataFileDatasetIndex,
                       std::string &dataFileName) const override;
  void PrintDataFile(std::ostream &os,
                     bool generateOneOutputFile) const override;
  bool IsEmpty() const override;
};

Gnuplot2dFunction::Function2d::Function2d(const std::string &title,
                                          const std::string &function)
    : Data(title), m_function(function) {}

std::string Gnuplot2dFunction::Function2d::GetCommand() const { return "plot"; }

void Gnuplot2dFunction::Function2d::PrintExpression(
    std::ostream &os, bool generateOneOutputFile,
    unsigned int dataFileDatasetIndex, std::string &dataFileName) const {
  os << m_function;

  if (!m_title.empty()) {
    os << " title \"" << m_title << "\"";
  }

  if (!m_extra.empty()) {
    os << " " << m_extra;
  }
}

void Gnuplot2dFunction::Function2d::PrintDataFile(
    std::ostream &os, bool generateOneOutputFile) const {}

bool Gnuplot2dFunction::Function2d::IsEmpty() const { return false; }

Gnuplot2dFunction::Gnuplot2dFunction(const std::string &title,
                                     const std::string &function)
    : GnuplotDataset(new Function2d(title, function)) {}

void Gnuplot2dFunction::SetFunction(const std::string &function) {
  reinterpret_cast<Function2d *>(m_data)->m_function = function;
}

struct Gnuplot3dDataset::Data3d : public GnuplotDataset::Data {

  std::string m_style;

  PointSet m_pointset;

  Data3d(const std::string &title);

  std::string GetCommand() const override;
  void PrintExpression(std::ostream &os, bool generateOneOutputFile,
                       unsigned int dataFileDatasetIndex,
                       std::string &dataFileName) const override;
  void PrintDataFile(std::ostream &os,
                     bool generateOneOutputFile) const override;
  bool IsEmpty() const override;
};

Gnuplot3dDataset::Data3d::Data3d(const std::string &title)
    : Data(title), m_style(m_defaultStyle) {}

std::string Gnuplot3dDataset::Data3d::GetCommand() const { return "splot"; }

void Gnuplot3dDataset::Data3d::PrintExpression(
    std::ostream &os, bool generateOneOutputFile,
    unsigned int dataFileDatasetIndex, std::string &dataFileName) const {
  os << "\"-\" ";

  if (!m_style.empty()) {
    os << " " << m_style;
  }

  if (!m_title.empty()) {
    os << " title \"" << m_title << "\"";
  }

  if (!m_extra.empty()) {
    os << " " << m_extra;
  }
}

void Gnuplot3dDataset::Data3d::PrintDataFile(std::ostream &os,
                                             bool generateOneOutputFile) const {
  for (auto i = m_pointset.begin(); i != m_pointset.end(); ++i) {
    if (i->empty) {
      os << std::endl;
      continue;
    }

    os << i->x << " " << i->y << " " << i->z << std::endl;
  }
  os << "e" << std::endl;
}

bool Gnuplot3dDataset::Data3d::IsEmpty() const { return m_pointset.empty(); }

std::string Gnuplot3dDataset::m_defaultStyle = "";

Gnuplot3dDataset::Gnuplot3dDataset(const std::string &title)
    : GnuplotDataset(new Data3d(title)) {}

void Gnuplot3dDataset::SetDefaultStyle(const std::string &style) {
  m_defaultStyle = style;
}

void Gnuplot3dDataset::SetStyle(const std::string &style) {
  reinterpret_cast<Data3d *>(m_data)->m_style = style;
}

void Gnuplot3dDataset::Add(double x, double y, double z) {
  Point data;
  data.empty = false;
  data.x = x;
  data.y = y;
  data.z = z;
  reinterpret_cast<Data3d *>(m_data)->m_pointset.push_back(data);
}

void Gnuplot3dDataset::AddEmptyLine() {
  Point data;
  data.empty = true;
  reinterpret_cast<Data3d *>(m_data)->m_pointset.push_back(data);
}

struct Gnuplot3dFunction::Function3d : public GnuplotDataset::Data {

  std::string m_function;

  Function3d(const std::string &title, const std::string &function);

  std::string GetCommand() const override;
  void PrintExpression(std::ostream &os, bool generateOneOutputFile,
                       unsigned int dataFileDatasetIndex,
                       std::string &dataFileName) const override;
  void PrintDataFile(std::ostream &os,
                     bool generateOneOutputFile) const override;
  bool IsEmpty() const override;
};

Gnuplot3dFunction::Function3d::Function3d(const std::string &title,
                                          const std::string &function)
    : Data(title), m_function(function) {}

std::string Gnuplot3dFunction::Function3d::GetCommand() const {
  return "splot";
}

void Gnuplot3dFunction::Function3d::PrintExpression(
    std::ostream &os, bool generateOneOutputFile,
    unsigned int dataFileDatasetIndex, std::string &dataFileName) const {
  os << m_function;

  if (!m_title.empty()) {
    os << " title \"" << m_title << "\"";
  }

  if (!m_extra.empty()) {
    os << " " << m_extra;
  }
}

void Gnuplot3dFunction::Function3d::PrintDataFile(
    std::ostream &os, bool generateOneOutputFile) const {}

bool Gnuplot3dFunction::Function3d::IsEmpty() const { return false; }

Gnuplot3dFunction::Gnuplot3dFunction(const std::string &title,
                                     const std::string &function)
    : GnuplotDataset(new Function3d(title, function)) {}

void Gnuplot3dFunction::SetFunction(const std::string &function) {
  reinterpret_cast<Function3d *>(m_data)->m_function = function;
}

Gnuplot::Gnuplot(const std::string &outputFilename, const std::string &title)
    : m_outputFilename(outputFilename),
      m_terminal(DetectTerminal(outputFilename)), m_title(title),
      m_generateOneOutputFile(false), m_dataFileDatasetIndex(0) {}

void Gnuplot::SetOutputFilename(const std::string &outputFilename) {
  m_outputFilename = outputFilename;
}

std::string Gnuplot::DetectTerminal(const std::string &filename) {
  std::string::size_type dotpos = filename.rfind('.');
  if (dotpos == std::string::npos) {
    return "";
  }

  if (filename.substr(dotpos) == ".png") {
    return "png";
  } else if (filename.substr(dotpos) == ".pdf") {
    return "pdf";
  }

  return "";
}

void Gnuplot::SetTerminal(const std::string &terminal) {
  m_terminal = terminal;
}

void Gnuplot::SetTitle(const std::string &title) { m_title = title; }

void Gnuplot::SetLegend(const std::string &xLegend,
                        const std::string &yLegend) {
  m_xLegend = xLegend;
  m_yLegend = yLegend;
}

void Gnuplot::SetExtra(const std::string &extra) { m_extra = extra; }

void Gnuplot::AppendExtra(const std::string &extra) {
  m_extra += "\n";
  m_extra += extra;
}

void Gnuplot::AddDataset(const GnuplotDataset &dataset) {
  m_datasets.push_back(dataset);
}

void Gnuplot::GenerateOutput(std::ostream &os) {
  m_generateOneOutputFile = true;

  GenerateOutput(os, os, "");
}

void Gnuplot::GenerateOutput(std::ostream &osControl, std::ostream &osData,
                             std::string dataFileName) {
  if (!m_terminal.empty()) {
    osControl << "set terminal " << m_terminal << std::endl;
  }

  if (!m_outputFilename.empty()) {
    osControl << "set output \"" << m_outputFilename << "\"" << std::endl;
  }

  if (!m_title.empty()) {
    osControl << "set title \"" << m_title << "\"" << std::endl;
  }

  if (!m_xLegend.empty()) {
    osControl << "set xlabel \"" << m_xLegend << "\"" << std::endl;
  }

  if (!m_yLegend.empty()) {
    osControl << "set ylabel \"" << m_yLegend << "\"" << std::endl;
  }

  if (!m_extra.empty()) {
    osControl << m_extra << std::endl;
  }

  if (m_datasets.empty()) {
    return;
  }

  std::string command = m_datasets.begin()->m_data->GetCommand();

  for (auto i = m_datasets.begin() + 1; i != m_datasets.end(); ++i) {
    NS_ASSERT_MSG(command == i->m_data->GetCommand(),
                  "Cannot mix 'plot' and 'splot' GnuplotDatasets.");
  }

  osControl << command << " ";

  bool isDataEmpty;
  for (auto i = m_datasets.begin(); i != m_datasets.end();) {
    isDataEmpty = i->m_data->IsEmpty();
    if (!isDataEmpty) {
      i->m_data->PrintExpression(osControl, m_generateOneOutputFile,
                                 m_dataFileDatasetIndex, dataFileName);

      m_dataFileDatasetIndex++;
    }

    i++;
    if (i != m_datasets.end() && !isDataEmpty) {
      osControl << ", ";
    }
  }
  osControl << std::endl;

  for (auto i = m_datasets.begin(); i != m_datasets.end(); i++) {
    i->m_data->PrintDataFile(osData, m_generateOneOutputFile);
  }
}

void Gnuplot::SetDataFileDatasetIndex(unsigned int index) {
  m_dataFileDatasetIndex = index;
}

GnuplotCollection::GnuplotCollection(const std::string &outputFilename)
    : m_outputFilename(outputFilename),
      m_terminal(Gnuplot::DetectTerminal(outputFilename)) {}

void GnuplotCollection::SetTerminal(const std::string &terminal) {
  m_terminal = terminal;
}

void GnuplotCollection::AddPlot(const Gnuplot &plot) {
  m_plots.push_back(plot);
}

Gnuplot &GnuplotCollection::GetPlot(unsigned int id) {
  if (id >= m_plots.size()) {
    throw(std::range_error("Gnuplot id is out of range"));
  } else {
    return m_plots[id];
  }
}

void GnuplotCollection::GenerateOutput(std::ostream &os) {

  if (!m_terminal.empty()) {
    os << "set terminal " << m_terminal << std::endl;
  }

  if (!m_outputFilename.empty()) {
    os << "set output \"" << m_outputFilename << "\"" << std::endl;
  }

  for (auto i = m_plots.begin(); i != m_plots.end(); ++i) {
    i->GenerateOutput(os);
  }
}

void GnuplotCollection::GenerateOutput(std::ostream &osControl,
                                       std::ostream &osData,
                                       std::string dataFileName) {

  if (!m_terminal.empty()) {
    osControl << "set terminal " << m_terminal << std::endl;
  }

  if (!m_outputFilename.empty()) {
    osControl << "set output \"" << m_outputFilename << "\"" << std::endl;
  }

  for (auto i = m_plots.begin(); i != m_plots.end(); ++i) {
    i->GenerateOutput(osControl, osData, dataFileName);
  }
}

} // namespace ns3
