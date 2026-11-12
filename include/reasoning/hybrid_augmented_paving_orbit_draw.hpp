/***************************************************************************
 *            reasoning/hybrid_augmented_paving_orbit_draw.hpp
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

/** @file reasoning/hybrid_augmented_paving_orbit_draw.hpp
 *  @brief Drawing adapters for hybrid augmented paving orbits.
 */

// Drawing of the reasoning-owned hybrid augmented paving orbit.
// The hybrid paving itself is rendered by hybrid/hybrid_augmented_paving_draw.hpp.
#ifndef ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_DRAW_HPP
#define ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_DRAW_HPP

#include "reasoning/hybrid_augmented_paving_orbit.hpp"
#include "hybrid/hybrid_augmented_paving_draw.hpp"

namespace Ariadne {
/** @brief Draw the reachable phase of a hybrid augmented paving orbit. */
inline Void draw(CanvasInterface& canvas, Set<DiscreteLocation> const& locations,
                 Variables2d const& axes, Orbit<HybridAugmentedPaving> const& orbit) {
    Ariadne::draw(canvas,locations,axes,orbit.reach());
}
} // namespace Ariadne
#endif // ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_DRAW_HPP
