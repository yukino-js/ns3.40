
#ifndef BLOCK_ACK_WINDOW_H
#define BLOCK_ACK_WINDOW_H

#include <cstdint>
#include <vector>

namespace ns3 {

class BlockAckWindow {
public:
  BlockAckWindow();
  void Init(uint16_t winStart, uint16_t winSize);
  void Reset(uint16_t winStart);
  uint16_t GetWinStart() const;
  uint16_t GetWinEnd() const;
  std::size_t GetWinSize() const;
  std::vector<bool>::reference At(std::size_t distance);
  std::vector<bool>::const_reference At(std::size_t distance) const;
  void Advance(std::size_t count);

private:
  uint16_t m_winStart;
  std::vector<bool> m_window;
  std::size_t m_head;
};

} // namespace ns3

#endif
