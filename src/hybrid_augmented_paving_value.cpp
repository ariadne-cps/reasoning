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

#include <stdexcept>
#include <utility>

#include "geometry/box.hpp"
#include "hybrid/hybrid_auxiliary_mapping.hpp"
#include "hybrid/hybrid_paving.hpp"
#include "hybrid/hybrid_set.hpp"
#include "labelled/set.hpp"
#include "paving/binary_tree_paving.hpp"
#include "system/hybrid_automaton_interface.hpp"

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

HybridAugmentedPavingValue make_hybrid_augmented_paving_value(const Hybrid<Grid>& grids,
                                                              const HybridAutomatonInterface& system) {
    return make_hybrid_augmented_paving_value(
        grids, system, [](const Grid& grid) { return Paving(new BinaryTreePaving(grid)); });
}

HybridAugmentedPavingValue make_hybrid_augmented_paving_value(const Hybrid<Grid>& grids,
                                                              const HybridAutomatonInterface& system,
                                                              std::function<Paving(const Grid&)> backend) {
    if (not backend)
        throw std::invalid_argument("Empty paving backend factory");
    const HybridSpace space = system.state_space();
    ARIADNE_PRECONDITION(possibly(grids.space() == space));
    HybridAuxiliaryMapping mapping = make_hybrid_auxiliary_mapping(system);
    return HybridAugmentedPavingValue(
        space,
        [grids = grids, mapping = std::move(mapping), backend = std::move(backend)](
            const DiscreteLocation& location, const RealSpace& real_space) mutable {
            ARIADNE_PRECONDITION(grids.space()[location] == real_space);
            return AugmentedPaving(backend(grids[location]), mapping(location).mapping());
        });
}

HybridAugmentedPavingValue make_hybrid_augmented_paving_value(const Hybrid<Paving>& state,
                                                              const HybridAuxiliaryMapping& mapping) {
    const HybridSpace space = state.space();
    HybridAugmentedPavingValue result(
        space,
        [state = state, mapping = mapping](const DiscreteLocation& location, const RealSpace& real_space) mutable {
            ARIADNE_PRECONDITION(state.space()[location] == real_space);
            return AugmentedPaving(state[location], mapping(location).mapping());
        });
    for (auto iter = state.begin(), end = state.end(); iter != end; ++iter)
        result.insert(iter->first,
                      AugmentedPaving(state[iter->first], mapping(iter->first).mapping()));
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

Void adjoin_lower_approximation(HybridAugmentedPavingValue& pavings,
                                const EffectiveHybridOvertSetInterface& set, Nat extent, Nat fineness) {
    const Set<DiscreteLocation> locations =
        dynamic_cast<const EffectiveHybridBoundedSetInterface&>(set).locations();
    for (const DiscreteLocation& location: locations) {
        ARIADNE_PRECONDITION(pavings.has_location(location));
        const RealSpace space = pavings.space()[location];
        pavings[location].state_paving().adjoin_lower_approximation(set.euclidean_set(location, space), extent, fineness);
    }
}

Void adjoin_outer_approximation(HybridAugmentedPavingValue& pavings,
                                const EffectiveHybridCompactSetInterface& set, Nat fineness) {
    for (const DiscreteLocation& location: set.locations()) {
        ARIADNE_PRECONDITION(pavings.has_location(location));
        const RealSpace space = pavings.space()[location];
        pavings[location].state_paving().adjoin_outer_approximation(set.euclidean_set(location, space), fineness);
    }
}

Void adjoin_outer_approximation(HybridAugmentedPavingValue& pavings,
                                const HybridExactBoxes& boxes, Nat fineness) {
    for (auto iter = boxes.begin(); iter != boxes.end(); ++iter) {
        const DiscreteLocation& location = iter->first;
        const LabelledSet<ExactBoxType>& location_boxes = iter->second;
        ARIADNE_PRECONDITION(pavings.has_location(location));
        ARIADNE_PRECONDITION(location_boxes.space() == pavings.space()[location]);
        pavings[location].state_paving().adjoin_outer_approximation(ExactBoxSetType(location_boxes.euclidean_set()), fineness);
    }
}

Void restrict_to_extent(HybridAugmentedPavingValue& pavings, Nat extent) {
    for (auto iter = pavings.begin(); iter != pavings.end(); ++iter) {
        iter->second.state_paving().restrict_to_extent(extent);
    }
}

Void mince(HybridAugmentedPavingValue& pavings, Nat fineness) {
    for (auto iter = pavings.begin(); iter != pavings.end(); ++iter) {
        iter->second.mince(fineness);
    }
}

Void recombine(HybridAugmentedPavingValue& pavings) {
    for (auto iter = pavings.begin(); iter != pavings.end(); ++iter) {
        iter->second.recombine();
    }
}

Bool is_empty(const HybridAugmentedPavingValue& pavings) {
    for (auto iter = pavings.begin(); iter != pavings.end(); ++iter) {
        if (not iter->second.is_empty())
            return false;
    }
    return true;
}

Void adjoin_inner_approximation(HybridAugmentedPavingValue& pavings,
                                const EffectiveHybridSetInterface& set, Nat fineness) {
    for (const DiscreteLocation& location: set.locations()) {
        ARIADNE_PRECONDITION(pavings.has_location(location));
        const RealSpace space = pavings.space()[location];
        pavings[location].adjoin_inner_approximation(set.euclidean_set(location, space), fineness);
    }
}

} // namespace Ariadne
