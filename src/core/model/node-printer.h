#ifndef NODE_PRINTER_H
#define NODE_PRINTER_H

#include <ostream>

namespace ns3 {

typedef void (*NodePrinter)(std::ostream &os);

void DefaultNodePrinter(std::ostream &os);

} // namespace ns3

#endif
