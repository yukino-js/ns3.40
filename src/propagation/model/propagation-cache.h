#ifndef PROPAGATION_CACHE_H_
#define PROPAGATION_CACHE_H_

#include "ns3/mobility-model.h"

#include <map>

namespace ns3 {
template <class T> class PropagationCache {
public:
  PropagationCache() {};
  ~PropagationCache() {};

  Ptr<T> GetPathData(Ptr<const MobilityModel> a, Ptr<const MobilityModel> b,
                     uint32_t modelUid) {
    PropagationPathIdentifier key = PropagationPathIdentifier(a, b, modelUid);
    auto it = m_pathCache.find(key);
    if (it == m_pathCache.end()) {
      return nullptr;
    }
    return it->second;
  };

  void AddPathData(Ptr<T> data, Ptr<const MobilityModel> a,
                   Ptr<const MobilityModel> b, uint32_t modelUid) {
    PropagationPathIdentifier key = PropagationPathIdentifier(a, b, modelUid);
    NS_ASSERT(m_pathCache.find(key) == m_pathCache.end());
    m_pathCache.insert(std::make_pair(key, data));
  };

  void Cleanup() {
    for (auto i : m_pathCache) {
      i.second->Dispose();
    }
    m_pathCache.clear();
  }

private:
  struct PropagationPathIdentifier {
    PropagationPathIdentifier(Ptr<const MobilityModel> a,
                              Ptr<const MobilityModel> b, uint32_t modelUid)
        : m_srcMobility(a), m_dstMobility(b), m_spectrumModelUid(modelUid) {};
    Ptr<const MobilityModel> m_srcMobility;
    Ptr<const MobilityModel> m_dstMobility;
    uint32_t m_spectrumModelUid;

    bool operator<(const PropagationPathIdentifier &other) const {
      if (m_spectrumModelUid != other.m_spectrumModelUid) {
        return m_spectrumModelUid < other.m_spectrumModelUid;
      }
      if (std::min(m_dstMobility, m_srcMobility) !=
          std::min(other.m_dstMobility, other.m_srcMobility)) {
        return std::min(m_dstMobility, m_srcMobility) <
               std::min(other.m_dstMobility, other.m_srcMobility);
      }
      if (std::max(m_dstMobility, m_srcMobility) !=
          std::max(other.m_dstMobility, other.m_srcMobility)) {
        return std::max(m_dstMobility, m_srcMobility) <
               std::max(other.m_dstMobility, other.m_srcMobility);
      }
      return false;
    }
  };

  typedef std::map<PropagationPathIdentifier, Ptr<T>> PathCache;

private:
  PathCache m_pathCache;
};
} // namespace ns3

#endif
