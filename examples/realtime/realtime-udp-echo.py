from ns import ns


def main(argv):
    cmd = ns.core.CommandLine()
    cmd.Parse(argv)

    ns.core.GlobalValue.Bind(
        "SimulatorImplementationType", ns.core.StringValue("ns3::RealtimeSimulatorImpl")
    )

    print("Create nodes.")
    n = ns.network.NodeContainer()
    n.Create(4)

    internet = ns.internet.InternetStackHelper()
    internet.Install(n)

    print("Create channels.")
    csma = ns.csma.CsmaHelper()
    csma.SetChannelAttribute(
        "DataRate", ns.network.DataRateValue(ns.network.DataRate(5000000))
    )
    csma.SetChannelAttribute("Delay", ns.core.TimeValue(ns.core.MilliSeconds(2)))
    csma.SetDeviceAttribute("Mtu", ns.core.UintegerValue(1400))
    d = csma.Install(n)

    print("Assign IP Addresses.")
    ipv4 = ns.internet.Ipv4AddressHelper()
    ipv4.SetBase(
        ns.network.Ipv4Address("10.1.1.0"), ns.network.Ipv4Mask("255.255.255.0")
    )
    i = ipv4.Assign(d)

    print("Create Applications.")

    port = 9
    server = ns.applications.UdpEchoServerHelper(port)
    apps = server.Install(n.Get(1))
    apps.Start(ns.core.Seconds(1.0))
    apps.Stop(ns.core.Seconds(10.0))

    packetSize = 1024
    maxPacketCount = 500
    interPacketInterval = ns.core.Seconds(0.01)
    client = ns.applications.UdpEchoClientHelper(i.GetAddress(1).ConvertTo(), port)
    client.SetAttribute("MaxPackets", ns.core.UintegerValue(maxPacketCount))
    client.SetAttribute("Interval", ns.core.TimeValue(interPacketInterval))
    client.SetAttribute("PacketSize", ns.core.UintegerValue(packetSize))
    apps = client.Install(n.Get(0))
    apps.Start(ns.core.Seconds(2.0))
    apps.Stop(ns.core.Seconds(10.0))

    ascii = ns.network.AsciiTraceHelper()
    csma.EnableAsciiAll(ascii.CreateFileStream("realtime-udp-echo.tr"))
    csma.EnablePcapAll("realtime-udp-echo", False)

    print("Run Simulation.")
    ns.core.Simulator.Stop(ns.Seconds(10))
    ns.core.Simulator.Run()
    ns.core.Simulator.Destroy()
    print("Done.")


if __name__ == "__main__":
    import sys

    main(sys.argv)
