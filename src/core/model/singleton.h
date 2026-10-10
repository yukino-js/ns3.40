#ifndef SINGLETON_H
#define SINGLETON_H

namespace ns3 {

template <typename T> class Singleton {
public:
  Singleton<T>(const Singleton<T> &) = delete;
  Singleton<T> &operator=(const Singleton<T> &) = delete;

  static T *Get();

protected:
  Singleton<T>() {}

  virtual ~Singleton<T>() {}
};

} // namespace ns3

namespace ns3 {

template <typename T> T *Singleton<T>::Get() {
  static T object;
  return &object;
}

} // namespace ns3

#endif
