# Reasoning tutorial

Both C++ and Python tutorials perform **infinite-time safety verification**
using `verify_safety` on the same one-dimensional contracting vector field:

- Dynamics: `x' = 3/8 - x` (attracting equilibrium `x = 3/8`).
- Initial states: `5/16 <= x <= 11/32`.
- Safe region: `-2 <= x <= 2`.
- Grid fineness: `2`.

The equilibrium is inside a grid cell, so the chain-reachability computation
can stabilise without an expensive expansion of the reachable grid. Each
tutorial checks that the returned certificate **proves** safety for all time,
and fails for indeterminate results rather than claiming they are unsafe.
Unlike `upper_reach`, `verify_safety` attempts a safety proof, not merely a
finite-time reachable-set approximation.

## C++ tutorial

After installing the library:

```bash
cmake -S tutorial -B tutorial/build -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build tutorial/build --parallel
./tutorial/build/tutorial_reasoning
```

## Python tutorial

Point `PYTHONPATH` to the installed `pyariadne_reasoning` extension and configure the
dynamic library search path if needed, then run:

```bash
python tutorial/tutorial_reasoning.py
```

The Python tutorial constructs the field, integrator, evolver, initial and
safe sets, and calls `ContinuousReachabilityAnalyser.verify_safety` directly.
It reads `SafetyCertificate.proved_safe()` (which is true only for a proven
safety result) and `chain_reach_size()` for the computed certificate.

The Python tutorial is also registered as a CTest test. A separate Python
test builds a single-mode hybrid automaton, constructs a `GeneralHybridEvolver`
and `HybridReachabilityAnalyser`, computes a finite-time reach set, and
requests an infinite-time `HybridSafetyCertificate`.

The hybrid bindings also expose `HybridBoundedConstraintSet`,
`HybridReachabilityAnalyserConfiguration`, and the main analyser methods.
A bounded hybrid constraint set supplies both the compact initial set and the
open safe set for `verify_safety`.
