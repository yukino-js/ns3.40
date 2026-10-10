

#include "attribute-iterator.h"

#include <gtk/gtk.h>

namespace ns3 {

enum { COL_NODE = 0, COL_LAST };

struct ModelNode {
  enum {
    NODE_ATTRIBUTE,
    NODE_POINTER,
    NODE_VECTOR,
    NODE_VECTOR_ITEM,
    NODE_OBJECT
  } type;

  std::string name;
  Ptr<Object> object;
  uint32_t index;
};

class ModelCreator : public AttributeIterator {
public:
  ModelCreator();

  void Build(GtkTreeStore *treestore);

private:
  void DoVisitAttribute(Ptr<Object> object, std::string name) override;
  void DoStartVisitObject(Ptr<Object> object) override;
  void DoEndVisitObject() override;
  void DoStartVisitPointerAttribute(Ptr<Object> object, std::string name,
                                    Ptr<Object> value) override;
  void DoEndVisitPointerAttribute() override;
  void
  DoStartVisitArrayAttribute(Ptr<Object> object, std::string name,
                             const ObjectPtrContainerValue &vector) override;
  void DoEndVisitArrayAttribute() override;
  void DoStartVisitArrayItem(const ObjectPtrContainerValue &vector,
                             uint32_t index, Ptr<Object> item) override;
  void DoEndVisitArrayItem() override;
  void Add(ModelNode *node);
  void Remove();

  GtkTreeStore *m_treestore;
  std::vector<GtkTreeIter *> m_iters;
};
} // namespace ns3
