
#include "ns3/core-module.h"

#include <iomanip>
#include <iostream>
#include <string>

using namespace ns3;

namespace {

std::string g_cbArg = "cbArg default";

bool SetCbArg(const std::string &val) {
  g_cbArg = val;
  return true;
}

} // namespace

#define DefaultFinal(label, defaultValue, finalValue)                          \
  std::left << std::setw(20) << label + std::string(":") << std::setw(20)      \
            << defaultValue << finalValue << "\n"

int main(int argc, char *argv[]) {
  int intArg = 1;
  bool boolArg = false;
  std::string strArg = "strArg default";

  const std::string attrClass = "ns3::RandomVariableStream";
  const std::string attrName = "Antithetic";
  const std::string attrPath = attrClass + "::" + attrName;

  constexpr int CHARBUF_SIZE = 10;
  char charbuf[CHARBUF_SIZE] = "charstar";

  int nonOpt1 = 1;
  int nonOpt2 = 1;

  const int intDef = intArg;
  const bool boolDef = boolArg;
  const std::string strDef = strArg;
  const std::string cbDef = g_cbArg;
  const TypeId tid = TypeId::LookupByName(attrClass);
  std::string attrDef;
  {
    TypeId::AttributeInformation info;
    tid.LookupAttributeByName(attrName, &info);
    attrDef = info.originalInitialValue->SerializeToString(info.checker);
  }
  const std::string charbufDef{charbuf};
  const int nonOpt1Def = nonOpt1;
  const int nonOpt2Def = nonOpt2;

  CommandLine cmd(__FILE__);
  cmd.Usage("CommandLine example program.\n"
            "\n"
            "This little program demonstrates how to use CommandLine.");
  cmd.AddValue("intArg", "an int argument", intArg);
  cmd.AddValue("boolArg", "a bool argument", boolArg);
  cmd.AddValue("strArg", "a string argument", strArg);
  cmd.AddValue("anti", attrPath);
  cmd.AddValue("cbArg", "a string via callback", MakeCallback(SetCbArg));
  cmd.AddValue("charbuf", "a char* buffer", charbuf, CHARBUF_SIZE);
  cmd.AddNonOption("nonOpt1", "first non-option", nonOpt1);
  cmd.AddNonOption("nonOpt2", "second non-option", nonOpt2);
  cmd.Parse(argc, argv);

  std::cout << std::endl;
  std::cout << cmd.GetName() << ":" << std::endl;

  std::cout << "Program Version: ";
  cmd.PrintVersion(std::cout);
  std::cout << std::endl;

  std::cout << "Argument            Initial Value       Final Value\n"
            << std::left << std::boolalpha;

  std::cout << DefaultFinal("intArg", intDef, intArg)
            << DefaultFinal("boolArg", (boolDef ? "true" : "false"),
                            (boolArg ? "true" : "false"))
            << DefaultFinal("strArg", "\"" + strDef + "\"",
                            "\"" + strArg + "\"");

  std::string antiArg;
  {
    TypeId::AttributeInformation info;
    tid.LookupAttributeByName(attrName, &info);
    antiArg = info.initialValue->SerializeToString(info.checker);
  }

  std::cout << DefaultFinal("anti", "\"" + attrDef + "\"",
                            "\"" + antiArg + "\"")
            << DefaultFinal("cbArg", cbDef, g_cbArg)
            << DefaultFinal("charbuf", "\"" + charbufDef + "\"",
                            "\"" + std::string(charbuf) + "\"")
            << DefaultFinal("nonOpt1", nonOpt1Def, nonOpt1)
            << DefaultFinal("nonOpt2", nonOpt2Def, nonOpt2) << std::endl;

  std::cout << std::setw(40) << "Number of extra non-option arguments:"
            << cmd.GetNExtraNonOptions() << std::endl;

  for (std::size_t i = 0; i < cmd.GetNExtraNonOptions(); ++i) {
    std::cout << DefaultFinal("extra non-option " + std::to_string(i), "",
                              "\"" + cmd.GetExtraNonOption(i) + "\"");
  }
  std::cout << std::endl;

#undef DefaultFinal

  return 0;
}
