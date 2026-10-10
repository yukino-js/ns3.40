
#ifndef SIMPLE_FRAME_CAPTURE_MODEL_H
#define SIMPLE_FRAME_CAPTURE_MODEL_H

#include "frame-capture-model.h"

namespace ns3 {
class SimpleFrameCaptureModel : public FrameCaptureModel {
public:
  static TypeId GetTypeId();

  SimpleFrameCaptureModel();
  ~SimpleFrameCaptureModel() override;

  void SetMargin(double margin);
  double GetMargin() const;

  bool CaptureNewFrame(Ptr<Event> currentEvent,
                       Ptr<Event> newEvent) const override;

private:
  double m_margin;
};

} // namespace ns3

#endif
