

#include "fatal-error.h"
#include "fd-reader.h"
#include "log.h"
#include "simple-ref-count.h"
#include "simulator.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/select.h>
#include <thread>
#include <unistd.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("FdReader");

FdReader::FdReader() : m_fd(-1), m_stop(false), m_destroyEvent() {
  NS_LOG_FUNCTION(this);
  m_evpipe[0] = -1;
  m_evpipe[1] = -1;
}

FdReader::~FdReader() {
  NS_LOG_FUNCTION(this);
  Stop();
}

void FdReader::Start(int fd, Callback<void, uint8_t *, ssize_t> readCallback) {
  NS_LOG_FUNCTION(this << fd << &readCallback);
  int tmp;

  NS_ASSERT_MSG(!m_readThread.joinable(), "read thread already exists");

  tmp = pipe(m_evpipe);
  if (tmp == -1) {
    NS_FATAL_ERROR("pipe() failed: " << std::strerror(errno));
  }

  tmp = fcntl(m_evpipe[0], F_GETFL);
  if (tmp == -1) {
    NS_FATAL_ERROR("fcntl() failed: " << std::strerror(errno));
  }
  if (fcntl(m_evpipe[0], F_SETFL, tmp | O_NONBLOCK) == -1) {
    NS_FATAL_ERROR("fcntl() failed: " << std::strerror(errno));
  }

  m_fd = fd;
  m_readCallback = readCallback;

  if (!m_destroyEvent.IsRunning()) {
    this->Ref();
    m_destroyEvent = Simulator::ScheduleDestroy(&FdReader::DestroyEvent, this);
  }

  NS_LOG_LOGIC("Spinning up read thread");

  m_readThread = std::thread(&FdReader::Run, this);
}

void FdReader::DestroyEvent() {
  NS_LOG_FUNCTION(this);
  Stop();
  this->Unref();
}

void FdReader::Stop() {
  NS_LOG_FUNCTION(this);
  m_stop = true;

  if (m_evpipe[1] != -1) {
    char zero = 0;
    ssize_t len = write(m_evpipe[1], &zero, sizeof(zero));
    if (len != sizeof(zero)) {
      NS_LOG_WARN("incomplete write(): " << std::strerror(errno));
    }
  }

  if (m_readThread.joinable()) {
    m_readThread.join();
  }

  if (m_evpipe[1] != -1) {
    close(m_evpipe[1]);
    m_evpipe[1] = -1;
  }

  if (m_evpipe[0] != -1) {
    close(m_evpipe[0]);
    m_evpipe[0] = -1;
  }

  m_fd = -1;
  m_readCallback.Nullify();
  m_stop = false;
}

void FdReader::Run() {
  NS_LOG_FUNCTION(this);
  int nfds;
  fd_set rfds;

  nfds = (m_fd > m_evpipe[0] ? m_fd : m_evpipe[0]) + 1;

  FD_ZERO(&rfds);
  FD_SET(m_fd, &rfds);
  FD_SET(m_evpipe[0], &rfds);

  for (;;) {
    int r;
    fd_set readfds = rfds;

    r = select(nfds, &readfds, nullptr, nullptr, nullptr);
    if (r == -1 && errno != EINTR) {
      NS_FATAL_ERROR("select() failed: " << std::strerror(errno));
    }

    if (FD_ISSET(m_evpipe[0], &readfds)) {
      for (;;) {
        char buf[1024];
        ssize_t len = read(m_evpipe[0], buf, sizeof(buf));
        if (len == 0) {
          NS_FATAL_ERROR("event pipe closed");
        }
        if (len < 0) {
          if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
            break;
          } else {
            NS_FATAL_ERROR("read() failed: " << std::strerror(errno));
          }
        }
      }
    }

    if (m_stop) {
      break;
    }

    if (FD_ISSET(m_fd, &readfds)) {
      FdReader::Data data = DoRead();
      if (data.m_len == 0) {
        break;
      } else if (data.m_len > 0) {
        m_readCallback(data.m_buf, data.m_len);
      }
    }
  }
}

} // namespace ns3
