/***************************************************************************
 *            reasoning/hybrid_augmented_paving_value_drawing.hpp
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

/** @file reasoning/hybrid_augmented_paving_value_drawing.hpp
 *  @brief Drawing adapter for generic location-indexed hybrid augmented pavings.
 */

#ifndef ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_VALUE_DRAWING_HPP
#define ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_VALUE_DRAWING_HPP

#include "drawing/labelled_figure.hpp"
#include "paving/binary_tree_paving.hpp"
#include "reasoning/hybrid_augmented_paving_value.hpp"
#include "reasoning/paving_drawing.hpp"

namespace Ariadne::PavingDrawing {

// Generic value counterpart of the legacy HybridAugmentedPaving draw: select
// the flattened state-auxiliary pavings per location and render them with the
// ordinary euclidean paving adapter. State coordinates come first in the
// state-auxiliary space, so the axes resolve to the same projection indices
// as the legacy joined space. An empty location filter selects all
// materialised locations, as with the legacy draw.
inline Void draw(CanvasInterface& canvas, const Set<DiscreteLocation>& locations,
                 const Variables2d& axes, const HybridAugmentedPavingValue& pavings) {
    for (auto iter = pavings.begin(); iter != pavings.end(); ++iter) {
        if (not locations.empty() and not locations.contains(iter->first))
            continue;
        const RealSpace state_space = pavings.space()[iter->first];
        const BinaryTreePaving slice = iter->second.state_auxiliary_paving();
        const Projection2d projection(slice.grid().dimension(),
                                      state_space.index(axes.x()), state_space.index(axes.y()));
        draw(canvas, projection, static_cast<const SubPavingInterface&>(slice));
    }
}

} // namespace Ariadne::PavingDrawing

#endif // ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_VALUE_DRAWING_HPP
