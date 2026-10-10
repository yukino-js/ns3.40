#ifndef PROPAGATION_DELAY_MODEL_H
#define PROPAGATION_DELAY_MODEL_H

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class MobilityModel;

class PropagationDelayModel : public Object {
public:
  static TypeId GetTypeId();
  ~PropagationDelayModel() override;
  virtual Time GetDelay(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const = 0;
  int64_t AssignStreams(int64_t stream);

protected:
  virtual int64_t DoAssignStreams(int64_t stream) = 0;
};

class RandomPropagationDelayModel : public PropagationDelayModel {
public:
  static TypeId GetTypeId();

  RandomPropagationDelayModel();
  ~RandomPropagationDelayModel() override;
  Time GetDelay(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const override;

private:
  int64_t DoAssignStreams(int64_t stream) override;
  Ptr<RandomVariableStream> m_variable;
};

class ConstantSpeedPropagationDelayModel : public PropagationDelayModel {
public:
  static TypeId GetTypeId();

  ConstantSpeedPropagationDelayModel();
  Time GetDelay(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const override;
  void SetSpeed(double speed);
  double GetSpeed() const;

private:
  int64_t DoAssignStreams(int64_t stream) override;
  double m_speed;
};

} // namespace ns3

#endif
