
#include "epc-x2-sap.h"

namespace ns3 {

EpcX2Sap::~EpcX2Sap() {}

EpcX2Sap::ErabToBeSetupItem::ErabToBeSetupItem()
    : erabLevelQosParameters(EpsBearer(EpsBearer::GBR_CONV_VOICE)) {}

EpcX2SapProvider::~EpcX2SapProvider() {}

EpcX2SapUser::~EpcX2SapUser() {}

} // namespace ns3
