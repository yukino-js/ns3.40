
#include "ns3/gnuplot.h"

#include <fstream>

using namespace ns3;

namespace {

void Create2DPlotFile() {
  std::string fileNameWithNoExtension = "plot-2d";
  std::string graphicsFileName = fileNameWithNoExtension + ".png";
  std::string plotFileName = fileNameWithNoExtension + ".plt";
  std::string plotTitle = "2-D Plot";
  std::string dataTitle = "2-D Data";

  Gnuplot plot(graphicsFileName);
  plot.SetTitle(plotTitle);

  plot.SetTerminal("png");

  plot.SetLegend("X Values", "Y Values");

  plot.AppendExtra("set xrange [-6:+6]");

  Gnuplot2dDataset dataset;
  dataset.SetTitle(dataTitle);
  dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);

  double x;
  double y;

  for (x = -5.0; x <= +5.0; x += 1.0) {
    y = x * x;

    dataset.Add(x, y);
  }

  plot.AddDataset(dataset);

  std::ofstream plotFile(plotFileName);

  plot.GenerateOutput(plotFile);

  plotFile.close();
}

void Create2DPlotWithErrorBarsFile() {
  std::string fileNameWithNoExtension = "plot-2d-with-error-bars";
  std::string graphicsFileName = fileNameWithNoExtension + ".png";
  std::string plotFileName = fileNameWithNoExtension + ".plt";
  std::string plotTitle = "2-D Plot With Error Bars";
  std::string dataTitle = "2-D Data With Error Bars";

  Gnuplot plot(graphicsFileName);
  plot.SetTitle(plotTitle);

  plot.SetTerminal("png");

  plot.SetLegend("X Values", "Y Values");

  plot.AppendExtra("set xrange [-6:+6]");

  Gnuplot2dDataset dataset;
  dataset.SetTitle(dataTitle);
  dataset.SetStyle(Gnuplot2dDataset::POINTS);

  dataset.SetErrorBars(Gnuplot2dDataset::XY);

  double x;
  double xErrorDelta;
  double y;
  double yErrorDelta;

  for (x = -5.0; x <= +5.0; x += 1.0) {
    y = x * x;

    xErrorDelta = 0.25;
    yErrorDelta = 0.1 * y;

    dataset.Add(x, y, xErrorDelta, yErrorDelta);
  }

  plot.AddDataset(dataset);

  std::ofstream plotFile(plotFileName);

  plot.GenerateOutput(plotFile);

  plotFile.close();
}

void Create3DPlotFile() {
  std::string fileNameWithNoExtension = "plot-3d";
  std::string graphicsFileName = fileNameWithNoExtension + ".png";
  std::string plotFileName = fileNameWithNoExtension + ".plt";
  std::string plotTitle = "3-D Plot";
  std::string dataTitle = "3-D Data";

  Gnuplot plot(graphicsFileName);
  plot.SetTitle(plotTitle);

  plot.SetTerminal("png");

  plot.AppendExtra("set view 30, 120, 1.0, 1.0");

  plot.AppendExtra("set ticslevel 0");

  plot.AppendExtra("set xlabel \"X Values\"");
  plot.AppendExtra("set ylabel \"Y Values\"");
  plot.AppendExtra("set zlabel \"Z Values\"");

  plot.AppendExtra("set xrange [-5:+5]");
  plot.AppendExtra("set yrange [-5:+5]");

  Gnuplot3dDataset dataset;
  dataset.SetTitle(dataTitle);
  dataset.SetStyle("with lines");

  double x;
  double y;
  double z;

  for (x = -5.0; x <= +5.0; x += 1.0) {
    for (y = -5.0; y <= +5.0; y += 1.0) {
      z = x * x * y * y;

      dataset.Add(x, y, z);
    }

    dataset.AddEmptyLine();
  }

  plot.AddDataset(dataset);

  std::ofstream plotFile(plotFileName);

  plot.GenerateOutput(plotFile);

  plotFile.close();
}

} // namespace

int main(int argc, char *argv[]) {
  Create2DPlotFile();

  Create2DPlotWithErrorBarsFile();

  Create3DPlotFile();

  return 0;
}
