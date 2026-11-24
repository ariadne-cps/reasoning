/***************************************************************************
 *            reasoning/hybrid_grid_compat.hpp
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

/** @file reasoning/hybrid_grid_compat.hpp
 *  @brief Legacy hybrid grid and scaling policies adapted to generic values.
 */

#ifndef ARIADNE_REASONING_HYBRID_GRID_COMPAT_HPP
#define ARIADNE_REASONING_HYBRID_GRID_COMPAT_HPP

#include <utility>

#include "evolution/scalings.hpp"
#include "hybrid/hybrid_grid.hpp"
#include "hybrid/value.hpp"
#include "reasoning/hybrid_grid_value.hpp"

namespace Ariadne {

// Legacy HybridGrid and its scaling policy are currently paving-dependent.
// This factory preserves the policy while storing the grids in Hybrid<Grid>,
// lazily, without materialising all valid locations.
inline Hybrid<Grid> make_hybrid_grids(HybridSpace space, HybridScalings scalings) {
    return Hybrid<Grid>(std::move(space),
        [scalings = std::move(scalings)](const DiscreteLocation& location,
                                        const RealSpace& real_space) {
            return scalings.grid(location, real_space);
        });
}


// Materialize only the variables belonging to the requested location.
// A shared HybridScalings policy may depend on both the location and the
// variable; resolving it lazily preserves that behavior for Hybrid<Scalings>.
inline Hybrid<Scalings> make_hybrid_scalings(HybridSpace space, HybridScalings policy) {
    return Hybrid<Scalings>(std::move(space),
        [policy = std::move(policy)](const DiscreteLocation& location,
                                     const RealSpace& real_space) {
            Map<RealVariable, ExactDouble> entries;
            for (SizeType index = 0; index < real_space.dimension(); ++index) {
                const RealVariable& variable = real_space[index];
                entries.insert(variable, policy.scaling(location, variable));
            }
            // All variables of this location have explicit scaling values.
            // The fallback is only used for names outside its RealSpace.
            return Scalings(1.0_x, entries);
        });
}

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_GRID_COMPAT_HPP
