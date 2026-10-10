
#include "hash.h"

#include "log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Hash");

Hasher &GetStaticHash() {
  static Hasher g_hasher = Hasher();
  g_hasher.clear();
  return g_hasher;
}

Hasher::Hasher() {
  m_impl = Create<Hash::Function::Murmur3>();
  NS_ASSERT(m_impl);
}

Hasher::Hasher(Ptr<Hash::Implementation> hp) : m_impl(hp) { NS_ASSERT(m_impl); }

Hasher &Hasher::clear() {
  m_impl->clear();
  return *this;
}

} // namespace ns3
