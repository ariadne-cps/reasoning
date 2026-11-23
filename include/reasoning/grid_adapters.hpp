/***************************************************************************
 *            reasoning/grid_adapters.hpp
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

/** @file reasoning/grid_adapters.hpp
 *  @brief Paving-specific factories over generic labelled and hybrid values.
 */

#ifndef ARIADNE_REASONING_GRID_ADAPTERS_HPP
#define ARIADNE_REASONING_GRID_ADAPTERS_HPP

#include "evolution/scalings.hpp"
#include "hybrid/hybrid_grid.hpp"
#include "hybrid/value.hpp"
#include "labelled/value.hpp"
#include "paving/grid.hpp"

namespace Ariadne {

// Keep grid-specific composition in Reasoning, while LabelledValue<T>
// remains a geometry-independent value/metadata wrapper.
inline LabelledValue<Grid> make_labelled_grid(const Grid& grid, const RealSpace& space) {
    return LabelledValue<Grid>(grid, space);
}

// Materialize a paving Grid only in Reasoning: Scalings owns the numeric
// policy and ordering, while the paving-specific representation stays here.
inline Grid grid_from_scalings(const Scalings& scalings, const RealSpace& space) {
    Vector<ExactDouble> lengths(space.dimension(), 1.0_x);
    for (SizeType axis = 0; axis < space.dimension(); ++axis) {
        lengths[axis] = scalings.scaling(space[axis]);
    }
    return Grid(lengths);
}

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

} // namespace Ariadne

#endif // ARIADNE_REASONING_GRID_ADAPTERS_HPP
