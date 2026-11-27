/***************************************************************************
 *            tests/hybrid_legacy_policy_test_adapter.hpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

#ifndef ARIADNE_REASONING_TESTS_HYBRID_LEGACY_POLICY_TEST_ADAPTER_HPP
#define ARIADNE_REASONING_TESTS_HYBRID_LEGACY_POLICY_TEST_ADAPTER_HPP

#include "evolution/scalings.hpp"
#include "hybrid/hybrid_scalings.hpp"
#include "hybrid/value.hpp"
#include "paving/grid.hpp"
#include "reasoning/hybrid_grid_compat.hpp"
#include "reasoning/hybrid_grid_value.hpp"

namespace Ariadne {

// Test-local old-to-new adapters for parity checks against the legacy
// HybridScalings policy; the library no longer ships these bridges.
inline Hybrid<Scalings> test_make_hybrid_scalings(HybridSpace space, HybridScalings policy) {
    return Hybrid<Scalings>(std::move(space),
                            [policy = std::move(policy)](const DiscreteLocation& location,
                                                         const RealSpace& real_space) {
                                Map<RealVariable, ExactDouble> entries;
                                for (SizeType index = 0; index < real_space.dimension(); ++index) {
                                    const RealVariable& variable = real_space[index];
                                    entries.insert(variable, policy.scaling(location, variable));
                                }
                                return Scalings(1.0_x, entries);
                            });
}

inline Hybrid<Grid> test_make_hybrid_grids(HybridSpace space, HybridScalings scalings) {
    return Hybrid<Grid>(std::move(space),
                        [scalings = std::move(scalings)](const DiscreteLocation& location,
                                                         const RealSpace& real_space) {
                            return grid_from_scalings(scalings, location, real_space);
                        });
}

} // namespace Ariadne

#endif // ARIADNE_REASONING_TESTS_HYBRID_LEGACY_POLICY_TEST_ADAPTER_HPP
