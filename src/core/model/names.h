
#ifndef OBJECT_NAMES_H
#define OBJECT_NAMES_H

#include "object.h"
#include "ptr.h"

namespace ns3 {

class Names {
public:
  static void Add(std::string name, Ptr<Object> object);

  static void Add(std::string path, std::string name, Ptr<Object> object);

  static void Add(Ptr<Object> context, std::string name, Ptr<Object> object);

  static void Rename(std::string oldpath, std::string newname);

  static void Rename(std::string path, std::string oldname,
                     std::string newname);

  static void Rename(Ptr<Object> context, std::string oldname,
                     std::string newname);

  static std::string FindName(Ptr<Object> object);

  static std::string FindPath(Ptr<Object> object);

  static void Clear();

  template <typename T> static Ptr<T> Find(std::string path);

  template <typename T> static Ptr<T> Find(std::string path, std::string name);

  template <typename T>
  static Ptr<T> Find(Ptr<Object> context, std::string name);

private:
  static Ptr<Object> FindInternal(std::string path);

  static Ptr<Object> FindInternal(std::string path, std::string name);

  static Ptr<Object> FindInternal(Ptr<Object> context, std::string name);
};

template <typename T> Ptr<T> Names::Find(std::string path) {
  Ptr<Object> obj = FindInternal(path);
  if (obj) {
    return obj->GetObject<T>();
  } else {
    return nullptr;
  }
}

template <typename T> Ptr<T> Names::Find(std::string path, std::string name) {
  Ptr<Object> obj = FindInternal(path, name);
  if (obj) {
    return obj->GetObject<T>();
  } else {
    return nullptr;
  }
}

template <typename T>
Ptr<T> Names::Find(Ptr<Object> context, std::string name) {
  Ptr<Object> obj = FindInternal(context, name);
  if (obj) {
    return obj->GetObject<T>();
  } else {
    return nullptr;
  }
}

} // namespace ns3

#endif
