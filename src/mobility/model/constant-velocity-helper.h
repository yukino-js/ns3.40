#ifndef CONSTANT_VELOCITY_HELPER_H
#define CONSTANT_VELOCITY_HELPER_H

#include "box.h"

#include "ns3/nstime.h"
#include "ns3/vector.h"

namespace ns3 {

class Rectangle;

class ConstantVelocityHelper {
public:
  ConstantVelocityHelper();
  ConstantVelocityHelper(const Vector &position);
  ConstantVelocityHelper(const Vector &position, const Vector &vel);

  void SetPosition(const Vector &position);
  Vector GetCurrentPosition() const;
  Vector GetVelocity() const;
  void SetVelocity(const Vector &vel);
  void Pause();
  void Unpause();

  void UpdateWithBounds(const Rectangle &rectangle) const;
  void UpdateWithBounds(const Box &bounds) const;
  void Update() const;

private:
  mutable Time m_lastUpdate;
  mutable Vector m_position;
  Vector m_velocity;
  bool m_paused;
};

} // namespace ns3

#endif
