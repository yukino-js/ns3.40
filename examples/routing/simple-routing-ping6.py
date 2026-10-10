from ns import ns


def main(argv):
    cmd = ns.CommandLine()

    cmd.Parse(argv)

    print("Create nodes")

    all = ns.NodeContainer(3)
    net1 = ns.NodeContainer()
    net1.Add(all.Get(0))
    net1.Add(all.Get(1))
    net2 = ns.NodeContainer()
    net2.Add(all.Get(1))
    net2.Add(all.Get(2))

    internetv6 = ns.InternetStackHelper()
    internetv6.Install(all)

    csma = ns.csma.CsmaHelper()
    csma.SetChannelAttribute("DataRate", ns.DataRateValue(ns.DataRate(5000000)))
    csma.SetChannelAttribute("Delay", ns.TimeValue(ns.MilliSeconds(2)))
    d1 = csma.Install(net1)
    d2 = csma.Install(net2)

    print("Addressing")
    ipv6 = ns.Ipv6AddressHelper()
    ipv6.SetBase(ns.Ipv6Address("2001:1::"), ns.Ipv6Prefix(64))
    i1 = ipv6.Assign(d1)
    i1.SetForwarding(1, True)
    i1.SetDefaultRouteInAllNodes(1)
    ipv6.SetBase(ns.Ipv6Address("2001:2::"), ns.Ipv6Prefix(64))
    i2 = ipv6.Assign(d2)
    i2.SetForwarding(0, True)
    i2.SetDefaultRouteInAllNodes(0)

    print("Application")
    packetSize = 1024
    maxPacketCount = 5
    interPacketInterval = ns.Seconds(1.0)
    ping = ns.PingHelper(i2.GetAddress(1, 1).ConvertTo())

    ping.SetAttribute("Count", ns.UintegerValue(maxPacketCount))
    ping.SetAttribute("Interval", ns.TimeValue(interPacketInterval))
    ping.SetAttribute("Size", ns.UintegerValue(packetSize))

    apps = ping.Install(ns.NodeContainer(net1.Get(0)))
    apps.Start(ns.Seconds(2.0))
    apps.Stop(ns.Seconds(20.0))

    print("Tracing")
    ascii = ns.AsciiTraceHelper()
    csma.EnableAsciiAll(ascii.CreateFileStream("simple-routing-ping6.tr"))
    csma.EnablePcapAll("simple-routing-ping6", True)

    ns.Simulator.Run()
    ns.Simulator.Destroy()


if __name__ == "__main__":
    import sys

    main(sys.argv)
