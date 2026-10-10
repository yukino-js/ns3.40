
#include "multi-model-spectrum-channel.h"

#include "spectrum-converter.h"
#include "spectrum-phy.h"
#include "spectrum-propagation-loss-model.h"
#include "spectrum-transmit-filter.h"

#include <ns3/angles.h>
#include <ns3/antenna-model.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/node.h>
#include <ns3/object.h>
#include <ns3/packet-burst.h>
#include <ns3/packet.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/simulator.h>

#include <algorithm>
#include <iostream>
#include <utility>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MultiModelSpectrumChannel");

NS_OBJECT_ENSURE_REGISTERED(MultiModelSpectrumChannel);

std::ostream &operator<<(std::ostream &lhs, TxSpectrumModelInfoMap_t &rhs) {
  for (auto it = rhs.begin(); it != rhs.end(); ++it) {
    for (auto jt = it->second.m_spectrumConverterMap.begin();
         jt != it->second.m_spectrumConverterMap.end(); ++jt) {
      lhs << "(" << it->first << "," << jt->first << ") ";
    }
  }
  return lhs;
}

TxSpectrumModelInfo::TxSpectrumModelInfo(
    Ptr<const SpectrumModel> txSpectrumModel)
    : m_txSpectrumModel(txSpectrumModel) {}

RxSpectrumModelInfo::RxSpectrumModelInfo(
    Ptr<const SpectrumModel> rxSpectrumModel)
    : m_rxSpectrumModel(rxSpectrumModel) {}

MultiModelSpectrumChannel::MultiModelSpectrumChannel() : m_numDevices{0} {
  NS_LOG_FUNCTION(this);
}

void MultiModelSpectrumChannel::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_txSpectrumModelInfoMap.clear();
  m_rxSpectrumModelInfoMap.clear();
  SpectrumChannel::DoDispose();
}

TypeId MultiModelSpectrumChannel::GetTypeId() {
  static TypeId tid = TypeId("ns3::MultiModelSpectrumChannel")
                          .SetParent<SpectrumChannel>()
                          .SetGroupName("Spectrum")
                          .AddConstructor<MultiModelSpectrumChannel>()

      ;
  return tid;
}

void MultiModelSpectrumChannel::RemoveRx(Ptr<SpectrumPhy> phy) {
  NS_LOG_FUNCTION(this << phy);

  for (auto rxInfoIterator = m_rxSpectrumModelInfoMap.begin();
       rxInfoIterator != m_rxSpectrumModelInfoMap.end(); ++rxInfoIterator) {
    auto phyIt = std::find(rxInfoIterator->second.m_rxPhys.begin(),
                           rxInfoIterator->second.m_rxPhys.end(), phy);
    if (phyIt != rxInfoIterator->second.m_rxPhys.end()) {
      rxInfoIterator->second.m_rxPhys.erase(phyIt);
      --m_numDevices;
      break;
    }
  }
}

void MultiModelSpectrumChannel::AddRx(Ptr<SpectrumPhy> phy) {
  NS_LOG_FUNCTION(this << phy);

  Ptr<const SpectrumModel> rxSpectrumModel = phy->GetRxSpectrumModel();

  NS_ASSERT_MSG(rxSpectrumModel, "phy->GetRxSpectrumModel () returned 0. "
                                 "Please check that the RxSpectrumModel is "
                                 "already set for the phy before calling "
                                 "MultiModelSpectrumChannel::AddRx (phy)");

  SpectrumModelUid_t rxSpectrumModelUid = rxSpectrumModel->GetUid();

  RemoveRx(phy);

  ++m_numDevices;

  auto [rxInfoIterator, inserted] = m_rxSpectrumModelInfoMap.emplace(
      rxSpectrumModelUid, RxSpectrumModelInfo(rxSpectrumModel));

  rxInfoIterator->second.m_rxPhys.push_back(phy);

  if (inserted) {
    for (auto txInfoIterator = m_txSpectrumModelInfoMap.begin();
         txInfoIterator != m_txSpectrumModelInfoMap.end(); ++txInfoIterator) {
      Ptr<const SpectrumModel> txSpectrumModel =
          txInfoIterator->second.m_txSpectrumModel;
      SpectrumModelUid_t txSpectrumModelUid = txSpectrumModel->GetUid();

      if (rxSpectrumModelUid != txSpectrumModelUid &&
          !txSpectrumModel->IsOrthogonal(*rxSpectrumModel)) {
        NS_LOG_LOGIC("Creating converter between SpectrumModelUid "
                     << txSpectrumModel->GetUid() << " and "
                     << rxSpectrumModelUid);
        SpectrumConverter converter(txSpectrumModel, rxSpectrumModel);
        auto ret2 = txInfoIterator->second.m_spectrumConverterMap.insert(
            std::make_pair(rxSpectrumModelUid, converter));
        NS_ASSERT(ret2.second);
      }
    }
  }
}

