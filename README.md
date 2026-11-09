# Ariadne Reasoning

[![macOS Status](https://github.com/ariadne-cps/reasoning/actions/workflows/macos.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/macos.yml)
[![Ubuntu Status](https://github.com/ariadne-cps/reasoning/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/ubuntu.yml)
[![Windows Status](https://github.com/ariadne-cps/reasoning/actions/workflows/win.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/win.yml)
[![Sanitizers Status](https://github.com/ariadne-cps/reasoning/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/sanitizers.yml)
[![Coverage Status](https://github.com/ariadne-cps/reasoning/actions/workflows/coverage.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/coverage.yml)
[![codecov](https://codecov.io/gh/ariadne-cps/reasoning/branch/main/graph/badge.svg)](https://codecov.io/gh/ariadne-cps/reasoning)

Ariadne Reasoning implements reachability analysis, safety verification and
grid-backed augmented pavings for continuous and hybrid dynamical systems.
Although independently maintained and usable, this repository is also part of the [Ariadne repository structure](https://github.com/ariadne-cps/ariadne/blob/main/doc/structure.md).

## Layout

Public C++ headers are in `include/reasoning`, implementations in `src`,
tests in `tests`, Python bindings in `python`, and standalone tutorials in
`tutorial`. The umbrella header is `ariadne-reasoning.hpp`.

## Dependencies

Reasoning directly uses [Evolution](https://github.com/ariadne-cps/evolution),
[SMT](https://github.com/ariadne-cps/smt), and
[SAT](https://github.com/ariadne-cps/sat). The
[Build](https://github.com/ariadne-cps/build) submodule is
used for the initial CMake bootstrap. Evolution supplies its dependencies
transitively. C++ test executables also link [Ariadne Testing](https://github.com/ariadne-cps/testing),
pinned as the test-only `ss/tst` submodule. Reachability and safety test cases
run in isolated fixtures.

## Neural symbolic expansion

The `make_expression(snapshot, space)` function converts a frozen
neural network over an ordered `Space<Real>` into native
`Expression<Real>` objects, using exact dyadic
interpretations of stored binary64 parameters. It supports Identity, ReLU,
Tanh, Sigmoid, and Softplus activations. The result can be evaluated,
simplified, or differentiated using Ariadne's symbolic facilities; the
expansion may grow with network size. C++ and Python APIs are provided.

## Build and test

```bash
git clone --recurse-submodules https://github.com/ariadne-cps/reasoning.git
cd reasoning
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel --target everything
ctest --test-dir build --output-on-failure
```

See [the tutorial](tutorial/README.md) for a small C++ safety-verification example.

## Infinite-time reachability benchmark

The development-only `benchmark_infinite_time_reachability` executable runs
**both** infinite-time reachability workloads from
`tests/test_infinite_time_reachability.cpp` by default. Select either
`--operation lower_reach`, `--operation outer_chain_reach`, or a comma-separated
list of both. `maximum_grid_fineness` and concurrency remain configurable.

```bash
cmake --build build --target benchmark_infinite_time_reachability
./build/benchmarks/benchmark_infinite_time_reachability --maximum-grid-fineness 2,3 --operation lower_reach,outer_chain_reach --backend tree,robdd --samples 3 --concurrency 0
```

Each sample measures only the complete selected analyser call. For
`lower_reach`, `transient_time=4.0`. For `outer_chain_reach`,
`transient_time=12.0` and `lock_to_grid_time=6.0`, matching the respective
test cases. The system, initial set and bounding domain also match those tests.
Construction, validation and progress reporting happen outside timed regions.

The summary statistics are displayed **in seconds** (variance in seconds
squared). Benchmarking's stored samples retain their native nanosecond
representation. Two nested Ariadne Logging progress indicators are displayed when
`--verbosity 2` is enabled. The level-1 indicator tracks completed
warm-ups and measured executions globally across all configurations.
The level-2 held indicator tracks the warm-ups and samples of the currently
running operation/backend/fineness; it is released before the next case.
The analyser call is executed with `LOGGING_RUN_AT(2,...)`, so its
internal diagnostics begin at level 3 (or deeper) rather than mixing with
the level-2 benchmark messages. Both indicators update outside the
timed operation callback.

With default parameters there are four configurations, each with three
measured rounds (12 executions total). Progress advances when a round
finishes, not continuously *within* a long-running analyser call.
The final report is printed at level 1 after releasing the global
progress line.

`--backend tree,robdd` executes the same analyser with BinaryTree or a
shared-manager ROBDD backend for every intermediate `Paving`. Tree and ROBDD
results are compared geometrically outside timing for each fineness/operation.

`CachedBinaryTreePaving` is still a query-cache wrapper, not a mutable
`PavingInterface` backend. It cannot yet be included in genuine end-to-end
reachability measurements and is not silently replaced by BinaryTree.

Release CI **compiles** benchmarks between the library build and installation;
benchmarks are not CTest cases and are not automatically executed. The
Benchmarking submodule is a development-only dependency.

## Python

The standalone build composes upstream Python bindings with Reasoning in
`pyariadne_reasoning`. Python smoke tests run through CTest with the `python` label.

## Augmented paving migration

The canonical `AugmentedPaving` is defined in `paving` and its rendering adapter
in `drawing`. `LabelledGrid`, `LabelledMapping`, `LabelledAugmentedPaving`
and the labelled rendering adapter are defined in `labelled`. The hybrid
paving, lazy auxiliary mapping, and hybrid drawing adapter live in `hybrid`.
Enclosure-to-augmented-paving conversions are implemented in `evolution`.

Reasoning now contains only analysis-related components: analyser traits,
reachability analysers, system-to-auxiliary-mapping adapters, and orbit
specializations. Callers should directly include the appropriate module
header. No legacy forwarding headers or `Storage` aliases are provided.

### Experimental parallel cell evolution in continuous reachability

The continuous reachability analyser can evaluate distinct input grid cells
concurrently inside `_adjoin_upper_reach_evolve`. It is opt-in through the
shared `ThreadManager` concurrency setting (zero retains the serial path);
the analyser uses at most eight lanes, with the calling thread participating.
Reach and evolved enclosures are adjoined **incrementally** to the two shared
destination pavings. This preserves their covered-region pruning and avoids
intermediate per-worker pavings. Input cells are snapshotted before scheduling
to accommodate the existing input/output aliasing patterns. Tasks are joined
before the next time step; the caller's floating-point rounding is installed
per lane, and exceptions propagate after join.

The parallel path currently applies to continuous labelled storage only.
Hybrid storage's location-indexed map is not protected by the per-location
paving locks and stays serial. Each lane uses an evolver clone; integrator and
configuration handles held through shared pointers must remain read-only
throughout the analysis. Both Tree and ROBDD backends have a dedicated serial
geometry oracle in `test_parallel_reachability`. For whole-analysis timing,
`benchmark_infinite_time_reachability` already accepts `--concurrency`.
The shared-destination approach may have backend-specific lock contention;
no performance speedup is claimed without measurements.
