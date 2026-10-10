#ifndef GNUPLOT_H
#define GNUPLOT_H

#include <string>
#include <utility>
#include <vector>

namespace ns3 {

class GnuplotDataset {
public:
  GnuplotDataset(const GnuplotDataset &original);

  ~GnuplotDataset();

  GnuplotDataset &operator=(const GnuplotDataset &original);

  void SetTitle(const std::string &title);

  static void SetDefaultExtra(const std::string &extra);

  void SetExtra(const std::string &extra);

protected:
  friend class Gnuplot;

  static std::string m_defaultExtra;

  struct Data;

  GnuplotDataset(Data *data);

  Data *m_data;
};

class Gnuplot2dDataset : public GnuplotDataset {
public:
  enum Style {
    LINES,
    POINTS,
    LINES_POINTS,
    DOTS,
    IMPULSES,
    STEPS,
    FSTEPS,
    HISTEPS,
  };

  enum ErrorBars { NONE, X, Y, XY };

  Gnuplot2dDataset(const std::string &title = "Untitled");

  static void SetDefaultStyle(Style style);

  void SetStyle(Style style);

  static void SetDefaultErrorBars(ErrorBars errorBars);

  void SetErrorBars(ErrorBars errorBars);

  void Add(double x, double y);

  void Add(double x, double y, double errorDelta);

  void Add(double x, double y, double xErrorDelta, double yErrorDelta);

  void AddEmptyLine();

private:
  struct Point {
    bool empty;
    double x;
    double y;
    double dx;
    double dy;
  };

  typedef std::vector<Point> PointSet;

  static Style m_defaultStyle;
  static ErrorBars m_defaultErrorBars;

  struct Data2d;
};

class Gnuplot2dFunction : public GnuplotDataset {
public:
  Gnuplot2dFunction(const std::string &title = "Untitled",
                    const std::string &function = "");

  void SetFunction(const std::string &function);

private:
  struct Function2d;
};

class Gnuplot3dDataset : public GnuplotDataset {
public:
  Gnuplot3dDataset(const std::string &title = "Untitled");

  static void SetDefaultStyle(const std::string &style);

  void SetStyle(const std::string &style);

  void Add(double x, double y, double z);

  void AddEmptyLine();

private:
  struct Point {
    bool empty;
    double x;
    double y;
    double z;
  };

  typedef std::vector<Point> PointSet;

  static std::string m_defaultStyle;

  struct Data3d;
};

class Gnuplot3dFunction : public GnuplotDataset {
public:
  Gnuplot3dFunction(const std::string &title = "Untitled",
                    const std::string &function = "");

  void SetFunction(const std::string &function);

private:
  struct Function3d;
};

class Gnuplot {
public:
  Gnuplot(const std::string &outputFilename = "",
          const std::string &title = "");

  void SetOutputFilename(const std::string &outputFilename);

  static std::string DetectTerminal(const std::string &filename);

  void SetTerminal(const std::string &terminal);

  void SetTitle(const std::string &title);

  void SetLegend(const std::string &xLegend, const std::string &yLegend);

  void SetExtra(const std::string &extra);

  void AppendExtra(const std::string &extra);

  void AddDataset(const GnuplotDataset &dataset);

  void GenerateOutput(std::ostream &os);

  void GenerateOutput(std::ostream &osControl, std::ostream &osData,
                      std::string dataFileName);

  void SetDataFileDatasetIndex(unsigned int index);

private:
  typedef std::vector<GnuplotDataset> Datasets;

  std::string m_outputFilename;
  std::string m_terminal;

  Datasets m_datasets;

  std::string m_title;
  std::string m_xLegend;
  std::string m_yLegend;
  std::string m_extra;

  bool m_generateOneOutputFile;

  unsigned int m_dataFileDatasetIndex;
};

class GnuplotCollection {
public:
  GnuplotCollection(const std::string &outputFilename);

  void SetTerminal(const std::string &terminal);

  void AddPlot(const Gnuplot &plot);

  Gnuplot &GetPlot(unsigned int id);

  void GenerateOutput(std::ostream &os);

  void GenerateOutput(std::ostream &osControl, std::ostream &osData,
                      std::string dataFileName);

private:
  typedef std::vector<Gnuplot> Plots;

  std::string m_outputFilename;
  std::string m_terminal;

  Plots m_plots;
};

} // namespace ns3

#endif
