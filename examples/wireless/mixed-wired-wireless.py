from ns import ns


def main(argv):
    from ctypes import c_int, c_double

    backboneNodes = c_int(10)
    infraNodes = c_int(2)
    lanNodes = c_int(2)
    stopTime = c_double(20)
    cmd = ns.CommandLine(__file__)

    ns.core.Config.SetDefault(
        "ns3::OnOffApplication::PacketSize", ns.core.StringValue("1472")
    )
    ns.core.Config.SetDefault(
        "ns3::OnOffApplication::DataRate", ns.core.StringValue("100kb/s")
    )

    cmd.AddValue("backboneNodes", "number of backbone nodes", backboneNodes)
    cmd.AddValue("infraNodes", "number of leaf nodes", infraNodes)
    cmd.AddValue("lanNodes", "number of LAN nodes", lanNodes)
    cmd.AddValue["double"]("stopTime", "simulation stop time(seconds)", stopTime)

    cmd.Parse(argv)

    if stopTime.value < 10:
        print("Use a simulation stop time >= 10 seconds")
        exit(1)

    backbone = ns.network.NodeContainer()
    backbone.Create(backboneNodes.value)
    wifi = ns.wifi.WifiHelper()
    mac = ns.wifi.WifiMacHelper()
    mac.SetType("ns3::AdhocWifiMac")
    wifi.SetRemoteStationManager(
        "ns3::ConstantRateWifiManager",
        "DataMode",
        ns.core.StringValue("OfdmRate54Mbps"),
    )
    wifiPhy = ns.wifi.YansWifiPhyHelper()
    wifiPhy.SetPcapDataLinkType(wifiPhy.DLT_IEEE802_11_RADIO)
    wifiChannel = ns.wifi.YansWifiChannelHelper.Default()
    wifiPhy.SetChannel(wifiChannel.Create())
    backboneDevices = wifi.Install(wifiPhy, mac, backbone)
    print("Enabling OLSR routing on all backbone nodes")
    internet = ns.internet.InternetStackHelper()
    olsr = ns.olsr.OlsrHelper()
    internet.SetRoutingHelper(olsr)
    internet.Install(backbone)
    ipAddrs = ns.internet.Ipv4AddressHelper()
    ipAddrs.SetBase(
        ns.network.Ipv4Address("192.168.0.0"), ns.network.Ipv4Mask("255.255.255.0")
    )
    ipAddrs.Assign(backboneDevices)

    mobility = ns.mobility.MobilityHelper()
    mobility.SetPositionAllocator(
        "ns3::GridPositionAllocator",
        "MinX",
        ns.core.DoubleValue(20.0),
        "MinY",
        ns.core.DoubleValue(20.0),
        "DeltaX",
        ns.core.DoubleValue(20.0),
        "DeltaY",
        ns.core.DoubleValue(20.0),
        "GridWidth",
        ns.core.UintegerValue(5),
        "LayoutType",
        ns.core.StringValue("RowFirst"),
    )
    mobility.SetMobilityModel(
        "ns3::RandomDirection2dMobilityModel",
        "Bounds",
        ns.mobility.RectangleValue(ns.mobility.Rectangle(-500, 500, -500, 500)),
        "Speed",
        ns.core.StringValue("ns3::ConstantRandomVariable[Constant=2]"),
        "Pause",
        ns.core.StringValue("ns3::ConstantRandomVariable[Constant=0.2]"),
    )
    mobility.Install(backbone)

    ipAddrs.SetBase(
        ns.network.Ipv4Address("172.16.0.0"), ns.network.Ipv4Mask("255.255.255.0")
    )

    for i in range(backboneNodes.value):
        print("Configuring local area network for backbone node ", i)
        newLanNodes = ns.network.NodeContainer()
        newLanNodes.Create(lanNodes.value - 1)
        lan = ns.network.NodeContainer(
            ns.network.NodeContainer(backbone.Get(i)), newLanNodes
        )
        csma = ns.csma.CsmaHelper()
        csma.SetChannelAttribute(
            "DataRate", ns.network.DataRateValue(ns.network.DataRate(5000000))
        )
        csma.SetChannelAttribute("Delay", ns.core.TimeValue(ns.core.MilliSeconds(2)))
        lanDevices = csma.Install(lan)
        internet.Install(newLanNodes)
        ipAddrs.Assign(lanDevices)
        ipAddrs.NewNetwork()
        mobilityLan = ns.mobility.MobilityHelper()
        positionAlloc = ns.mobility.ListPositionAllocator()
        for j in range(newLanNodes.GetN()):
            positionAlloc.Add(ns.core.Vector(0.0, (j * 10 + 10), 0.0))

        mobilityLan.SetPositionAllocator(positionAlloc)
        mobilityLan.PushReferenceMobilityModel(backbone.Get(i))
        mobilityLan.SetMobilityModel("ns3::ConstantPositionMobilityModel")
        mobilityLan.Install(newLanNodes)

    ipAddrs.SetBase(
        ns.network.Ipv4Address("10.0.0.0"), ns.network.Ipv4Mask("255.255.255.0")
    )
    tempRef = []
    for i in range(backboneNodes.value):
        print("Configuring wireless network for backbone node ", i)
        stas = ns.network.NodeContainer()
        stas.Create(infraNodes.value - 1)
        infra = ns.network.NodeContainer(
            ns.network.NodeContainer(backbone.Get(i)), stas
        )
        ssid = ns.wifi.Ssid("wifi-infra" + str(i))
        wifiInfra = ns.wifi.WifiHelper()
        wifiPhy.SetChannel(wifiChannel.Create())
        macInfra = ns.wifi.WifiMacHelper()
        macInfra.SetType("ns3::StaWifiMac", "Ssid", ns.wifi.SsidValue(ssid))

        staDevices = wifiInfra.Install(wifiPhy, macInfra, stas)
        macInfra.SetType("ns3::ApWifiMac", "Ssid", ns.wifi.SsidValue(ssid))
        apDevices = wifiInfra.Install(wifiPhy, macInfra, backbone.Get(i))
        infraDevices = ns.network.NetDeviceContainer(apDevices, staDevices)

        internet.Install(stas)
        ipAddrs.Assign(infraDevices)
        ipAddrs.NewNetwork()

        subnetAlloc = ns.mobility.ListPositionAllocator()

        tempRef.append(subnetAlloc)

        for j in range(infra.GetN()):
            subnetAlloc.Add(ns.core.Vector(0.0, j, 0.0))

        mobility.PushReferenceMobilityModel(backbone.Get(i))
        mobility.SetPositionAllocator(subnetAlloc)
        mobility.SetMobilityModel(
            "ns3::RandomDirection2dMobilityModel",
            "Bounds",
            ns.mobility.RectangleValue(ns.mobility.Rectangle(-10, 10, -10, 10)),
            "Speed",
            ns.core.StringValue("ns3::ConstantRandomVariable[Constant=3]"),
            "Pause",
            ns.core.StringValue("ns3::ConstantRandomVariable[Constant=0.4]"),
        )
        mobility.Install(stas)

    print("Create Applications.")
    port = 9

    appSource = ns.network.NodeList.GetNode(backboneNodes.value)
    lastNodeIndex = (
        backboneNodes.value
        + backboneNodes.value * (lanNodes.value - 1)
        + backboneNodes.value * (infraNodes.value - 1)
        - 1
    )
    appSink = ns.network.NodeList.GetNode(lastNodeIndex)

    ns.cppyy.cppdef("""
        Ipv4Address getIpv4AddressFromNode(Ptr<Node> node){
        return node->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
        }
    """)
    remoteAddr = ns.cppyy.gbl.getIpv4AddressFromNode(appSink)
    socketAddr = ns.network.InetSocketAddress(remoteAddr, port)
    onoff = ns.applications.OnOffHelper("ns3::UdpSocketFactory", socketAddr.ConvertTo())
    apps = onoff.Install(ns.network.NodeContainer(appSource))
    apps.Start(ns.core.Seconds(3))
    apps.Stop(ns.core.Seconds(stopTime.value - 1))

    sink = ns.applications.PacketSinkHelper(
        "ns3::UdpSocketFactory",
        ns.network.InetSocketAddress(
            ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), port)
        ).ConvertTo(),
    )
    sinkContainer = ns.network.NodeContainer(appSink)
    apps = sink.Install(sinkContainer)
    apps.Start(ns.core.Seconds(3))

    print("Configure Tracing.")
    csma = ns.csma.CsmaHelper()
    ascii = ns.network.AsciiTraceHelper()
    stream = ascii.CreateFileStream("mixed-wireless.tr")
    wifiPhy.EnableAsciiAll(stream)
    csma.EnableAsciiAll(stream)
    internet.EnableAsciiIpv4All(stream)
    csma.EnablePcapAll("mixed-wireless", False)
    wifiPhy.EnablePcap("mixed-wireless", backboneDevices)
    wifiPhy.EnablePcap("mixed-wireless", appSink.GetId(), 0)

    print("Run Simulation.")
    ns.core.Simulator.Stop(ns.core.Seconds(stopTime.value))
    ns.core.Simulator.Run()
    ns.core.Simulator.Destroy()


if __name__ == "__main__":
    import sys

    main(sys.argv)
