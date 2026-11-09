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

The development-only `benchmark_infinite_time_reachability` executable
measures the infinite-time **outer chain** reachability workload from
`tests/test_infinite_time_reachability.cpp`, without plotting.
Each measured sample executes `ReachabilityAnalyser::outer_chain_reach` in full.
The system, initial set and bounding domain match the reference test; `transient_time` is 12.0 and `lock_to_grid_time` is 6.0. Constructing and configuring
a fresh analyser and verifying its result are outside the timed region.

```bash
cmake --build build --target benchmark_infinite_time_reachability
./build/benchmarks/benchmark_infinite_time_reachability --maximum-grid-fineness 2,3 --backend tree,robdd --samples 3 --concurrency 0
```

Concurrency is set by Threading (`0` denotes sequential execution);
warm-ups and report formatting come from Ariadne Benchmarking.
The benchmark is not a CTest case. The Benchmarking submodule is used
only by development executables and is not linked to production Reasoning.

**Backend selection:** `--backend tree,robdd` runs the same analyser
with either a BinaryTree backend or a shared-manager ROBDD backend for
every intermediate `Paving`. The ROBDD manager is new for each complete
analysis and shared by all its pavings and clones.

`CachedBinaryTreePaving` is a query-cache wrapper, not a mutable
`PavingInterface` backend. It is explicitly rejected rather than
silently substituted with BinaryTree; a genuine CachedTree analysis
requires implementing that backend's mutation contract first.

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
