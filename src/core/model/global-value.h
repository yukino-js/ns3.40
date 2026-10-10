#ifndef GLOBAL_VALUE_H
#define GLOBAL_VALUE_H

#include "attribute.h"
#include "ptr.h"

#include <string>
#include <vector>

namespace ns3 {

namespace tests {
class GlobalValueTestCase;
}

class GlobalValue {
  typedef std::vector<GlobalValue *> Vector;

public:
  typedef Vector::const_iterator Iterator;

  GlobalValue(std::string name, std::string help,
              const AttributeValue &initialValue,
              Ptr<const AttributeChecker> checker);

  std::string GetName() const;
  std::string GetHelp() const;
  void GetValue(AttributeValue &value) const;
  Ptr<const AttributeChecker> GetChecker() const;
  bool SetValue(const AttributeValue &value);

  void ResetInitialValue();

  static void Bind(std::string name, const AttributeValue &value);

  static bool BindFailSafe(std::string name, const AttributeValue &value);

  static Iterator Begin();
  static Iterator End();

  static bool GetValueByNameFailSafe(std::string name, AttributeValue &value);

  static void GetValueByName(std::string name, AttributeValue &value);

private:
  friend class tests::GlobalValueTestCase;

  static Vector *GetVector();
  void InitializeFromEnv();

  std::string m_name;
  std::string m_help;
  Ptr<AttributeValue> m_initialValue;
  Ptr<AttributeValue> m_currentValue;
  Ptr<const AttributeChecker> m_checker;
};

} // namespace ns3

#endif
