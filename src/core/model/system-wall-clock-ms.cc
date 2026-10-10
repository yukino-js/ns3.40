
#include "system-wall-clock-ms.h"

#include "log.h"

#include <chrono>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SystemWallClockMs");

class SystemWallClockMsPrivate {
public:
  void Start();
  int64_t End();
  int64_t GetElapsedReal() const;
  int64_t GetElapsedUser() const;
  int64_t GetElapsedSystem() const;

private:
  std::chrono::system_clock::time_point m_startTime;
  int64_t m_elapsedReal;
  int64_t m_elapsedUser;
  int64_t m_elapsedSystem;
};

void SystemWallClockMsPrivate::Start() {
  NS_LOG_FUNCTION(this);
  m_startTime = std::chrono::system_clock::now();
}

int64_t SystemWallClockMsPrivate::End() {
  NS_LOG_FUNCTION(this);

  auto endTime = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = endTime - m_startTime;
  m_elapsedReal =
      std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_seconds)
          .count();

  m_elapsedUser = 0;
  m_elapsedSystem = 0;

  return m_elapsedReal;
}

int64_t SystemWallClockMsPrivate::GetElapsedReal() const {
  NS_LOG_FUNCTION(this);
  return m_elapsedReal;
}

int64_t SystemWallClockMsPrivate::GetElapsedUser() const {
  NS_LOG_FUNCTION(this);
  return m_elapsedUser;
}

int64_t SystemWallClockMsPrivate::GetElapsedSystem() const {
  NS_LOG_FUNCTION(this);
  return m_elapsedSystem;
}

SystemWallClockMs::SystemWallClockMs()
    : m_priv(new SystemWallClockMsPrivate()) {
  NS_LOG_FUNCTION(this);
}

SystemWallClockMs::~SystemWallClockMs() {
  NS_LOG_FUNCTION(this);
  delete m_priv;
  m_priv = nullptr;
}

void SystemWallClockMs::Start() {
  NS_LOG_FUNCTION(this);
  m_priv->Start();
}

int64_t SystemWallClockMs::End() {
  NS_LOG_FUNCTION(this);
  return m_priv->End();
}

int64_t SystemWallClockMs::GetElapsedReal() const {
  NS_LOG_FUNCTION(this);
  return m_priv->GetElapsedReal();
}

int64_t SystemWallClockMs::GetElapsedUser() const {
  NS_LOG_FUNCTION(this);
  return m_priv->GetElapsedUser();
}

int64_t SystemWallClockMs::GetElapsedSystem() const {
  NS_LOG_FUNCTION(this);
  return m_priv->GetElapsedSystem();
}

} // namespace ns3
