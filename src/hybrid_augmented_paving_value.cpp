/***************************************************************************
 *            src/hybrid_augmented_paving_value.cpp
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne Reasoning.  If not, see <https://www.gnu.org/licenses/>.
 */

/** @file src/hybrid_augmented_paving_value.cpp
 *  @brief Generic location-indexed snapshots of legacy hybrid augmented pavings.
 */

#include "reasoning/hybrid_augmented_paving_value.hpp"

#include <utility>

#include "hybrid/hybrid_auxiliary_mapping.hpp"
#include "hybrid/hybrid_paving.hpp"

namespace Ariadne {

HybridAugmentedPavingValue make_hybrid_augmented_paving_value(const HybridAugmentedPaving& legacy) {
    const HybridSpace space = legacy.state_paving().space();
    HybridPaving state = legacy.state_paving();
    HybridAuxiliaryMapping mapping = legacy.auxiliary_data();
    HybridAugmentedPavingValue result(
        space,
        [state = std::move(state), mapping = std::move(mapping)](
            const DiscreteLocation& location, const RealSpace& real_space) mutable {
            ARIADNE_PRECONDITION(state.space()[location] == real_space);
            return AugmentedPaving(state[location], mapping(location).mapping());
        });
    for (auto iter = legacy.state_paving().locations_begin(),
              end = legacy.state_paving().locations_end();
         iter != end; ++iter) {
        result.insert(iter->first,
                      AugmentedPaving(legacy.state_paving()[iter->first],
                                       legacy.auxiliary_mapping(iter->first).mapping()));
    }
    return result;
}

Void adjoin(HybridAugmentedPavingValue& left, const HybridAugmentedPavingValue& right) {
    for (auto iter = right.begin(); iter != right.end(); ++iter) {
        left[iter->first].adjoin(iter->second);
    }
}

Void remove(HybridAugmentedPavingValue& left, const HybridAugmentedPavingValue& right) {
    for (auto iter = right.begin(); iter != right.end(); ++iter) {
        left[iter->first].remove(iter->second);
    }
}

Void restrict(HybridAugmentedPavingValue& left, const HybridAugmentedPavingValue& right) {
    for (auto iter = left.begin(); iter != left.end(); ++iter) {
        if (not right.has_value(iter->first)) {
            iter->second.clear();
        } else {
            iter->second.restrict(right[iter->first]);
        }
    }
}

Bool subset(const HybridAugmentedPavingValue& left, const HybridAugmentedPavingValue& right) {
    for (auto iter = left.begin(); iter != left.end(); ++iter) {
        if (iter->second.is_empty())
            continue;
        if (not right.has_value(iter->first))
            return false;
        if (not subset(iter->second, right[iter->first]))
            return false;
    }
    return true;
}

Bool intersect(const HybridAugmentedPavingValue& left, const HybridAugmentedPavingValue& right) {
    for (auto iter = left.begin(); iter != left.end(); ++iter) {
        if (iter->second.is_empty() || not right.has_value(iter->first))
            continue;
        if (intersect(iter->second, right[iter->first]))
            return true;
    }
    return false;
}

} // namespace Ariadne
