
#include "ns3/test.h"
#include "ns3/wifi-phy-operating-channel.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiRuAllocationTest");

class Wifi20MHzIndicesCoveringRuTest : public TestCase {
public:
  Wifi20MHzIndicesCoveringRuTest();
  ~Wifi20MHzIndicesCoveringRuTest() override = default;

  void RunOne(uint8_t primary20, HeRu::RuSpec ru, uint16_t width,
              const std::set<uint8_t> &indices);

private:
  void DoRun() override;

  WifiPhyOperatingChannel m_channel;
};

Wifi20MHzIndicesCoveringRuTest::Wifi20MHzIndicesCoveringRuTest()
    : TestCase("Check computation of the indices of the 20 MHz channels "
               "covering an RU") {}

void Wifi20MHzIndicesCoveringRuTest::RunOne(uint8_t primary20, HeRu::RuSpec ru,
                                            uint16_t width,
                                            const std::set<uint8_t> &indices) {
  auto printToStr = [](const std::set<uint8_t> &s) {
    std::stringstream ss;
    ss << "{";
    for (const auto &index : s) {
      ss << +index << " ";
    }
    ss << "}";
    return ss.str();
  };

  m_channel.SetPrimary20Index(primary20);

  auto actualIndices = m_channel.Get20MHzIndicesCoveringRu(ru, width);
  NS_TEST_ASSERT_MSG_EQ(
      (actualIndices == indices), true,
      "Channel width=" << m_channel.GetWidth() << " MHz, PPDU width=" << width
                       << " MHz, p20Index=" << +primary20 << " , RU=" << ru
                       << ". Expected indices " << printToStr(indices)
                       << " differs from actual " << printToStr(actualIndices));
}

