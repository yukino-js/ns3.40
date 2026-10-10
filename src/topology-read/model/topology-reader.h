
#ifndef TOPOLOGY_READER_H
#define TOPOLOGY_READER_H

#include "ns3/node.h"
#include "ns3/object.h"

#include <list>
#include <map>
#include <string>

namespace ns3 {

class NodeContainer;

class TopologyReader : public Object {
public:
  class Link {
  public:
    typedef std::map<std::string, std::string>::const_iterator
        ConstAttributesIterator;

    Link(Ptr<Node> fromPtr, const std::string &fromName, Ptr<Node> toPtr,
         const std::string &toName);

    Ptr<Node> GetFromNode() const;
    std::string GetFromNodeName() const;
    Ptr<Node> GetToNode() const;
    std::string GetToNodeName() const;
    std::string GetAttribute(const std::string &name) const;
    bool GetAttributeFailSafe(const std::string &name,
                              std::string &value) const;
    void SetAttribute(const std::string &name, const std::string &value);
    ConstAttributesIterator AttributesBegin() const;
    ConstAttributesIterator AttributesEnd() const;

  private:
    Link();
    std::string m_fromName;
    Ptr<Node> m_fromPtr;
    std::string m_toName;
    Ptr<Node> m_toPtr;
    std::map<std::string, std::string> m_linkAttr;
  };

  typedef std::list<Link>::const_iterator ConstLinksIterator;

  static TypeId GetTypeId();

  TopologyReader();
  ~TopologyReader() override;

  TopologyReader(const TopologyReader &) = delete;
  TopologyReader &operator=(const TopologyReader &) = delete;

  virtual NodeContainer Read() = 0;

  void SetFileName(const std::string &fileName);

  std::string GetFileName() const;

  ConstLinksIterator LinksBegin() const;

  ConstLinksIterator LinksEnd() const;

  int LinksSize() const;

  bool LinksEmpty() const;

  void AddLink(Link link);

private:
  std::string m_fileName;

  std::list<Link> m_linksList;
};

}; // namespace ns3

#endif
