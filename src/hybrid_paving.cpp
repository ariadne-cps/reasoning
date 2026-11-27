/***************************************************************************
 *            src/hybrid_paving.cpp
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

/** @file src/hybrid_paving.cpp
 *  @brief Implement hybrid paving extensions.
 */

// Hybrid paving operations depending on a system specification.

#include <utility>

#include "function/functional.hpp"
#include "geometry/box.hpp"
#include "hybrid/hybrid_set.hpp"
#include "labelled/set.hpp"
#include "paving/binary_tree_paving.hpp"
#include "reasoning/hybrid_grid_compat.hpp"
#include "reasoning/hybrid_paving_value.hpp"
#include "system/hybrid_automaton_interface.hpp"

namespace Ariadne {

namespace {
// Use the same backend-neutral geometric kernel for legacy HybridPaving
// and generic Hybrid<Paving>. The binary tree is a derived representation;
// only the destination's selected tree/ROBDD backend remains authoritative.
Void adjoin_auxiliary_skew_product(
    Paving& destination, const Paving& source,
    const Grid& auxiliary_grid,
    const EffectiveVectorMultivariateFunction& auxiliary_function) {
    BinaryTreePaving tree(source.grid());
    for (auto cell = source.begin(), end = source.end(); cell != end; ++cell)
        tree.adjoin(*cell);
    const BinaryTreePaving product =
        outer_skew_product(tree, auxiliary_grid, auxiliary_function);
    for (const auto& cell: product)
        destination.adjoin(cell);
}
} // namespace

Hybrid<Paving> make_hybrid_pavings(
    HybridSpace space, Hybrid<Grid> grids,
    std::function<Paving(const Grid&)> backend) {
    ARIADNE_PRECONDITION(static_cast<bool>(backend));
    return Hybrid<Paving>(std::move(space),
                          [grids = std::move(grids), backend = std::move(backend)](
                              const DiscreteLocation& location, const RealSpace& real_space) mutable {
                              ARIADNE_PRECONDITION(grids.has_location(location));
                              ARIADNE_PRECONDITION(grids.space()[location] == real_space);
                              const Grid& grid = grids[location];
                              Paving result = backend(grid);
                              ARIADNE_PRECONDITION(result.grid() == grid);
                              return result;
                          });
}

Void adjoin(Hybrid<Paving>& left, const Hybrid<Paving>& right) {
    for (auto iter = right.begin(); iter != right.end(); ++iter) {
        const PavingInterface& rhs = static_cast<const PavingInterface&>(iter->second);
        left[iter->first].adjoin(rhs);
    }
}

Void remove(Hybrid<Paving>& left, const Hybrid<Paving>& right) {
    for (auto iter = right.begin(); iter != right.end(); ++iter) {
        const PavingInterface& rhs = static_cast<const PavingInterface&>(iter->second);
        left[iter->first].remove(rhs);
    }
}

Void restrict(Hybrid<Paving>& left, const Hybrid<Paving>& right) {
    for (auto iter = left.begin(); iter != left.end(); ++iter) {
        if (not right.has_value(iter->first)) {
            iter->second.clear();
        } else {
            const PavingInterface& rhs =
                static_cast<const PavingInterface&>(right[iter->first]);
            iter->second.restrict(rhs);
        }
    }
}

Bool subset(const Hybrid<Paving>& left, const Hybrid<Paving>& right) {
    for (auto iter = left.begin(); iter != left.end(); ++iter) {
        if (iter->second.is_empty())
            continue;
        if (not right.has_value(iter->first))
            return false;
        const PavingInterface& rhs = static_cast<const PavingInterface&>(right[iter->first]);
        if (not iter->second.subset(rhs))
            return false;
    }
    return true;
}

Bool intersect(const Hybrid<Paving>& left, const Hybrid<Paving>& right) {
    for (auto iter = left.begin(); iter != left.end(); ++iter) {
        if (iter->second.is_empty() || not right.has_value(iter->first))
            continue;
        const PavingInterface& rhs = static_cast<const PavingInterface&>(right[iter->first]);
        if (iter->second.intersects(rhs))
            return true;
    }
    return false;
}

Void adjoin_lower_approximation(Hybrid<Paving>& pavings,
                                const EffectiveHybridOvertSetInterface& set, Nat extent, Nat fineness) {
    const Set<DiscreteLocation> locations =
        dynamic_cast<const EffectiveHybridBoundedSetInterface&>(set).locations();
    for (const DiscreteLocation& location: locations) {
        ARIADNE_PRECONDITION(pavings.has_location(location));
        const RealSpace space = pavings.space()[location];
        pavings[location].adjoin_lower_approximation(set.euclidean_set(location, space), extent, fineness);
    }
}

Void adjoin_outer_approximation(Hybrid<Paving>& pavings,
                                const EffectiveHybridCompactSetInterface& set, Nat fineness) {
    for (const DiscreteLocation& location: set.locations()) {
        ARIADNE_PRECONDITION(pavings.has_location(location));
        const RealSpace space = pavings.space()[location];
        pavings[location].adjoin_outer_approximation(set.euclidean_set(location, space), fineness);
    }
}

Void adjoin_outer_approximation(Hybrid<Paving>& pavings,
                                const HybridExactBoxes& boxes, Nat fineness) {
    for (auto iter = boxes.begin(); iter != boxes.end(); ++iter) {
        const DiscreteLocation& location = iter->first;
        const LabelledSet<ExactBoxType>& location_boxes = iter->second;
        ARIADNE_PRECONDITION(pavings.has_location(location));
        ARIADNE_PRECONDITION(location_boxes.space() == pavings.space()[location]);
        pavings[location].adjoin_outer_approximation(ExactBoxSetType(location_boxes.euclidean_set()), fineness);
    }
}

Void restrict_to_extent(Hybrid<Paving>& pavings, Nat extent) {
    for (auto iter = pavings.begin(); iter != pavings.end(); ++iter) {
        iter->second.restrict_to_extent(extent);
    }
}

Void mince(Hybrid<Paving>& pavings, Nat fineness) {
    for (auto iter = pavings.begin(); iter != pavings.end(); ++iter) {
        iter->second.mince(fineness);
    }
}

Void recombine(Hybrid<Paving>& pavings) {
    for (auto iter = pavings.begin(); iter != pavings.end(); ++iter) {
        iter->second.recombine();
    }
}

Bool is_empty(const Hybrid<Paving>& pavings) {
    for (auto iter = pavings.begin(); iter != pavings.end(); ++iter) {
        if (not iter->second.is_empty())
            return false;
    }
    return true;
}

Void adjoin_inner_approximation(Hybrid<Paving>& pavings,
                                const EffectiveHybridSetInterface& set, Nat fineness) {
    for (const DiscreteLocation& location: set.locations()) {
        ARIADNE_PRECONDITION(pavings.has_location(location));
        const RealSpace space = pavings.space()[location];
        pavings[location].adjoin_inner_approximation(set.euclidean_set(location, space), fineness);
    }
}

Hybrid<Paving> inner_approximation(const EffectiveHybridSetInterface& set,
                                   const Hybrid<Grid>& grids,
                                   std::function<Paving(const Grid&)> backend,
                                   Nat fineness) {
    Hybrid<Paving> result = make_hybrid_pavings(grids.space(), grids, std::move(backend));
    adjoin_inner_approximation(result, set, fineness);
    return result;
}

Hybrid<Paving> extend_auxiliary(
    const Hybrid<Paving>& state_pavings,
    Hybrid<Scalings> scalings,
    std::function<Paving(const Grid&)> backend,
    const HybridAutomatonInterface& automaton) {
    // Keep a copy for validating the input's location-dependent grid.
    Hybrid<Scalings> state_scalings(scalings);
    const HybridSpace expanded_space = automaton.state_auxiliary_space();
    Hybrid<Paving> result = make_hybrid_pavings(
        expanded_space,
        make_hybrid_grids(expanded_space, std::move(scalings)),
        std::move(backend));

    // Materialized input locations are the only locations to transfer.
    // Neither the input nor the output materializes any other location.
    for (auto iter = state_pavings.begin(); iter != state_pavings.end(); ++iter) {
        const DiscreteLocation& location = iter->first;
        const Paving& state_paving = iter->second;
        ARIADNE_PRECONDITION(expanded_space.has_location(location));
        ARIADNE_PRECONDITION(state_pavings.space()[location] ==
                             automaton.continuous_state_space(location));
        ARIADNE_PRECONDITION(state_scalings.has_location(location));
        const Grid expected_state_grid =
            grid_from_scalings(state_scalings[location],
                               automaton.continuous_state_space(location));
        ARIADNE_PRECONDITION(state_paving.grid() == expected_state_grid);

        const RealSpace auxiliary_space = automaton.continuous_auxiliary_space(location);
        const EffectiveVectorMultivariateFunction auxiliary_function =
            automaton.auxiliary_function(location);
        const Grid auxiliary_grid =
            grid_from_scalings(state_scalings[location], auxiliary_space);

        Paving& destination = result[location];
        adjoin_auxiliary_skew_product(
            destination, state_paving, auxiliary_grid, auxiliary_function);
    }
    return result;
}

} // namespace Ariadne
