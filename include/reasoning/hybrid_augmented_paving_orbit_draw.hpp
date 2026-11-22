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

// Rendering consumes Euclidean pavings after location filtering, without
// adding a hybrid-specific drawing adapter to the Drawing module.
#ifndef ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_DRAW_HPP
#define ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_DRAW_HPP

#include "reasoning/hybrid_paving_flatten.hpp"
#include "reasoning/paving_drawing.hpp"
#include "reasoning/hybrid_augmented_paving_orbit.hpp"

namespace Ariadne {
/** @brief Draw the reachable phase of a hybrid augmented paving orbit. */
inline Void draw(CanvasInterface& canvas, const Set<DiscreteLocation>& locations,
                 const Variables2d& axes, const Orbit<HybridAugmentedPaving>& orbit) {
    // The orbit owns its hybrid state; the drawing path sees only
    // location-filtered Euclidean pavings and ordinary projections.
    const HybridPaving graph = orbit.reach().state_auxiliary_paving();
    for_each_flattened_paving(graph, locations, axes,
        [&](const Projection2d& projection, const SubPavingInterface& paving) {
            PavingDrawing::draw(canvas, projection, paving);
        });
}
} // namespace Ariadne
#endif // ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_DRAW_HPP