TxSpectrumModelInfoMap_t::const_iterator
MultiModelSpectrumChannel::FindAndEventuallyAddTxSpectrumModel(
    Ptr<const SpectrumModel> txSpectrumModel) {
  NS_LOG_FUNCTION(this << txSpectrumModel);
  SpectrumModelUid_t txSpectrumModelUid = txSpectrumModel->GetUid();
  auto txInfoIterator = m_txSpectrumModelInfoMap.find(txSpectrumModelUid);

  if (txInfoIterator == m_txSpectrumModelInfoMap.end()) {
    auto ret = m_txSpectrumModelInfoMap.insert(std::make_pair(
        txSpectrumModelUid, TxSpectrumModelInfo(txSpectrumModel)));
    NS_ASSERT(ret.second);
    txInfoIterator = ret.first;

    for (auto rxInfoIterator = m_rxSpectrumModelInfoMap.begin();
         rxInfoIterator != m_rxSpectrumModelInfoMap.end(); ++rxInfoIterator) {
      Ptr<const SpectrumModel> rxSpectrumModel =
          rxInfoIterator->second.m_rxSpectrumModel;
      SpectrumModelUid_t rxSpectrumModelUid = rxSpectrumModel->GetUid();

      if (rxSpectrumModelUid != txSpectrumModelUid &&
          !txSpectrumModel->IsOrthogonal(*rxSpectrumModel)) {
        NS_LOG_LOGIC("Creating converter between SpectrumModelUid "
                     << txSpectrumModelUid << " and " << rxSpectrumModelUid);

        SpectrumConverter converter(txSpectrumModel, rxSpectrumModel);
        auto ret2 = txInfoIterator->second.m_spectrumConverterMap.insert(
            std::make_pair(rxSpectrumModelUid, converter));
        NS_ASSERT(ret2.second);
      }
    }
  } else {
    NS_LOG_LOGIC("SpectrumModelUid " << txSpectrumModelUid
                                     << " already present");
  }
  return txInfoIterator;
}

