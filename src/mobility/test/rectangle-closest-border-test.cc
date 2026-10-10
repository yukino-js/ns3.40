

#include <ns3/rectangle.h>
#include <ns3/simulator.h>
#include <ns3/test.h>

using namespace ns3;

class RectangleClosestBorderTestSuite : public TestSuite {
public:
  RectangleClosestBorderTestSuite();
};

class RectangleClosestBorderTestCase : public TestCase {
public:
  RectangleClosestBorderTestCase(double x, double y, Rectangle rectangle,
                                 Rectangle::Side side);
  std::string BuildNameString(double x, double y, Rectangle rectangle,
                              Rectangle::Side side);
  ~RectangleClosestBorderTestCase() override;

private:
  void DoRun() override;

  double m_x{0.0};
  double m_y{0.0};
  Rectangle m_rectangle;

  Rectangle::Side m_side{ns3::Rectangle::TOPSIDE};
};

RectangleClosestBorderTestSuite::RectangleClosestBorderTestSuite()
    : TestSuite("rectangle-closest-border", UNIT) {
  Rectangle rectangle = Rectangle(0.0, 10.0, 0.0, 10.0);

  AddTestCase(
      new RectangleClosestBorderTestCase(-5, 5, rectangle, Rectangle::LEFTSIDE),
      TestCase::QUICK);
  AddTestCase(
      new RectangleClosestBorderTestCase(2, 5, rectangle, Rectangle::LEFTSIDE),
      TestCase::QUICK);
  AddTestCase(new RectangleClosestBorderTestCase(17, 5, rectangle,
                                                 Rectangle::RIGHTSIDE),
              TestCase::QUICK);
  AddTestCase(
      new RectangleClosestBorderTestCase(7, 5, rectangle, Rectangle::RIGHTSIDE),
      TestCase::QUICK);
  AddTestCase(new RectangleClosestBorderTestCase(5, -7, rectangle,
                                                 Rectangle::BOTTOMSIDE),
              TestCase::QUICK);
  AddTestCase(new RectangleClosestBorderTestCase(5, 1, rectangle,
                                                 Rectangle::BOTTOMSIDE),
              TestCase::QUICK);
  AddTestCase(
      new RectangleClosestBorderTestCase(5, 15, rectangle, Rectangle::TOPSIDE),
      TestCase::QUICK);
  AddTestCase(
      new RectangleClosestBorderTestCase(5, 7, rectangle, Rectangle::TOPSIDE),
      TestCase::QUICK);
  AddTestCase(new RectangleClosestBorderTestCase(-1, -1, rectangle,
                                                 Rectangle::BOTTOMLEFTCORNER),
              TestCase::QUICK);
  AddTestCase(new RectangleClosestBorderTestCase(0, 0, rectangle,
                                                 Rectangle::BOTTOMLEFTCORNER),
              TestCase::QUICK);
  AddTestCase(new RectangleClosestBorderTestCase(11, -1, rectangle,
                                                 Rectangle::BOTTOMRIGHTCORNER),
              TestCase::QUICK);
  AddTestCase(new RectangleClosestBorderTestCase(9, 1, rectangle,
                                                 Rectangle::BOTTOMRIGHTCORNER),
              TestCase::QUICK);
  AddTestCase(new RectangleClosestBorderTestCase(-1, 11, rectangle,
                                                 Rectangle::TOPLEFTCORNER),
              TestCase::QUICK);
  AddTestCase(new RectangleClosestBorderTestCase(1, 9, rectangle,
                                                 Rectangle::TOPLEFTCORNER),
              TestCase::QUICK);
  AddTestCase(new RectangleClosestBorderTestCase(11, 11, rectangle,
                                                 Rectangle::TOPRIGHTCORNER),
              TestCase::QUICK);
  AddTestCase(new RectangleClosestBorderTestCase(9, 9, rectangle,
                                                 Rectangle::TOPRIGHTCORNER),
              TestCase::QUICK);
  AddTestCase(
      new RectangleClosestBorderTestCase(5, 5, rectangle, Rectangle::TOPSIDE),
      TestCase::QUICK);
}

static RectangleClosestBorderTestSuite rectangleClosestBorderTestSuite;

RectangleClosestBorderTestCase::RectangleClosestBorderTestCase(
    double x, double y, Rectangle rectangle, Rectangle::Side side)
    : TestCase(BuildNameString(x, y, rectangle, side)), m_x(x), m_y(y),
      m_rectangle(rectangle), m_side(side) {}

RectangleClosestBorderTestCase::~RectangleClosestBorderTestCase() {}

std::string RectangleClosestBorderTestCase::BuildNameString(
    double x, double y, Rectangle rectangle, Rectangle::Side side) {
  std::ostringstream oss;
  oss << "Rectangle closest border test : checking"
      << " (x,y) = (" << x << "," << y << ") closest border to the rectangle [("
      << rectangle.xMin << ", " << rectangle.yMin << "), (" << rectangle.xMax
      << ", " << rectangle.yMax << ")]. The expected side = " << side;
  return oss.str();
}

void RectangleClosestBorderTestCase::DoRun() {
  Vector position(m_x, m_y, 0.0);
  Rectangle::Side side = m_rectangle.GetClosestSideOrCorner(position);

  NS_TEST_ASSERT_MSG_EQ(side, m_side, "Unexpected result of rectangle side!");
  Simulator::Destroy();
}
