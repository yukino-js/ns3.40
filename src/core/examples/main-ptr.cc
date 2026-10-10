
#include "ns3/command-line.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <iostream>

using namespace ns3;

class PtrExample : public Object {
public:
  PtrExample();
  ~PtrExample() override;
  void Method();
};

PtrExample::PtrExample() { std::cout << "PtrExample constructor" << std::endl; }

PtrExample::~PtrExample() { std::cout << "PtrExample destructor" << std::endl; }

void PtrExample::Method() { std::cout << "PtrExample method" << std::endl; }

static Ptr<PtrExample> g_ptr = nullptr;

static Ptr<PtrExample> StorePtr(Ptr<PtrExample> p) {
  Ptr<PtrExample> prev = g_ptr;
  g_ptr = p;
  return prev;
}

static void ClearPtr() { g_ptr = nullptr; }

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  {
    Ptr<PtrExample> p = CreateObject<PtrExample>();
    p->Method();
    Ptr<PtrExample> prev = StorePtr(p);
    NS_ASSERT(!prev);
  }

  {
    Ptr<PtrExample> p = CreateObject<PtrExample>();
    Ptr<PtrExample> prev = StorePtr(p);
    prev->Method();
    ClearPtr();
    PtrExample *raw = GetPointer(prev);
    prev = nullptr;
    raw->Method();
    raw->Unref();
  }

  return 0;
}
