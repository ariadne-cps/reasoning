/***************************************************************************
 *            src/hybrid_paving_value_operations.cpp
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

/** @file src/hybrid_paving_value_operations.cpp
 *  @brief Extend generic lazy hybrid pavings with system auxiliary coordinates.
 */

#include "reasoning/hybrid_paving_value_operations.hpp"

#include "function/functional.hpp"
#include "paving/binary_tree_paving.hpp"
#include "system/hybrid_automaton_interface.hpp"

namespace Ariadne {

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

        // Match the legacy geometric skew product without converting the
        // authoritative tree/ROBDD input backend.
        BinaryTreePaving tree(state_paving.grid());
        for (auto cell = state_paving.begin(), end = state_paving.end(); cell != end; ++cell)
            tree.adjoin(*cell);
        const BinaryTreePaving product =
            outer_skew_product(tree, auxiliary_grid, auxiliary_function);

        Paving& destination = result[location];
        for (const auto& cell : product)
            destination.adjoin(cell);
    }
    return result;
}

} // namespace Ariadne