void MultiModelSpectrumChannel::StartTx(
    Ptr<SpectrumSignalParameters> txParams) {
  NS_LOG_FUNCTION(this << txParams);

  NS_ASSERT(txParams->txPhy);
  NS_ASSERT(txParams->psd);
  Ptr<SpectrumSignalParameters> txParamsTrace = txParams->Copy();
  m_txSigParamsTrace(txParamsTrace);

  Ptr<MobilityModel> txMobility = txParams->txPhy->GetMobility();
  SpectrumModelUid_t txSpectrumModelUid = txParams->psd->GetSpectrumModelUid();
  NS_LOG_LOGIC("txSpectrumModelUid " << txSpectrumModelUid);

  auto txInfoIteratorerator =
      FindAndEventuallyAddTxSpectrumModel(txParams->psd->GetSpectrumModel());
  NS_ASSERT(txInfoIteratorerator != m_txSpectrumModelInfoMap.end());

  NS_LOG_LOGIC("converter map for TX SpectrumModel with Uid "
               << txInfoIteratorerator->first);
  NS_LOG_LOGIC("converter map size: "
               << txInfoIteratorerator->second.m_spectrumConverterMap.size());
  NS_LOG_LOGIC(
      "converter map first element: "
      << txInfoIteratorerator->second.m_spectrumConverterMap.begin()->first);

  for (auto rxInfoIterator = m_rxSpectrumModelInfoMap.begin();
       rxInfoIterator != m_rxSpectrumModelInfoMap.end(); ++rxInfoIterator) {
    SpectrumModelUid_t rxSpectrumModelUid =
        rxInfoIterator->second.m_rxSpectrumModel->GetUid();
    NS_LOG_LOGIC("rxSpectrumModelUids " << rxSpectrumModelUid);

    Ptr<SpectrumValue> convertedTxPowerSpectrum;
    if (txSpectrumModelUid == rxSpectrumModelUid) {
      NS_LOG_LOGIC("no spectrum conversion needed");
      convertedTxPowerSpectrum = txParams->psd;
    } else {
      NS_LOG_LOGIC("converting txPowerSpectrum SpectrumModelUids "
                   << txSpectrumModelUid << " --> " << rxSpectrumModelUid);
      auto rxConverterIterator =
          txInfoIteratorerator->second.m_spectrumConverterMap.find(
              rxSpectrumModelUid);
      if (rxConverterIterator ==
          txInfoIteratorerator->second.m_spectrumConverterMap.end()) {
        continue;
      }
      convertedTxPowerSpectrum =
          rxConverterIterator->second.Convert(txParams->psd);
    }

    for (auto rxPhyIterator = rxInfoIterator->second.m_rxPhys.begin();
         rxPhyIterator != rxInfoIterator->second.m_rxPhys.end();
         ++rxPhyIterator) {
      NS_ASSERT_MSG(
          (*rxPhyIterator)->GetRxSpectrumModel()->GetUid() ==
              rxSpectrumModelUid,
          "SpectrumModel change was not notified to MultiModelSpectrumChannel "
          "(i.e., AddRx should be called again after model is changed)");

      if ((*rxPhyIterator) != txParams->txPhy) {
        Ptr<NetDevice> rxNetDevice = (*rxPhyIterator)->GetDevice();
        Ptr<NetDevice> txNetDevice = txParams->txPhy->GetDevice();

        if (rxNetDevice && txNetDevice) {
          if (rxNetDevice->GetNode()->GetId() ==
              txNetDevice->GetNode()->GetId()) {
            NS_LOG_DEBUG(
                "Skipping the pathloss calculation among different antennas of "
                "the "
                "same node, not supported yet by any pathloss model in ns-3.");
            continue;
          }
        }

        if (m_filter && m_filter->Filter(txParams, *rxPhyIterator)) {
          continue;
        }

        NS_LOG_LOGIC("copying signal parameters " << txParams);
        Ptr<SpectrumSignalParameters> rxParams = txParams->Copy();
        rxParams->psd = Copy<SpectrumValue>(convertedTxPowerSpectrum);
        Time delay = MicroSeconds(0);

        Ptr<MobilityModel> receiverMobility = (*rxPhyIterator)->GetMobility();

        if (txMobility && receiverMobility) {
          double txAntennaGain = 0;
          double rxAntennaGain = 0;
          double propagationGainDb = 0;
          double pathLossDb = 0;
          if (rxParams->txAntenna) {
            Angles txAngles(receiverMobility->GetPosition(),
                            txMobility->GetPosition());
            txAntennaGain = rxParams->txAntenna->GetGainDb(txAngles);
            NS_LOG_LOGIC("txAntennaGain = " << txAntennaGain << " dB");
            pathLossDb -= txAntennaGain;
          }
          Ptr<AntennaModel> rxAntenna =
              DynamicCast<AntennaModel>((*rxPhyIterator)->GetAntenna());
          if (rxAntenna) {
            Angles rxAngles(txMobility->GetPosition(),
                            receiverMobility->GetPosition());
            rxAntennaGain = rxAntenna->GetGainDb(rxAngles);
            NS_LOG_LOGIC("rxAntennaGain = " << rxAntennaGain << " dB");
            pathLossDb -= rxAntennaGain;
          }
          if (m_propagationLoss) {
            propagationGainDb =
                m_propagationLoss->CalcRxPower(0, txMobility, receiverMobility);
            NS_LOG_LOGIC("propagationGainDb = " << propagationGainDb << " dB");
            pathLossDb -= propagationGainDb;
          }
          NS_LOG_LOGIC("total pathLoss = " << pathLossDb << " dB");
          m_gainTrace(txMobility, receiverMobility, txAntennaGain,
                      rxAntennaGain, propagationGainDb, pathLossDb);
          m_pathLossTrace(txParams->txPhy, *rxPhyIterator, pathLossDb);
          if (pathLossDb > m_maxLossDb) {
            continue;
          }
          double pathGainLinear = std::pow(10.0, (-pathLossDb) / 10.0);
          *(rxParams->psd) *= pathGainLinear;

          if (m_propagationDelay) {
            delay = m_propagationDelay->GetDelay(txMobility, receiverMobility);
          }
        }

        if (rxNetDevice) {
          uint32_t dstNode = rxNetDevice->GetNode()->GetId();
          Simulator::ScheduleWithContext(dstNode, delay,
                                         &MultiModelSpectrumChannel::StartRx,
                                         this, rxParams, *rxPhyIterator);
        } else {
          Simulator::Schedule(delay, &MultiModelSpectrumChannel::StartRx, this,
                              rxParams, *rxPhyIterator);
        }
      }
    }
  }
}

