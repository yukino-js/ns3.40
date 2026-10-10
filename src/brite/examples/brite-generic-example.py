from ns import ns

ns.LogComponentEnable("BriteTopologyHelper", ns.LOG_LEVEL_ALL)

confFile = "src/brite/examples/conf_files/TD_ASBarabasi_RTWaxman.conf"

bth = ns.BriteTopologyHelper(confFile)
bth.AssignStreams(3)

p2p = ns.PointToPointHelper()

stack = ns.InternetStackHelper()

nixRouting = ns.Ipv4NixVectorHelper()
stack.SetRoutingHelper(nixRouting)

address = ns.Ipv4AddressHelper()
address.SetBase("10.0.0.0", "255.255.255.252")

bth.BuildBriteTopology(stack)
bth.AssignIpv4Addresses(address)

print(f"Number of AS created {bth.GetNAs()}")

client = ns.NodeContainer()
server = ns.NodeContainer()

client.Create(1)
stack.Install(client)

numLeafNodesInAsZero = bth.GetNLeafNodesForAs(0)
client.Add(bth.GetLeafNodeForAs(0, numLeafNodesInAsZero - 1))

server.Create(1)
stack.Install(server)

numLeafNodesInAsOne = bth.GetNLeafNodesForAs(1)
server.Add(bth.GetLeafNodeForAs(1, numLeafNodesInAsOne - 1))

p2p.SetDeviceAttribute("DataRate", ns.StringValue("5Mbps"))
p2p.SetChannelAttribute("Delay", ns.StringValue("2ms"))

p2pClientDevices = p2p.Install(client)
p2pServerDevices = p2p.Install(server)

address.SetBase("10.1.0.0", "255.255.0.0")
clientInterfaces = address.Assign(p2pClientDevices)

address.SetBase("10.2.0.0", "255.255.0.0")
serverInterfaces = ns.Ipv4InterfaceContainer()
serverInterfaces = address.Assign(p2pServerDevices)

echoServer = ns.UdpEchoServerHelper(9)
serverApps = echoServer.Install(server.Get(0))
serverApps.Start(ns.Seconds(1.0))
serverApps.Stop(ns.Seconds(5.0))

echoClient = ns.UdpEchoClientHelper(serverInterfaces.GetAddress(0).ConvertTo(), 9)
echoClient.SetAttribute("MaxPackets", ns.UintegerValue(1))
echoClient.SetAttribute("Interval", ns.TimeValue(ns.Seconds(1.0)))
echoClient.SetAttribute("PacketSize", ns.UintegerValue(1024))

clientApps = echoClient.Install(client.Get(0))
clientApps.Start(ns.Seconds(2.0))
clientApps.Stop(ns.Seconds(5.0))

asciiTrace = ns.AsciiTraceHelper()
p2p.EnableAsciiAll(asciiTrace.CreateFileStream("briteLeaves.tr"))

ns.Simulator.Stop(ns.Seconds(6.0))
ns.Simulator.Run()
ns.Simulator.Destroy()
