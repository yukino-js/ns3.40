#!/usr/bin/perl

  while(<>) {
    s|ns3::PppHeader \(Point-to-Point Protocol: IP \(0x0021\)\) ||;
    s|/TxQueue||;
    s|/TxQ/|Q|;
    s|NodeList/|N|;
    s|/DeviceList/|D|;
    s|/MacRx||;
    s|/Enqueue||;
    s|/Dequeue||;
    s|/\$ns3::QbbNetDevice||;
    s|/\$ns3::PointToPointNetDevice||;
    s| /|\t|;
    s| ns3::|\t|g;
    s|tos 0x0 ||;
    s|protocol 6 ||;
    s|offset 0 ||;
    s|flags \[none\] ||;
    s|length:|len|;
    s|Header||g;
    s|/PhyRxDrop||;
    print;
 };
