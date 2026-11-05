#!/usr/bin/env python3
"""Verify an infinite-time safety property for a one-dimensional contraction.

The same example is exercised by the C++ tutorial and test_verify_safety.cpp.
"""

from pyariadne_reasoning import (
    ContinuousReachabilityAnalyser,
    GradedTaylorSeriesIntegrator,
    Rational,
    Real,
    RealExpressionBoundedConstraintSet,
    RealSpace,
    RealVariable,
    RealVariableInterval,
    VectorField,
    VectorFieldEvolver,
    dot,
)


def exact_real(numerator, denominator=1):
    """Use exact rational bounds instead of binary floating-point literals."""
    return Real(Rational(numerator, denominator))


def verify_contracting_system_safety():
    x = RealVariable("x")

    # Dynamics: x' = 3/8 - x; all trajectories converge to x = 3/8.
    field = VectorField([dot(x) << (exact_real(3, 8) - x)])
    integrator = GradedTaylorSeriesIntegrator(1e-2)
    evolver = VectorFieldEvolver(field, integrator)

    analyser = ContinuousReachabilityAnalyser(evolver)
    analyser.configuration().set_maximum_grid_fineness(2)

    # Initial: 5/16 <= x <= 11/32. Safe: -2 <= x <= 2.
    state_space = RealSpace([x])
    initial = RealExpressionBoundedConstraintSet(
        [RealVariableInterval(exact_real(5, 16), x, exact_real(11, 32))],
        [],
    ).euclidean_set(state_space)
    safe = RealExpressionBoundedConstraintSet(
        [RealVariableInterval(exact_real(-2), x, exact_real(2))],
        [],
    ).euclidean_set(state_space)

    # This is an infinite-time verification, not just finite-time reachability.
    certificate = analyser.verify_safety(initial, safe)
    if not certificate.proved_safe():
        raise RuntimeError("Safety could not be proved (possibly indeterminate)")

    cells = certificate.chain_reach_size()
    if cells == 0:
        raise RuntimeError("Safety certificate has an empty chain-reachable set")

    print("Infinite-time safety verified.")
    print(f"Chain-reachable grid cells: {cells}")
    return certificate


if __name__ == "__main__":
    verify_contracting_system_safety()
