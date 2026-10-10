

#include "attribute-default-iterator.h"

#include "ns3/type-id.h"

#include <gtk/gtk.h>
#include <vector>

namespace ns3 {

enum { COL_TYPEID = 0, COL_LASTID };

struct ModelTypeid {
  enum { NODE_ATTRIBUTE, NODE_TYPEID } type;

  std::string name;
  std::string defaultValue;
  TypeId tid;
  uint32_t index;
};

class ModelTypeidCreator : public AttributeDefaultIterator {
public:
  ModelTypeidCreator();
  void Build(GtkTreeStore *treestore);

private:
  void VisitAttribute(TypeId tid, std::string name, std::string defaultValue,
                      uint32_t index) override;
  void StartVisitTypeId(std::string name) override;
  void EndVisitTypeId() override;
  void Add(ModelTypeid *node);
  void Remove();
  GtkTreeStore *m_treestore;
  std::vector<GtkTreeIter *> m_iters;
};
} // namespace ns3
