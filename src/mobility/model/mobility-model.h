#ifndef MOBILITY_MODEL_H
#define MOBILITY_MODEL_H

#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/vector.h"

namespace ns3 {

class MobilityModel : public Object {
public:
  static TypeId GetTypeId();
  MobilityModel();
  ~MobilityModel() override = 0;

  Vector GetPosition() const;
  Vector GetPositionWithReference(const Vector &referencePosition) const;
  void SetPosition(const Vector &position);
  Vector GetVelocity() const;
  double GetDistanceFrom(Ptr<const MobilityModel> position) const;
  double GetRelativeSpeed(Ptr<const MobilityModel> other) const;
  int64_t AssignStreams(int64_t stream);

  typedef void (*TracedCallback)(Ptr<const MobilityModel> model);

protected:
  void NotifyCourseChange() const;

private:
  virtual Vector DoGetPosition() const = 0;
  virtual Vector
  DoGetPositionWithReference(const Vector &referencePosition) const;
  virtual void DoSetPosition(const Vector &position) = 0;
  virtual Vector DoGetVelocity() const = 0;
  virtual int64_t DoAssignStreams(int64_t start);

  ns3::TracedCallback<Ptr<const MobilityModel>> m_courseChangeTrace;
};

} // namespace ns3

#endif
