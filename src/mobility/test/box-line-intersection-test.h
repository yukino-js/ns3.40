
#ifndef BOX_LINE_INTERSECTION_TEST_H
#define BOX_LINE_INTERSECTION_TEST_H

#include <ns3/box.h>
#include <ns3/test.h>
#include <ns3/vector.h>

using namespace ns3;

class BoxLineIntersectionTestSuite : public TestSuite {
public:
  BoxLineIntersectionTestSuite();
};

class BoxLineIntersectionTestCase : public TestCase {
public:
  BoxLineIntersectionTestCase(uint16_t indexPos1, uint16_t indexPos2, Box box,
                              bool intersect);
  std::string BuildNameString(uint16_t indexPos1, uint16_t indexPos2, Box box,
                              bool intersect);
  ~BoxLineIntersectionTestCase() override;

private:
  void DoRun() override;
  Vector CreatePosition(uint16_t index, double boxHeight);

  uint16_t m_indexPos1{0};
  uint16_t m_indexPos2{0};
  Box m_box;
  bool m_intersect{false};
};

#endif
