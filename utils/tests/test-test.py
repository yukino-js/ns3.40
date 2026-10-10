#! /usr/bin/env python3


from __future__ import print_function
from TestBase import TestBaseClass
import sys


def main(argv):
    """
    Prepares test cases and executes
    """
    test_cases = [
        "",
        "-h",
        "--help",
        "-b build/",
        "--buildpath=build/",
        "-c performance",
        "--constrain=performance",
        "-d",
        "--duration",
        "-e socket-options-ipv6",
        "--example=socket-options-ipv6",
        "-u",
        "--update-data",
        "-f EXTENSIVE",
        "--fullness=EXTENSIVE",
        "-g",
        "--grind",
        "-l",
        "--list",
        "-m",
        "--multiple",
        "-n",
        "--no-build",
        "-p first",
        "--pyexample=first",
        "-r",
        "--retain",
        "-s ns3-tcp-state",
        "--suite=ns3-tcp-state",
        "-t t_opt.txt",
        "--text=t_opt.txt && rm t_opt.txt",
        "-v",
        "--verbose",
        "-w t_opt.html && rm t_opt.html",
        "--web=t_opt.html && rm t_opt.html",
        "--html=t_opt.html && rm t_opt.html",
        "-x t_opt.xml && rm t_opt.xml",
        "--xml=t_opt.xml && rm t_opt.xml",
    ]

    configure_string = (
        sys.executable + " ns3 configure --enable-tests --enable-examples"
    )
    clean_string = sys.executable + " ns3 clean"
    cmd_execute_list = [
        "%s && %s test.py %s && %s"
        % (configure_string, sys.executable, option, clean_string)
        for option in test_cases
    ]
    runner = TestBaseClass(
        argv[1:], "Test suite for the ns-3 unit test runner", "test-py"
    )
    return runner.runtests(cmd_execute_list)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
