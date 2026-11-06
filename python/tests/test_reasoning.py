"""Check Reasoning bindings in the composed Python module."""
import pyariadne_reasoning as a

def test_reasoning_bindings():
    for name in ("SatSolver", "SmtSolver", "Storage", "LabelledStorage", "HybridStorage",
                 "ContinuousReachabilityAnalyser", "SafetyCertificate", "GeneralHybridEvolver",
                 "HybridBoundedConstraintSet", "HybridSafetyCertificate",
                 "HybridReachabilityAnalyser", "HybridReachabilityAnalyserConfiguration",
                 "ReachabilityAnalyserConfiguration"):
        assert hasattr(a, name), name

def test_neural_symbolic_expansion():
    x = a.RealVariable("x")
    for activation in (a.NeuralActivation.Identity, a.NeuralActivation.Relu,
                       a.NeuralActivation.Tanh, a.NeuralActivation.Sigmoid,
                       a.NeuralActivation.Softplus):
        network = a.NeuralNetwork(1, [a.NeuralDenseLayerSpec(1, activation)])
        snapshot = a.NeuralParameterSnapshot(network, [2.0, 0.5])
        expressions = a.make_expression(snapshot, a.RealSpace([x]))
        assert len(expressions) == 1

    network = a.NeuralNetwork(2, [a.NeuralDenseLayerSpec(1, a.NeuralActivation.Identity)])
    snapshot = a.NeuralParameterSnapshot(network, [2.0, -3.0, 0.5])
    y = a.RealVariable("y")
    assert len(a.make_expression(snapshot, a.RealSpace([x, y]))) == 1
    assert len(a.make_expression(snapshot, a.RealSpace([y, x]))) == 1
    try:
        a.make_expression(snapshot, a.RealSpace([x]))
    except ValueError:
        pass
    else:
        raise AssertionError("Wrong neural input space dimension was accepted")


def test_hybrid_reachability_analyser():
    """Compute a hybrid reachability result and an infinite-time certificate."""
    x = a.RealVariable("x")
    q = a.DiscreteLocation(0)
    system = a.HybridAutomaton()
    system.new_mode(q, [a.dot(x) << (a.Real(a.Rational(3, 8)) - x)])

    evolver = a.GeneralHybridEvolver(system)
    evolver.set_integrator(a.GradedTaylorSeriesIntegrator(1e-2))
    evolver.configuration().set_maximum_step_size(0.1)
    evolver.configuration().set_maximum_enclosure_radius(1.0)

    analyser = a.HybridReachabilityAnalyser(evolver)
    analyser.configuration().set_maximum_grid_fineness(2)
    analyser.configuration().set_lock_to_grid_time(1.0)
    analyser.configuration().set_lock_to_grid_steps(1)

    def bounded(lower, upper):
        interval = a.RealVariableInterval(
            a.Real(a.Rational(lower[0], lower[1])),
            x,
            a.Real(a.Rational(upper[0], upper[1])),
        )
        return a.HybridBoundedConstraintSet(
            q, a.RealExpressionBoundedConstraintSet([interval], []))

    initial = bounded((5, 16), (11, 32))
    safe = bounded((-2, 1), (2, 1))

    # A finite horizon first checks the evolver/analyser/storage connection.
    reach = analyser.upper_reach(initial, a.HybridTime(a.Real(a.Rational(1, 10)), a.Integer(1)))
    assert reach.size() > 0

    # The certificate checks infinite-time safety, not just bounded reach.
    certificate = analyser.verify_safety(initial, safe)
    assert certificate.proved_safe(), "Hybrid safety could not be proven"
    assert certificate.chain_reach_size() > 0


if __name__ == "__main__":
    test_reasoning_bindings()
    test_neural_symbolic_expansion()
    test_hybrid_reachability_analyser()
