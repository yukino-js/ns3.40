# -*- Mode:Python; -*-


import numpy as np
import matplotlib.pyplot as plt
import sys
import argparse
from ns import ns


def main():
    parser = argparse.ArgumentParser("sample-rng-plot")
    parser.add_argument("--not-blocking", action="store_true", default=False)
    args = parser.parse_args(sys.argv[1:])

    rng = ns.CreateObject("NormalRandomVariable")
    rng.SetAttribute("Mean", ns.DoubleValue(100.0))
    rng.SetAttribute("Variance", ns.DoubleValue(225.0))

    x = [rng.GetValue() for t in range(10000)]

    density = 1
    facecolor = "g"
    alpha = 0.75

    n, bins, patches = plt.hist(x, 50, density=True, facecolor="g", alpha=0.75)

    plt.title("ns-3 histogram")
    plt.text(60, 0.025, r"$\mu=100,\ \sigma=15$")
    plt.axis([40, 160, 0, 0.03])
    plt.grid(True)
    plt.show(block=not args.not_blocking)


if __name__ == "__main__":
    main()
