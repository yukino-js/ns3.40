
#ifndef FD_READER_H
#define FD_READER_H

#include "callback.h"
#include "event-id.h"

#include <cstdint>
#include <thread>

#ifdef __WIN32__
#include <BaseTsd.h>

typedef SSIZE_T ssize_t;
#endif

namespace ns3 {

class FdReader : public SimpleRefCount<FdReader> {
public:
  FdReader();
  virtual ~FdReader();

  void Start(int fd, Callback<void, uint8_t *, ssize_t> readCallback);

  void Stop();

#ifdef __WIN32__
  static bool winsock_initialized;
#endif
protected:
  struct Data {
    Data() : m_buf(nullptr), m_len(0) {}

    Data(uint8_t *buf, ssize_t len) : m_buf(buf), m_len(len) {}

    uint8_t *m_buf;
    ssize_t m_len;
  };

  virtual FdReader::Data DoRead() = 0;

  int m_fd;

private:
  void Run();
  void DestroyEvent();

  Callback<void, uint8_t *, ssize_t> m_readCallback;

  std::thread m_readThread;

  int m_evpipe[2];
  bool m_stop;

  EventId m_destroyEvent;
};

} // namespace ns3

#endif
