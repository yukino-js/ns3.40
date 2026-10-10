from ns import ns


def main(argv):
    """The main function in this Battery discharge example

    Parameters:
    argv: System parameters to use if necessary
    """

    ns.core.LogComponentEnable("GenericBatteryModel", ns.core.LOG_LEVEL_DEBUG)

    node = ns.network.Node()
    batteryHelper = ns.energy.GenericBatteryModelHelper()
    batteryModel = ns.CreateObject("GenericBatteryModel")
    devicesEnergyModel = ns.energy.SimpleDeviceEnergyModel()

    batteryModel.SetAttribute("FullVoltage", ns.core.DoubleValue(1.39))
    batteryModel.SetAttribute("MaxCapacity", ns.core.DoubleValue(7.0))

    batteryModel.SetAttribute("NominalVoltage", ns.core.DoubleValue(1.18))
    batteryModel.SetAttribute("NominalCapacity", ns.core.DoubleValue(6.25))

    batteryModel.SetAttribute("ExponentialVoltage", ns.core.DoubleValue(1.28))
    batteryModel.SetAttribute("ExponentialCapacity", ns.core.DoubleValue(1.3))

    batteryModel.SetAttribute("InternalResistance", ns.core.DoubleValue(0.0046))
    batteryModel.SetAttribute("TypicalDischargeCurrent", ns.core.DoubleValue(1.3))
    batteryModel.SetAttribute("CutoffVoltage", ns.core.DoubleValue(1.0))

    batteryModel.SetAttribute("BatteryType", ns.core.EnumValue(ns.NIMH_NICD))

    devicesEnergyModel.SetEnergySource(batteryModel)
    batteryModel.AppendDeviceEnergyModel(devicesEnergyModel)
    devicesEnergyModel.SetNode(node)

    devicesEnergyModel.SetCurrentA(6.5)

    ns.core.Simulator.Stop(ns.core.Seconds(3600))
    ns.core.Simulator.Run()
    ns.core.Simulator.Destroy()


if __name__ == "__main__":
    import sys

    main(sys.argv)