void MultiModelSpectrumChannel::StartRx(Ptr<SpectrumSignalParameters> params,
                                        Ptr<SpectrumPhy> receiver) {
  NS_LOG_FUNCTION(this);
  if (m_spectrumPropagationLoss) {
    params->psd = m_spectrumPropagationLoss->CalcRxPowerSpectralDensity(
        params, params->txPhy->GetMobility(), receiver->GetMobility());
  } else if (m_phasedArraySpectrumPropagationLoss) {
    Ptr<const PhasedArrayModel> txPhasedArrayModel =
        DynamicCast<PhasedArrayModel>(params->txPhy->GetAntenna());
    Ptr<const PhasedArrayModel> rxPhasedArrayModel =
        DynamicCast<PhasedArrayModel>(receiver->GetAntenna());

    NS_ASSERT_MSG(
        txPhasedArrayModel && rxPhasedArrayModel,
        "PhasedArrayModel instances should be installed at both TX and RX "
        "SpectrumPhy in order to use PhasedArraySpectrumPropagationLoss.");

    params->psd =
        m_phasedArraySpectrumPropagationLoss->CalcRxPowerSpectralDensity(
            params, params->txPhy->GetMobility(), receiver->GetMobility(),
            txPhasedArrayModel, rxPhasedArrayModel);
  }
  receiver->StartRx(params);
}

std::size_t MultiModelSpectrumChannel::GetNDevices() const {
  return m_numDevices;
}

Ptr<NetDevice> MultiModelSpectrumChannel::GetDevice(std::size_t i) const {
  NS_ASSERT(i < m_numDevices);
  std::size_t j = 0;
  for (auto rxInfoIterator = m_rxSpectrumModelInfoMap.begin();
       rxInfoIterator != m_rxSpectrumModelInfoMap.end(); ++rxInfoIterator) {
    for (const auto &phyIt : rxInfoIterator->second.m_rxPhys) {
      if (j == i) {
        return (*phyIt).GetDevice();
      }
      j++;
    }
  }
  NS_FATAL_ERROR("m_numDevices > actual number of devices");
  return nullptr;
}

} // namespace ns3
