#ifndef POSITION_ALLOCATOR_H
#define POSITION_ALLOCATOR_H

#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include "ns3/vector.h"

namespace ns3 {

class PositionAllocator : public Object {
public:
  static TypeId GetTypeId();
  PositionAllocator();
  ~PositionAllocator() override;
  virtual Vector GetNext() const = 0;
  virtual int64_t AssignStreams(int64_t stream) = 0;
};

class ListPositionAllocator : public PositionAllocator {
public:
  static TypeId GetTypeId();
  ListPositionAllocator();

  void Add(Vector v);

  void Add(const std::string filePath, double defaultZ = 0,
           char delimiter = ',');

  uint32_t GetSize() const;
  Vector GetNext() const override;
  int64_t AssignStreams(int64_t stream) override;

private:
  std::vector<Vector> m_positions;
  mutable std::vector<Vector>::const_iterator m_current;
};

class GridPositionAllocator : public PositionAllocator {
public:
  static TypeId GetTypeId();

  enum LayoutType { ROW_FIRST, COLUMN_FIRST };

  GridPositionAllocator();

  void SetMinX(double xMin);
  void SetMinY(double yMin);
  void SetZ(double z);
  void SetDeltaX(double deltaX);
  void SetDeltaY(double deltaY);
  void SetN(uint32_t n);
  void SetLayoutType(LayoutType layoutType);

  double GetMinX() const;
  double GetMinY() const;
  double GetDeltaX() const;
  double GetDeltaY() const;
  uint32_t GetN() const;
  LayoutType GetLayoutType() const;

  Vector GetNext() const override;
  int64_t AssignStreams(int64_t stream) override;

private:
  mutable uint32_t m_current;
  LayoutType m_layoutType;
  double m_xMin;
  double m_yMin;
  double m_z;
  uint32_t m_n;
  double m_deltaX;
  double m_deltaY;
};

class RandomRectanglePositionAllocator : public PositionAllocator {
public:
  static TypeId GetTypeId();
  RandomRectanglePositionAllocator();
  ~RandomRectanglePositionAllocator() override;

  void SetX(Ptr<RandomVariableStream> x);
  void SetY(Ptr<RandomVariableStream> y);
  void SetZ(double z);

  Vector GetNext() const override;
  int64_t AssignStreams(int64_t stream) override;

private:
  Ptr<RandomVariableStream> m_x;
  Ptr<RandomVariableStream> m_y;
  double m_z;
};

class RandomBoxPositionAllocator : public PositionAllocator {
public:
  static TypeId GetTypeId();
  RandomBoxPositionAllocator();
  ~RandomBoxPositionAllocator() override;

  void SetX(Ptr<RandomVariableStream> x);
  void SetY(Ptr<RandomVariableStream> y);
  void SetZ(Ptr<RandomVariableStream> z);

  Vector GetNext() const override;
  int64_t AssignStreams(int64_t stream) override;

private:
  Ptr<RandomVariableStream> m_x;
  Ptr<RandomVariableStream> m_y;
  Ptr<RandomVariableStream> m_z;
};

class RandomDiscPositionAllocator : public PositionAllocator {
public:
  static TypeId GetTypeId();
  RandomDiscPositionAllocator();
  ~RandomDiscPositionAllocator() override;

  void SetTheta(Ptr<RandomVariableStream> theta);
  void SetRho(Ptr<RandomVariableStream> rho);
  void SetX(double x);
  void SetY(double y);
  void SetZ(double z);

  Vector GetNext() const override;
  int64_t AssignStreams(int64_t stream) override;

private:
  Ptr<RandomVariableStream> m_theta;
  Ptr<RandomVariableStream> m_rho;
  double m_x;
  double m_y;
  double m_z;
};

class UniformDiscPositionAllocator : public PositionAllocator {
public:
  static TypeId GetTypeId();
  UniformDiscPositionAllocator();
  ~UniformDiscPositionAllocator() override;

  void SetRho(double rho);

  void SetX(double x);

  void SetY(double y);

  void SetZ(double z);

  Vector GetNext() const override;
  int64_t AssignStreams(int64_t stream) override;

private:
  Ptr<UniformRandomVariable> m_rv;
  double m_rho;
  double m_x;
  double m_y;
  double m_z;
};

} // namespace ns3

#endif
