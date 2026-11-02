# Ariadne Reasoning

[![macOS Status](https://github.com/ariadne-cps/reasoning/actions/workflows/macos.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/macos.yml)
[![Ubuntu Status](https://github.com/ariadne-cps/reasoning/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/ubuntu.yml)
[![Windows Status](https://github.com/ariadne-cps/reasoning/actions/workflows/win.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/win.yml)
[![Sanitizers Status](https://github.com/ariadne-cps/reasoning/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/sanitizers.yml)
[![Coverage Status](https://github.com/ariadne-cps/reasoning/actions/workflows/coverage.yml/badge.svg)](https://github.com/ariadne-cps/reasoning/actions/workflows/coverage.yml)
[![codecov](https://codecov.io/gh/ariadne-cps/reasoning/branch/main/graph/badge.svg)](https://codecov.io/gh/ariadne-cps/reasoning)

Ariadne Reasoning implements reachability analysis, safety verification and
grid-backed storage for continuous and hybrid dynamical systems.

## Layout

Public C++ headers are in `include/reasoning`, implementations in `src`,
tests in `tests`, Python bindings in `python`, and standalone tutorials in
`tutorial`. The umbrella header is `ariadne-reasoning.hpp`.

## Dependencies

Reasoning directly uses [Evolution](https://github.com/ariadne-cps/evolution),
[SMT](https://github.com/ariadne-cps/smt), and
[SAT](https://github.com/ariadne-cps/sat). The
[Configuration](https://github.com/ariadne-cps/configuration) submodule is
retained for the initial CMake bootstrap. Evolution supplies its dependencies
transitively.

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
`pyariadne`. Python smoke tests run through CTest with the `python` label.

## License

Ariadne Reasoning is distributed under the GNU General Public License v3.0.
