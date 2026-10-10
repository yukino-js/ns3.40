#ifndef SIMULATION_SINGLETON_H
#define SIMULATION_SINGLETON_H

namespace ns3 {

template <typename T> class SimulationSingleton {
public:
  SimulationSingleton<T>() = delete;
  SimulationSingleton<T>(const SimulationSingleton<T> &) = delete;
  SimulationSingleton<T> &operator=(const SimulationSingleton<T> &) = delete;

  static T *Get();

private:
  static T **GetObject();

  static void DeleteObject();
};

} // namespace ns3

#include "simulator.h"

namespace ns3 {

template <typename T> T *SimulationSingleton<T>::Get() {
  T **ppobject = GetObject();
  return *ppobject;
}

template <typename T> T **SimulationSingleton<T>::GetObject() {
  static T *pobject = nullptr;
  if (pobject == nullptr) {
    pobject = new T();
    Simulator::ScheduleDestroy(&SimulationSingleton<T>::DeleteObject);
  }
  return &pobject;
}

template <typename T> void SimulationSingleton<T>::DeleteObject() {
  T **ppobject = GetObject();
  delete (*ppobject);
  *ppobject = nullptr;
}

} // namespace ns3

#endif
