/***************************************************************************
 *            reasoning/hybrid_paving_drawing.hpp
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

/** @file reasoning/hybrid_paving_drawing.hpp
 *  @brief Location selection and flattening before paving rendering.
 */

#ifndef ARIADNE_REASONING_HYBRID_PAVING_DRAWING_HPP
#define ARIADNE_REASONING_HYBRID_PAVING_DRAWING_HPP

#include "hybrid/hybrid_augmented_paving.hpp"
#include "reasoning/hybrid_paving_flatten.hpp"
#include "reasoning/paving_drawing.hpp"

namespace Ariadne::PavingDrawing {

// The actual drawing receives only Euclidean subpavings. The location
// filter and per-location coordinate projection belong to Reasoning.
inline Void draw(CanvasInterface& canvas, const Set<DiscreteLocation>& locations,
                 const Variables2d& axes, const HybridPaving& paving) {
    for_each_flattened_paving(paving, locations, axes,
        [&](const Projection2d& projection, const SubPavingInterface& euclidean) {
            PavingDrawing::draw(canvas, projection, euclidean);
        });
}

// The derived auxiliary graph is rendered, as in the legacy adapter;
// authoritative state paving and backend are never modified.
inline Void draw(CanvasInterface& canvas, const Set<DiscreteLocation>& locations,
                 const Variables2d& axes, const HybridAugmentedPaving& paving) {
    const HybridPaving graph = paving.state_auxiliary_paving();
    PavingDrawing::draw(canvas, locations, axes, graph);
}

} // namespace Ariadne::PavingDrawing

#endif // ARIADNE_REASONING_HYBRID_PAVING_DRAWING_HPP
