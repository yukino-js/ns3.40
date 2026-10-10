#! /usr/bin/env python3


cpp_examples = [
    (
        "nsclick-simple-lan --clickConfigFolder=../../src/click/examples",
        "NSCLICK == True",
        "False",
    ),
    (
        "nsclick-raw-wlan --clickConfigFolder=../../src/click/examples",
        "NSCLICK == True",
        "False",
    ),
    (
        "nsclick-udp-client-server-csma --clickConfigFolder=../../src/click/examples",
        "NSCLICK == True",
        "False",
    ),
    (
        "nsclick-udp-client-server-wifi --clickConfigFolder=../../src/click/examples",
        "NSCLICK == True",
        "False",
    ),
    (
        "nsclick-routing --clickConfigFolder=../../src/click/examples",
        "NSCLICK == True",
        "False",
    ),
    (
        "nsclick-defines --clickConfigFolder=../../src/click/examples",
        "NSCLICK == True",
        "False",
    ),
]

python_examples = [
    ("nsclick-simple-lan.py", "NSCLICK == True"),
]
