"""Check Reasoning bindings in the composed Python module."""
import pyariadne_reasoning as a

def test_reasoning_bindings():
    for name in ("SatSolver", "SmtSolver", "AugmentedPaving", "LabelledAugmentedPavingValue",
                 "LabelledAugmentedPavingValue",
                 "ContinuousReachabilityAnalyser", "SafetyCertificate", "GeneralHybridEvolver",
                 "HybridBoundedConstraintSet", "HybridSafetyCertificate",
                 "HybridReachabilityAnalyser", "HybridReachabilityAnalyserConfiguration",
                 "ReachabilityAnalyserConfiguration"):
        assert hasattr(a, name), name
    # Binding-level coverage only: C++ regression exercises the snapshots'
    # geometry and ownership; the Release workflows already run the tutorial.
    for method in ("chain_reach_value", "safe_value"):
        assert callable(getattr(a.SafetyCertificate, method))
    # The C++ regressions verify actual set, backend and snapshot semantics;
    # this import test verifies the public Python entry points, not the tutorial.
    for method in ("lower_evolve_value", "lower_reach_evolve_values",
                   "lower_reach_value", "upper_evolve_value",
                   "upper_reach_evolve_values", "upper_reach_value",
                   "outer_chain_reach_value"):
        assert callable(getattr(a.ContinuousReachabilityAnalyser, method))

def test_generic_labelled_paving_values():
    """Use generic labelled values from Python without legacy subclass APIs."""
    x = a.RealVariable("x")
    y = a.RealVariable("y")
    space = a.RealSpace([y, x])
    grid = a.Grid(2)

    labelled_grid = a.make_labelled_grid(grid, space)
    assert isinstance(labelled_grid, a.LabelledGridValue)
    assert labelled_grid.space().dimension() == 2
    assert labelled_grid.value().dimension() == 2
    assert a.LabelledGridValue(grid, space).value().dimension() == 2

    cell = a.GridCell(grid, 0, a.BinaryWord(""))
    original_tree = a.Paving.tree(grid)
    labelled_tree = a.make_labelled_paving(original_tree, space)
    assert isinstance(labelled_tree, a.LabelledPavingValue)
    assert labelled_tree.value().is_empty()
    labelled_tree.value().adjoin(cell)
    assert labelled_tree.value().superset(cell)
    assert original_tree.is_empty()  # The labelled wrapper owns its backend.
    assert labelled_tree.space().dimension() == 2

    manager = a.RobddManager()
    original_robdd = a.Paving.robdd(grid, manager)
    labelled_robdd = a.LabelledPavingValue(original_robdd, space)
    assert labelled_robdd.value().is_empty()
    labelled_robdd.value().adjoin(cell)
    assert labelled_robdd.value().superset(cell)
    assert original_robdd.is_empty()

    # Both constructors enforce the ordered space dimension as in C++.
    wrong_space = a.RealSpace([x])
    for create in (lambda: a.make_labelled_grid(grid, wrong_space),
                   lambda: a.make_labelled_paving(original_tree, wrong_space)):
        try:
            create()
        except RuntimeError as error:
            assert "Precondition" in str(error) and "dimension" in str(error)
        else:
            raise AssertionError("Mismatched labelled space dimension was accepted")


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
    test_generic_labelled_paving_values()
    test_neural_symbolic_expansion()
    test_hybrid_reachability_analyser()
