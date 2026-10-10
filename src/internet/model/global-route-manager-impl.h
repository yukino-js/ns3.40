
#ifndef GLOBAL_ROUTE_MANAGER_IMPL_H
#define GLOBAL_ROUTE_MANAGER_IMPL_H

#include "global-router-interface.h"

#include "ns3/ipv4-address.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <list>
#include <map>
#include <queue>
#include <stdint.h>
#include <vector>

namespace ns3 {

const uint32_t SPF_INFINITY = 0xffffffff;

class CandidateQueue;
class Ipv4GlobalRouting;

class SPFVertex {
public:
  enum VertexType { VertexUnknown = 0, VertexRouter, VertexNetwork };

  SPFVertex();

  SPFVertex(GlobalRoutingLSA *lsa);

  ~SPFVertex();

  SPFVertex(const SPFVertex &) = delete;
  SPFVertex &operator=(const SPFVertex &) = delete;

  VertexType GetVertexType() const;

  void SetVertexType(VertexType type);

  Ipv4Address GetVertexId() const;

  void SetVertexId(Ipv4Address id);

  GlobalRoutingLSA *GetLSA() const;

  void SetLSA(GlobalRoutingLSA *lsa);

  uint32_t GetDistanceFromRoot() const;

  void SetDistanceFromRoot(uint32_t distance);

  void SetRootExitDirection(Ipv4Address nextHop, int32_t id = SPF_INFINITY);

  typedef std::pair<Ipv4Address, int32_t> NodeExit_t;

  void SetRootExitDirection(SPFVertex::NodeExit_t exit);
  NodeExit_t GetRootExitDirection(uint32_t i) const;
  NodeExit_t GetRootExitDirection() const;
  void MergeRootExitDirections(const SPFVertex *vertex);
  void InheritAllRootExitDirections(const SPFVertex *vertex);
  uint32_t GetNRootExitDirections() const;

  SPFVertex *GetParent(uint32_t i = 0) const;

  void SetParent(SPFVertex *parent);
  void MergeParent(const SPFVertex *v);

  uint32_t GetNChildren() const;

  SPFVertex *GetChild(uint32_t n) const;

  uint32_t AddChild(SPFVertex *child);

  void SetVertexProcessed(bool value);

  bool IsVertexProcessed() const;

  void ClearVertexProcessed();

private:
  VertexType m_vertexType;
  Ipv4Address m_vertexId;
  GlobalRoutingLSA *m_lsa;
  uint32_t m_distanceFromRoot;
  int32_t m_rootOif;
  Ipv4Address m_nextHop;
  typedef std::list<NodeExit_t> ListOfNodeExit_t;
  ListOfNodeExit_t m_ecmpRootExits;
  typedef std::list<SPFVertex *> ListOfSPFVertex_t;
  ListOfSPFVertex_t m_parents;
  ListOfSPFVertex_t m_children;
  bool m_vertexProcessed;

  friend std::ostream &operator<<(std::ostream &os,
                                  const SPFVertex::ListOfSPFVertex_t &vs);
};

class GlobalRouteManagerLSDB {
public:
  GlobalRouteManagerLSDB();

  ~GlobalRouteManagerLSDB();

  GlobalRouteManagerLSDB(const GlobalRouteManagerLSDB &) = delete;
  GlobalRouteManagerLSDB &operator=(const GlobalRouteManagerLSDB &) = delete;

  void Insert(Ipv4Address addr, GlobalRoutingLSA *lsa);

  GlobalRoutingLSA *GetLSA(Ipv4Address addr) const;
  GlobalRoutingLSA *GetLSAByLinkData(Ipv4Address addr) const;

  void Initialize();

  GlobalRoutingLSA *GetExtLSA(uint32_t index) const;
  uint32_t GetNumExtLSAs() const;

private:
  typedef std::map<Ipv4Address, GlobalRoutingLSA *> LSDBMap_t;
  typedef std::pair<Ipv4Address, GlobalRoutingLSA *> LSDBPair_t;

  LSDBMap_t m_database;
  std::vector<GlobalRoutingLSA *> m_extdatabase;
};

class GlobalRouteManagerImpl {
public:
  GlobalRouteManagerImpl();
  virtual ~GlobalRouteManagerImpl();

  GlobalRouteManagerImpl(const GlobalRouteManagerImpl &) = delete;
  GlobalRouteManagerImpl &operator=(const GlobalRouteManagerImpl &) = delete;

  virtual void DeleteGlobalRoutes();

  virtual void BuildGlobalRoutingDatabase();

  virtual void InitializeRoutes();

  void DebugUseLsdb(GlobalRouteManagerLSDB *lsdb);

  void DebugSPFCalculate(Ipv4Address root);

private:
  SPFVertex *m_spfroot;
  GlobalRouteManagerLSDB *m_lsdb;

  bool CheckForStubNode(Ipv4Address root);

  void SPFCalculate(Ipv4Address root);

  void SPFProcessStubs(SPFVertex *v);

  void ProcessASExternals(SPFVertex *v, GlobalRoutingLSA *extlsa);

  void SPFNext(SPFVertex *v, CandidateQueue &candidate);

  int SPFNexthopCalculation(SPFVertex *v, SPFVertex *w,
                            GlobalRoutingLinkRecord *l, uint32_t distance);

  void SPFVertexAddParent(SPFVertex *v);

  GlobalRoutingLinkRecord *SPFGetNextLink(SPFVertex *v, SPFVertex *w,
                                          GlobalRoutingLinkRecord *prev_link);

  void SPFIntraAddRouter(SPFVertex *v);

  void SPFIntraAddTransit(SPFVertex *v);

  void SPFIntraAddStub(GlobalRoutingLinkRecord *l, SPFVertex *v);

  void SPFAddASExternal(GlobalRoutingLSA *extlsa, SPFVertex *v);

  int32_t FindOutgoingInterfaceId(Ipv4Address a,
                                  Ipv4Mask amask = Ipv4Mask("255.255.255.255"));
};

} // namespace ns3

#endif
