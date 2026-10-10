# -*- Mode:Python; -*-


from ns import ns


def RandomFunction():
    print("RandomFunction received event at", ns.core.Simulator.Now().GetSeconds(), "s")


def CancelledEvent():
    print("I should never be called... ")


ns.cppyy.cppdef("""
    #include "CPyCppyy/API.h"

    using namespace ns3;
    /** Simple model object to illustrate event handling. */
    class MyModel
    {
    public:
      /** Start model execution by scheduling a HandleEvent. */
      void Start ();

    private:
      /**
       *  Simple event handler.
       *
       * \param [in] eventValue Event argument.
       */
      void HandleEvent (double eventValue);
    };

    void
    MyModel::Start ()
    {
      Simulator::Schedule (Seconds (10.0),
                           &MyModel::HandleEvent,
                           this, Simulator::Now ().GetSeconds ());
    }
    void
    MyModel::HandleEvent (double value)
    {
      std::cout << "Member method received event at "
                << Simulator::Now ().GetSeconds ()
                << "s started at " << value << "s" << std::endl;
    }

    void ExampleFunction(MyModel& model){
      std::cout << "ExampleFunction received event at " << Simulator::Now().GetSeconds() << "s" << std::endl;
      model.Start();
    };

    EventImpl* ExampleFunctionEvent(MyModel& model)
    {
        return MakeEvent(&ExampleFunction, model);
    }

    void RandomFunctionCpp(MyModel& model) {
        CPyCppyy::Eval("RandomFunction()");
    }

    EventImpl* RandomFunctionEvent(MyModel& model)
    {
        return MakeEvent(&RandomFunctionCpp, model);
    }

    void CancelledFunctionCpp() {
        CPyCppyy::Eval("CancelledEvent()");
    }

    EventImpl* CancelledFunctionEvent()
    {
        return MakeEvent(&CancelledFunctionCpp);
    }
   """)


def main(argv):
    cmd = ns.CommandLine(__file__)
    cmd.Parse(argv)

    model = ns.cppyy.gbl.MyModel()
    v = ns.CreateObject("UniformRandomVariable")
    v.SetAttribute("Min", ns.core.DoubleValue(10))
    v.SetAttribute("Max", ns.core.DoubleValue(20))

    ev = ns.cppyy.gbl.ExampleFunctionEvent(model)
    ns.core.Simulator.Schedule(ns.core.Seconds(10.0), ev)

    ev2 = ns.cppyy.gbl.RandomFunctionEvent(model)
    ns.core.Simulator.Schedule(ns.core.Seconds(v.GetValue()), ev2)

    ev3 = ns.cppyy.gbl.CancelledFunctionEvent()
    id = ns.core.Simulator.Schedule(ns.core.Seconds(30.0), ev3)
    ns.core.Simulator.Cancel(id)

    ns.core.Simulator.Run()

    ns.core.Simulator.Destroy()


if __name__ == "__main__":
    import sys

    main(sys.argv)
