#ifndef TIME_PRINTER_H
#define TIME_PRINTER_H

#include <ostream>

namespace ns3 {

typedef void (*TimePrinter)(std::ostream &os);

void DefaultTimePrinter(std::ostream &os);

} // namespace ns3

#endif
