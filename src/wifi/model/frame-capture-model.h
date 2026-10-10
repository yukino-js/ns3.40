
#ifndef FRAME_CAPTURE_MODEL_H
#define FRAME_CAPTURE_MODEL_H

#include "ns3/nstime.h"
#include "ns3/object.h"

namespace ns3 {

class Event;
class Time;

class FrameCaptureModel : public Object {
public:
  static TypeId GetTypeId();

  virtual bool CaptureNewFrame(Ptr<Event> currentEvent,
                               Ptr<Event> newEvent) const = 0;

  virtual bool IsInCaptureWindow(Time timePreambleDetected) const;

private:
  Time m_captureWindow;
};

} // namespace ns3

#endif
