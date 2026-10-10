# -*-  Mode: Python; -*-

import sys
from ns import ns


def main(argv):
    ns.core.CommandLine().Parse(argv)

    ns.core.GlobalValue.Bind(
        "SimulatorImplementationType", ns.core.StringValue("ns3::RealtimeSimulatorImpl")
    )
    ns.core.GlobalValue.Bind("ChecksumEnabled", ns.core.BooleanValue(True))

    nodes = ns.network.NodeContainer()
    nodes.Create(2)
    wifi = ns.wifi.WifiHelper()
    wifi.SetStandard(ns.wifi.WIFI_STANDARD_80211a)
    wifi.SetRemoteStationManager(
        "ns3::ConstantRateWifiManager",
        "DataMode",
        ns.core.StringValue("OfdmRate54Mbps"),
    )
    wifiMac = ns.wifi.WifiMacHelper()
    wifiMac.SetType("ns3::AdhocWifiMac")
    wifiChannel = ns.wifi.YansWifiChannelHelper.Default()
    wifiPhy = ns.wifi.YansWifiPhyHelper()
    wifiPhy.SetChannel(wifiChannel.Create())

    devices = wifi.Install(wifiPhy, wifiMac, nodes)

    mobility = ns.mobility.MobilityHelper()
    positionAlloc = ns.mobility.ListPositionAllocator()
    positionAlloc.Add(ns.core.Vector(0.0, 0.0, 0.0))
    positionAlloc.Add(ns.core.Vector(5.0, 0.0, 0.0))
    mobility.SetPositionAllocator(positionAlloc)
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel")
    mobility.Install(nodes)

    tapBridge = ns.tap_bridge.TapBridgeHelper()
    tapBridge.SetAttribute("Mode", ns.core.StringValue("UseLocal"))
    tapBridge.SetAttribute("DeviceName", ns.core.StringValue("tap-left"))
    tapBridge.Install(nodes.Get(0), devices.Get(0))
    tapBridge.SetAttribute("DeviceName", ns.core.StringValue("tap-right"))
    tapBridge.Install(nodes.Get(1), devices.Get(1))
    ns.core.Simulator.Stop(ns.core.Seconds(600))
    ns.core.Simulator.Run()
    ns.core.Simulator.Destroy()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