void Wifi20MHzIndicesCoveringRuTest::DoRun() {
  m_channel.SetDefault(20, WIFI_STANDARD_80211ax, WIFI_PHY_BAND_5GHZ);

  {
    const uint16_t width = 20;
    const uint8_t p20Index = 0;

    for (std::size_t idx = 1; idx <= 9; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {p20Index});
    }
    for (std::size_t idx = 1; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {p20Index});
    }
    for (std::size_t idx = 1; idx <= 2; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {p20Index});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 1, true), width,
           {p20Index});
  }

  m_channel.SetDefault(40, WIFI_STANDARD_80211ax, WIFI_PHY_BAND_5GHZ);

  for (uint8_t p20Index = 0; p20Index < 2; p20Index++) {
    const uint16_t width = 20;

    for (std::size_t idx = 1; idx <= 9; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {p20Index});
    }
    for (std::size_t idx = 1; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {p20Index});
    }
    for (std::size_t idx = 1; idx <= 2; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {p20Index});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 1, true), width,
           {p20Index});
  }

  for (uint8_t p20Index = 0; p20Index < 2; p20Index++) {
    const uint16_t width = 40;

    for (std::size_t idx = 1; idx <= 9; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width, {0});
    }
    for (std::size_t idx = 10; idx <= 18; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width, {1});
    }
    for (std::size_t idx = 1; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width, {0});
    }
    for (std::size_t idx = 5; idx <= 8; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width, {1});
    }
    for (std::size_t idx = 1; idx <= 2; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width, {0});
    }
    for (std::size_t idx = 3; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width, {1});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 1, true), width, {0});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 2, true), width, {1});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_484_TONE, 1, true), width, {0, 1});
  }

  m_channel.SetDefault(80, WIFI_STANDARD_80211ax, WIFI_PHY_BAND_5GHZ);

  for (uint8_t p20Index = 0; p20Index < 4; p20Index++) {
    const uint16_t width = 20;

    for (std::size_t idx = 1; idx <= 9; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {p20Index});
    }
    for (std::size_t idx = 1; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {p20Index});
    }
    for (std::size_t idx = 1; idx <= 2; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {p20Index});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 1, true), width,
           {p20Index});
  }

  for (uint8_t p20Index = 0; p20Index < 4; p20Index++) {
    const uint16_t width = 40;
    const uint8_t p40Index = p20Index / 2;
    const uint8_t ch20Index0 = p40Index * 2;
    const uint8_t ch20Index1 = p40Index * 2 + 1;

    for (std::size_t idx = 1; idx <= 9; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {ch20Index0});
    }
    for (std::size_t idx = 10; idx <= 18; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {ch20Index1});
    }
    for (std::size_t idx = 1; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {ch20Index0});
    }
    for (std::size_t idx = 5; idx <= 8; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {ch20Index1});
    }
    for (std::size_t idx = 1; idx <= 2; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {ch20Index0});
    }
    for (std::size_t idx = 3; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {ch20Index1});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 1, true), width,
           {ch20Index0});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 2, true), width,
           {ch20Index1});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_484_TONE, 1, true), width,
           {ch20Index0, ch20Index1});
  }

  for (uint8_t p20Index = 0; p20Index < 4; p20Index++) {
    const uint16_t width = 80;

    for (std::size_t idx = 1; idx <= 9; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width, {0});
    }
    for (std::size_t idx = 10; idx <= 18; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width, {1});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, 19, true), width, {1, 2});
    for (std::size_t idx = 20; idx <= 28; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width, {2});
    }
    for (std::size_t idx = 29; idx <= 37; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width, {3});
    }
    for (std::size_t idx = 1; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width, {0});
    }
    for (std::size_t idx = 5; idx <= 8; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width, {1});
    }
    for (std::size_t idx = 9; idx <= 12; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width, {2});
    }
    for (std::size_t idx = 13; idx <= 16; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width, {3});
    }
    for (std::size_t idx = 1; idx <= 2; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width, {0});
    }
    for (std::size_t idx = 3; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width, {1});
    }
    for (std::size_t idx = 5; idx <= 6; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width, {2});
    }
    for (std::size_t idx = 7; idx <= 8; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width, {3});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 1, true), width, {0});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 2, true), width, {1});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 3, true), width, {2});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 4, true), width, {3});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_484_TONE, 1, true), width, {0, 1});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_484_TONE, 2, true), width, {2, 3});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_996_TONE, 1, true), width,
           {0, 1, 2, 3});
  }

  m_channel.SetDefault(160, WIFI_STANDARD_80211ax, WIFI_PHY_BAND_5GHZ);

  for (uint8_t p20Index = 0; p20Index < 8; p20Index++) {
    const uint16_t width = 20;

    for (std::size_t idx = 1; idx <= 9; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {p20Index});
    }
    for (std::size_t idx = 1; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {p20Index});
    }
    for (std::size_t idx = 1; idx <= 2; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {p20Index});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 1, true), width,
           {p20Index});
  }

  for (uint8_t p20Index = 0; p20Index < 8; p20Index++) {
    const uint16_t width = 40;
    const uint8_t p40Index = p20Index / 2;
    const uint8_t ch20Index0 = p40Index * 2;
    const uint8_t ch20Index1 = p40Index * 2 + 1;

    for (std::size_t idx = 1; idx <= 9; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {ch20Index0});
    }
    for (std::size_t idx = 10; idx <= 18; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {ch20Index1});
    }
    for (std::size_t idx = 1; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {ch20Index0});
    }
    for (std::size_t idx = 5; idx <= 8; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {ch20Index1});
    }
    for (std::size_t idx = 1; idx <= 2; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {ch20Index0});
    }
    for (std::size_t idx = 3; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {ch20Index1});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 1, true), width,
           {ch20Index0});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 2, true), width,
           {ch20Index1});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_484_TONE, 1, true), width,
           {ch20Index0, ch20Index1});
  }

  for (uint8_t p20Index = 0; p20Index < 8; p20Index++) {
    const uint16_t width = 80;
    const uint8_t p80Index = p20Index / 4;
    const uint8_t ch20Index0 = p80Index * 4;
    const uint8_t ch20Index1 = p80Index * 4 + 1;
    const uint8_t ch20Index2 = p80Index * 4 + 2;
    const uint8_t ch20Index3 = p80Index * 4 + 3;

    for (std::size_t idx = 1; idx <= 9; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {ch20Index0});
    }
    for (std::size_t idx = 10; idx <= 18; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {ch20Index1});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, 19, true), width,
           {ch20Index1, ch20Index2});
    for (std::size_t idx = 20; idx <= 28; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {ch20Index2});
    }
    for (std::size_t idx = 29; idx <= 37; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, true), width,
             {ch20Index3});
    }
    for (std::size_t idx = 1; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {ch20Index0});
    }
    for (std::size_t idx = 5; idx <= 8; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {ch20Index1});
    }
    for (std::size_t idx = 9; idx <= 12; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {ch20Index2});
    }
    for (std::size_t idx = 13; idx <= 16; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, true), width,
             {ch20Index3});
    }
    for (std::size_t idx = 1; idx <= 2; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {ch20Index0});
    }
    for (std::size_t idx = 3; idx <= 4; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {ch20Index1});
    }
    for (std::size_t idx = 5; idx <= 6; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {ch20Index2});
    }
    for (std::size_t idx = 7; idx <= 8; idx++) {
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, true), width,
             {ch20Index3});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 1, true), width,
           {ch20Index0});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 2, true), width,
           {ch20Index1});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 3, true), width,
           {ch20Index2});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 4, true), width,
           {ch20Index3});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_484_TONE, 1, true), width,
           {ch20Index0, ch20Index1});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_484_TONE, 2, true), width,
           {ch20Index2, ch20Index3});
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_996_TONE, 1, true), width,
           {ch20Index0, ch20Index1, ch20Index2, ch20Index3});
  }

  for (uint8_t p20Index = 0; p20Index < 8; p20Index++) {
    const uint16_t width = 160;

    for (auto primary80MHz : {true, false}) {
      const uint8_t p80Index = (primary80MHz == (p20Index < 4)) ? 0 : 1;
      const uint8_t ch20Index0 = p80Index * 4;
      const uint8_t ch20Index1 = p80Index * 4 + 1;
      const uint8_t ch20Index2 = p80Index * 4 + 2;
      const uint8_t ch20Index3 = p80Index * 4 + 3;

      for (std::size_t idx = 1; idx <= 9; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, primary80MHz),
               width, {ch20Index0});
      }
      for (std::size_t idx = 10; idx <= 18; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, primary80MHz),
               width, {ch20Index1});
      }
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, 19, primary80MHz), width,
             {ch20Index1, ch20Index2});
      for (std::size_t idx = 20; idx <= 28; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, primary80MHz),
               width, {ch20Index2});
      }
      for (std::size_t idx = 29; idx <= 37; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_26_TONE, idx, primary80MHz),
               width, {ch20Index3});
      }
      for (std::size_t idx = 1; idx <= 4; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, primary80MHz),
               width, {ch20Index0});
      }
      for (std::size_t idx = 5; idx <= 8; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, primary80MHz),
               width, {ch20Index1});
      }
      for (std::size_t idx = 9; idx <= 12; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, primary80MHz),
               width, {ch20Index2});
      }
      for (std::size_t idx = 13; idx <= 16; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_52_TONE, idx, primary80MHz),
               width, {ch20Index3});
      }
      for (std::size_t idx = 1; idx <= 2; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, primary80MHz),
               width, {ch20Index0});
      }
      for (std::size_t idx = 3; idx <= 4; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, primary80MHz),
               width, {ch20Index1});
      }
      for (std::size_t idx = 5; idx <= 6; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, primary80MHz),
               width, {ch20Index2});
      }
      for (std::size_t idx = 7; idx <= 8; idx++) {
        RunOne(p20Index, HeRu::RuSpec(HeRu::RU_106_TONE, idx, primary80MHz),
               width, {ch20Index3});
      }
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 1, primary80MHz), width,
             {ch20Index0});
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 2, primary80MHz), width,
             {ch20Index1});
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 3, primary80MHz), width,
             {ch20Index2});
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_242_TONE, 4, primary80MHz), width,
             {ch20Index3});
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_484_TONE, 1, primary80MHz), width,
             {ch20Index0, ch20Index1});
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_484_TONE, 2, primary80MHz), width,
             {ch20Index2, ch20Index3});
      RunOne(p20Index, HeRu::RuSpec(HeRu::RU_996_TONE, 1, primary80MHz), width,
             {ch20Index0, ch20Index1, ch20Index2, ch20Index3});
    }
    RunOne(p20Index, HeRu::RuSpec(HeRu::RU_2x996_TONE, 1, true), width,
           {0, 1, 2, 3, 4, 5, 6, 7});
  }
}

class WifiRuAllocationTestSuite : public TestSuite {
public:
  WifiRuAllocationTestSuite();
};

WifiRuAllocationTestSuite::WifiRuAllocationTestSuite()
    : TestSuite("wifi-ru-allocation", UNIT) {
  AddTestCase(new Wifi20MHzIndicesCoveringRuTest(), TestCase::QUICK);
}

static WifiRuAllocationTestSuite g_wifiRuAllocationTestSuite;
