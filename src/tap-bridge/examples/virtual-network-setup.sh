#!/bin/bash

ip link add br-left type bridge
ip link add br-right type bridge
ip tuntap add mode tap tap-left
ip tuntap add mode tap tap-right
ip link set tap-left promisc on up
ip link set tap-right promisc on up
ip link set dev tap-left master br-left
ip link set dev br-left up
ip link set dev tap-right master br-right
ip link set dev br-right up
pushd /proc/sys/net/bridge
for f in bridge-nf-*; do echo 0 >$f; done
popd
lxc-create -n left -t ubuntu -f lxc-left.conf
lxc-create -n right -t ubuntu -f lxc-right.conf
lxc-start -n left -d
lxc-start -n right -d
