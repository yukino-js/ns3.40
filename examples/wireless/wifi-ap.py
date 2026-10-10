# -*-  Mode: Python; -*-

import sys

from ns import ns


ns.cppyy.cppdef("""
    using namespace ns3;
    void AdvancePosition(Ptr<Node> node){
        Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
        Vector pos = mob->GetPosition();
        pos.x += 5.0;
        if (pos.x >= 210.0)
            return;
        mob->SetPosition(pos);
        Simulator::Schedule(Seconds(1.0), AdvancePosition, node);
    }""")


def main(argv):
    ns.core.CommandLine().Parse(argv)

    ns.network.Packet.EnablePrinting()

    wifi = ns.wifi.WifiHelper()
    mobility = ns.mobility.MobilityHelper()
    stas = ns.network.NodeContainer()
    ap = ns.network.NodeContainer()
    packetSocket = ns.network.PacketSocketHelper()

    stas.Create(2)
    ap.Create(1)

    packetSocket.Install(stas)
    packetSocket.Install(ap)

    wifiPhy = ns.wifi.YansWifiPhyHelper()
    wifiChannel = ns.wifi.YansWifiChannelHelper.Default()
    wifiPhy.SetChannel(wifiChannel.Create())

    ssid = ns.wifi.Ssid("wifi-default")
    wifiMac = ns.wifi.WifiMacHelper()

    wifiMac.SetType(
        "ns3::StaWifiMac",
        "ActiveProbing",
        ns.core.BooleanValue(True),
        "Ssid",
        ns.wifi.SsidValue(ssid),
    )
    staDevs = wifi.Install(wifiPhy, wifiMac, stas)
    wifiMac.SetType("ns3::ApWifiMac", "Ssid", ns.wifi.SsidValue(ssid))
    wifi.Install(wifiPhy, wifiMac, ap)

    mobility.Install(stas)
    mobility.Install(ap)

    ns.core.Simulator.Schedule(
        ns.core.Seconds(1.0), ns.cppyy.gbl.AdvancePosition, ap.Get(0)
    )

    socket = ns.network.PacketSocketAddress()
    socket.SetSingleDevice(staDevs.Get(0).GetIfIndex())
    socket.SetPhysicalAddress(staDevs.Get(1).GetAddress())
    socket.SetProtocol(1)

    onoff = ns.applications.OnOffHelper("ns3::PacketSocketFactory", socket.ConvertTo())
    onoff.SetConstantRate(ns.network.DataRate("500kb/s"))

    apps = onoff.Install(ns.network.NodeContainer(stas.Get(0)))
    apps.Start(ns.core.Seconds(0.5))
    apps.Stop(ns.core.Seconds(43.0))

    ns.core.Simulator.Stop(ns.core.Seconds(44.0))

    ns.core.Simulator.Run()
    ns.core.Simulator.Destroy()

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
