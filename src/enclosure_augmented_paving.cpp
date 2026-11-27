/***************************************************************************
 *            enclosure_augmented_paving.cpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne Reasoning.
 *
 *  Ariadne Reasoning is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne Reasoning is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne Reasoning. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file src/enclosure_augmented_paving.cpp
 *  @brief Implement rendering-independent outer-paving adapters for enclosures.
 */

// Adapt evolved enclosures into drawing-independent augmented pavings.
#include "reasoning/enclosure_augmented_paving.hpp"

#include "evolution/hybrid_enclosure.hpp"
#include "function/constraint.hpp"
#include "function/functional.hpp"
#include "labelled/figure.hpp"
#include "logging/logging.hpp"
#include "paving/binary_tree_paving.hpp"
#include "set/function_set.hpp"
#include "symbolic/space.hpp"

namespace Ariadne {

Void adjoin_outer_approximation_to(const Enclosure& enclosure, AugmentedPaving& paving, Nat fineness, const Paver& paver) {
    if (enclosure.auxiliary_mapping().result_size() != 0 &&
        enclosure.auxiliary_mapping().managed_pointer() == paving.auxiliary_mapping().managed_pointer()) {
        ARIADNE_WARN("enclosure=" << enclosure << ", "
                                                  "enclosure.auxiliary_mapping()="
                                  << enclosure.auxiliary_mapping() << ", "
                                                                      "paving.auxiliary_mapping()="
                                  << paving.auxiliary_mapping());
    }
    paver.adjoin_outer_approximation(paving.state_paving(), enclosure.state_set(), fineness);
}

AugmentedPaving outer_approximation(const Enclosure& enclosure, const Grid& grid, Nat fineness, const Paver& paver) {
    AugmentedPaving result(grid, enclosure.auxiliary_mapping());
    adjoin_outer_approximation_to(enclosure, result, fineness, paver);
    return result;
}

Void adjoin_outer_approximation_to(const HybridEnclosure& enclosure, Hybrid<Paving>& pavings, Nat fineness, const Paver& paver) {
    const DiscreteLocation location = enclosure.location();
    const Enclosure& set = enclosure.continuous_set();
    ARIADNE_PRECONDITION_MSG(enclosure.state_space() == pavings.space()[location],
                             "HybridEnclosure's state variables " << enclosure.state_space() << " do not match variables " << pavings.space()[location] << " of the hybrid paving family in location " << location);
    paver.adjoin_outer_approximation(pavings[location], set.state_set(), fineness);
}

Void adjoin_outer_approximation_to(const HybridEnclosure& enclosure, HybridAugmentedPavingValue& pavings, Nat fineness, const Paver& paver) {
    const DiscreteLocation location = enclosure.location();
    const Enclosure& set = enclosure.continuous_set();
    ARIADNE_PRECONDITION_MSG(enclosure.state_space() == pavings.space()[location],
                             "HybridEnclosure's state variables " << enclosure.state_space() << " do not match variables " << pavings.space()[location] << " of the hybrid augmented paving family in location " << location);
    paver.adjoin_outer_approximation(pavings[location].state_paving(), set.state_set(), fineness);
}

} // namespace Ariadne
