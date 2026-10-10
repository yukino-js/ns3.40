#include "fatal-impl.h"

#include "log.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>

#ifdef __WIN32__
struct sigaction {
  void (*sa_handler)(int);
  int sa_flags;
  int sa_mask;
};

int sigaction(int sig, struct sigaction *action, struct sigaction *old) {
  if (sig == -1) {
    return 0;
  }
  if (old == nullptr) {
    if (signal(sig, SIG_DFL) == SIG_ERR) {
      return -1;
    }
  } else {
    if (signal(sig, action->sa_handler) == SIG_ERR) {
      return -1;
    }
  }
  return 0;
}
#endif

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("FatalImpl");

namespace FatalImpl {

namespace {

std::list<std::ostream *> **PeekStreamList() {
  NS_LOG_FUNCTION_NOARGS();
  static std::list<std::ostream *> *streams = nullptr;
  return &streams;
}

std::list<std::ostream *> *GetStreamList() {
  NS_LOG_FUNCTION_NOARGS();
  std::list<std::ostream *> **pstreams = PeekStreamList();
  if (*pstreams == nullptr) {
    *pstreams = new std::list<std::ostream *>();
  }
  return *pstreams;
}

} // namespace

void RegisterStream(std::ostream *stream) {
  NS_LOG_FUNCTION(stream);
  GetStreamList()->push_back(stream);
}

void UnregisterStream(std::ostream *stream) {
  NS_LOG_FUNCTION(stream);
  std::list<std::ostream *> **pl = PeekStreamList();
  if (*pl == nullptr) {
    return;
  }
  (*pl)->remove(stream);
  if ((*pl)->empty()) {
    delete *pl;
    *pl = nullptr;
  }
}

namespace {

void sigHandler(int sig) {
  NS_LOG_FUNCTION(sig);
  FlushStreams();
  std::abort();
}
} // namespace

void FlushStreams() {
  NS_LOG_FUNCTION_NOARGS();
  std::list<std::ostream *> **pl = PeekStreamList();
  if (*pl == nullptr) {
    return;
  }

  struct sigaction hdl;
  hdl.sa_handler = sigHandler;
  sigaction(SIGSEGV, &hdl, nullptr);

  std::list<std::ostream *> *l = *pl;

  while (!l->empty()) {
    std::ostream *s(l->front());
    l->pop_front();
    s->flush();
  }

  hdl.sa_handler = SIG_DFL;
  sigaction(SIGSEGV, &hdl, nullptr);

  std::fflush(nullptr);

  std::cout.flush();
  std::cerr.flush();
  std::clog.flush();

  delete l;
  *pl = nullptr;
}

} // namespace FatalImpl

} // namespace ns3
