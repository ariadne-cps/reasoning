# Ariadne Reasoning

[![macOS Status](https://github.com/ariadne-cps/reasoning/actions/workflows/macos.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/macos.yml)
[![Ubuntu Status](https://github.com/ariadne-cps/reasoning/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/ubuntu.yml)
[![Windows Status](https://github.com/ariadne-cps/reasoning/actions/workflows/win.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/win.yml)
[![Sanitizers Status](https://github.com/ariadne-cps/reasoning/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/sanitizers.yml)
[![Coverage Status](https://github.com/ariadne-cps/reasoning/actions/workflows/coverage.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/coverage.yml)
[![codecov](https://codecov.io/gh/ariadne-cps/reasoning/branch/main/graph/badge.svg)](https://codecov.io/gh/ariadne-cps/reasoning)

Ariadne Reasoning implements reachability analysis, safety verification and
grid-backed storage for continuous and hybrid dynamical systems.
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

## Python

The standalone build composes upstream Python bindings with Reasoning in
`pyariadne_reasoning`. Python smoke tests run through CTest with the `python` label.
