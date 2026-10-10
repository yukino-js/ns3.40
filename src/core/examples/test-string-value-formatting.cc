
#include "ns3/core-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TestStringValueFormatting");

namespace {

class FormattingTestObject : public Object {
public:
  static TypeId GetTypeId();
  FormattingTestObject();
  Ptr<RandomVariableStream> GetTestVariable() const;

private:
  Ptr<RandomVariableStream> m_testVariable;
};

NS_OBJECT_ENSURE_REGISTERED(FormattingTestObject);

TypeId FormattingTestObject::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::FormattingTestObject")
          .SetParent<Object>()
          .AddConstructor<FormattingTestObject>()
          .AddAttribute(
              "OnTime",
              "A RandomVariableStream used to pick the duration of the 'On' "
              "state.",
              StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
              MakePointerAccessor(&FormattingTestObject::m_testVariable),
              MakePointerChecker<RandomVariableStream>());
  return tid;
}

FormattingTestObject::FormattingTestObject() {}

Ptr<RandomVariableStream> FormattingTestObject::GetTestVariable() const {
  return m_testVariable;
}

class FormattingTestObjectHelper {
public:
  FormattingTestObjectHelper();
  void SetAttribute(std::string name, const AttributeValue &value);
  Ptr<Object> CreateFromFactory();

private:
  ObjectFactory m_factory;
};

FormattingTestObjectHelper::FormattingTestObjectHelper() {
  m_factory.SetTypeId(FormattingTestObject::GetTypeId());
}

void FormattingTestObjectHelper::SetAttribute(std::string name,
                                              const AttributeValue &value) {
  m_factory.Set(name, value);
}

Ptr<Object> FormattingTestObjectHelper::CreateFromFactory() {
  return m_factory.Create();
}

} // namespace

int main(int argc, char *argv[]) {
  Ptr<FormattingTestObject> obj = CreateObject<FormattingTestObject>();
  obj->SetAttribute("OnTime", StringValue("ns3::UniformRandomVariable"));
  obj->SetAttribute("OnTime",
                    StringValue("ns3::UniformRandomVariable[Min=0.]"));
  obj->SetAttribute("OnTime",
                    StringValue("ns3::UniformRandomVariable[Min=0.|Max=1.]"));
  obj->SetAttribute(
      "OnTime", StringValue("ns3::UniformRandomVariable[Min=50.|Max=100.]"));

  Ptr<RandomVariableStream> rvStream = obj->GetTestVariable();
  Ptr<UniformRandomVariable> uniformStream =
      rvStream->GetObject<UniformRandomVariable>();
  NS_ASSERT(uniformStream);

  DoubleValue val;
  uniformStream->GetAttribute("Min", val);
  NS_ASSERT_MSG(val.Get() == 50, "Minimum not set to 50");
  uniformStream->GetAttribute("Max", val);
  NS_ASSERT_MSG(val.Get() == 100, "Maximum not set to 100");

  FormattingTestObjectHelper formattingHelper;
  formattingHelper.SetAttribute(
      "OnTime", StringValue("ns3::UniformRandomVariable[Min=30.|Max=60.0]"));

  Ptr<Object> outputObj = formattingHelper.CreateFromFactory();
  Ptr<FormattingTestObject> fto = DynamicCast<FormattingTestObject>(outputObj);
  NS_ASSERT_MSG(fto, "object creation failed");
  rvStream = fto->GetTestVariable();
  uniformStream = rvStream->GetObject<UniformRandomVariable>();
  NS_ASSERT(uniformStream);
  uniformStream->GetAttribute("Min", val);
  NS_ASSERT_MSG(val.Get() == 30, "Minimum not set to 30");
  uniformStream->GetAttribute("Max", val);
  NS_ASSERT_MSG(val.Get() == 60, "Maximum not set to 60");

  return 0;
}
