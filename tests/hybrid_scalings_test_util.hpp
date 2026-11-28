/***************************************************************************
 *            tests/hybrid_scalings_test_util.hpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

#ifndef ARIADNE_REASONING_TESTS_HYBRID_SCALINGS_TEST_UTIL_HPP
#define ARIADNE_REASONING_TESTS_HYBRID_SCALINGS_TEST_UTIL_HPP

#include <utility>

#include "evolution/scalings.hpp"
#include "hybrid/value.hpp"

namespace Ariadne {

/** @brief Build a lazy Hybrid<Scalings> from a named scaling map and a default
 *  scaling for unlisted variables, without any legacy HybridScalings policy.
 */
inline Hybrid<Scalings> test_scalings_from_map(HybridSpace space, ExactDouble default_scaling,
                                               Map<Identifier, ExactDouble> named) {
    return Hybrid<Scalings>(std::move(space),
                            [default_scaling, named = std::move(named)](const DiscreteLocation&,
                                                                        const RealSpace& real_space) {
                                Map<RealVariable, ExactDouble> entries;
                                for (SizeType index = 0; index < real_space.dimension(); ++index) {
                                    const RealVariable& variable = real_space[index];
                                    if (named.has_key(variable.name()))
                                        entries.insert(variable, named.at(variable.name()));
                                }
                                return Scalings(default_scaling, entries);
                            });
}

} // namespace Ariadne

#endif // ARIADNE_REASONING_TESTS_HYBRID_SCALINGS_TEST_UTIL_HPP
