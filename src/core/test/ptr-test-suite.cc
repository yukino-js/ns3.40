
#include "ns3/ptr.h"
#include "ns3/test.h"

namespace ns3 {

namespace tests {

class PtrTestCase;

class PtrTestBase {
public:
  PtrTestBase();
  virtual ~PtrTestBase();
  void Ref() const;
  void Unref() const;

private:
  mutable uint32_t m_count;
};

class NoCount : public PtrTestBase {
public:
  NoCount(PtrTestCase *test);
  ~NoCount() override;
  void Nothing() const;

private:
  PtrTestCase *m_test;
};

class PtrTestCase : public TestCase {
public:
  PtrTestCase();
  void DestroyNotify();

private:
  void DoRun() override;
  Ptr<NoCount> CallTest(Ptr<NoCount> p);
  const Ptr<NoCount> CallTestConst(const Ptr<NoCount> p);
  uint32_t m_nDestroyed;
};

PtrTestBase::PtrTestBase() : m_count(1) {}

PtrTestBase::~PtrTestBase() {}

void PtrTestBase::Ref() const { m_count++; }

void PtrTestBase::Unref() const {
  m_count--;
  if (m_count == 0) {
    delete this;
  }
}

NoCount::NoCount(PtrTestCase *test) : m_test(test) {}

NoCount::~NoCount() { m_test->DestroyNotify(); }

void NoCount::Nothing() const {}

PtrTestCase::PtrTestCase() : TestCase("Sanity checking of Ptr<>") {}

void PtrTestCase::DestroyNotify() { m_nDestroyed++; }

Ptr<NoCount> PtrTestCase::CallTest(Ptr<NoCount> p) { return p; }

const Ptr<NoCount> PtrTestCase::CallTestConst(const Ptr<NoCount> p) {
  return p;
}

void PtrTestCase::DoRun() {
  m_nDestroyed = 0;
  {
    Ptr<NoCount> p = Create<NoCount>(this);
  }
  NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "001");

  m_nDestroyed = 0;
  {
    Ptr<NoCount> p;
    p = Create<NoCount>(this);
#if defined(__clang__)
#if __has_warning("-Wself-assign-overloaded")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
#endif
    p = p;
#if defined(__clang__)
#if __has_warning("-Wself-assign-overloaded")
#pragma clang diagnostic pop
#endif
#endif
  }
  NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "002");

  m_nDestroyed = 0;
  {
    Ptr<NoCount> p1;
    p1 = Create<NoCount>(this);
    Ptr<NoCount> p2 = p1;
  }
  NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "003");

  m_nDestroyed = 0;
  {
    Ptr<NoCount> p1;
    p1 = Create<NoCount>(this);
    Ptr<NoCount> p2;
    p2 = p1;
  }
  NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "004");

  m_nDestroyed = 0;
  {
    Ptr<NoCount> p1;
    p1 = Create<NoCount>(this);
    Ptr<NoCount> p2 = Create<NoCount>(this);
    p2 = p1;
  }
  NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 2, "005");

  m_nDestroyed = 0;
  {
    Ptr<NoCount> p1;
    p1 = Create<NoCount>(this);
    Ptr<NoCount> p2;
    p2 = Create<NoCount>(this);
    p2 = p1;
  }
  NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 2, "006");

  m_nDestroyed = 0;
  {
    Ptr<NoCount> p1;
    p1 = Create<NoCount>(this);
    p1 = Create<NoCount>(this);
  }
  NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 2, "007");

  m_nDestroyed = 0;
  {
    Ptr<NoCount> p1;
    {
      Ptr<NoCount> p2;
      p1 = Create<NoCount>(this);
      p2 = Create<NoCount>(this);
      p2 = p1;
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "008");
  }
  NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 2, "009");

  m_nDestroyed = 0;
  {
    Ptr<NoCount> p1;
    {
      Ptr<NoCount> p2;
      p1 = Create<NoCount>(this);
      p2 = Create<NoCount>(this);
      p2 = CallTest(p1);
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "010");
  }
  NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 2, "011");

  {
    Ptr<NoCount> p1;
    const Ptr<NoCount> p2 = CallTest(p1);
    const Ptr<NoCount> p3 = CallTestConst(p1);
    Ptr<NoCount> p4 = CallTestConst(p1);
    Ptr<const NoCount> p5 = p4;
    p4 = ConstCast<NoCount>(p5);
    p5 = p1;
    Ptr<NoCount> p;
    if (!p) {
    }
    if (p) {
    }
    if (!p) {
    }
    if (p) {
    }
    if (p) {
    }
    if (!p) {
    }
  }

  m_nDestroyed = 0;
  {
    NoCount *raw;
    {
      Ptr<NoCount> p = Create<NoCount>(this);
      {
        Ptr<const NoCount> p1 = p;
      }
      raw = GetPointer(p);
      p = nullptr;
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 0, "012");
    delete raw;
  }

  m_nDestroyed = 0;
  {
    Ptr<NoCount> p = Create<NoCount>(this);
    const NoCount *v1 = PeekPointer(p);
    NoCount *v2 = PeekPointer(p);
    v1->Nothing();
    v2->Nothing();
  }
  NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "013");

  {
    Ptr<PtrTestBase> p0 = Create<NoCount>(this);
    Ptr<NoCount> p1 = Create<NoCount>(this);
    NS_TEST_EXPECT_MSG_EQ((p0 == p1), false, "operator == failed");
    NS_TEST_EXPECT_MSG_EQ((p0 != p1), true, "operator != failed");
  }
}

class PtrTestSuite : public TestSuite {
public:
  PtrTestSuite() : TestSuite("ptr") { AddTestCase(new PtrTestCase()); }
};

static PtrTestSuite g_ptrTestSuite;

} // namespace tests

} // namespace ns3
