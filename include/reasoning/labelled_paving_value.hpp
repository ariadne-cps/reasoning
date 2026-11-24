/***************************************************************************
 *            reasoning/labelled_paving_value.hpp
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

/** @file reasoning/labelled_paving_value.hpp
 *  @brief Labelled values for grid, paving, and augmented paving geometry.
 */

#ifndef ARIADNE_REASONING_LABELLED_PAVING_VALUE_HPP
#define ARIADNE_REASONING_LABELLED_PAVING_VALUE_HPP

#include <utility>

#include "labelled/value.hpp"
#include "paving/augmented_paving.hpp"
#include "paving/grid.hpp"
#include "paving/paving_interface.hpp"

namespace Ariadne {

// Keep grid-specific composition in Reasoning, while LabelledValue<T>
// remains a geometry-independent value/metadata wrapper.
inline LabelledValue<Grid> make_labelled_grid(const Grid& grid, const RealSpace& space) {
    return LabelledValue<Grid>(grid, space);
}

// Keep backend-specific Paving creation in Reasoning; the wrapper records
// coordinate labels but never assumes tree rather than ROBDD storage.
inline LabelledValue<Paving> make_labelled_paving(Paving paving, const RealSpace& space) {
    return LabelledValue<Paving>(std::move(paving), space);
}

// The first space names state coordinates; the second names the auxiliary
// mapping's outputs. No paving-specific metadata is added to LabelledValue.
using LabelledAugmentedPavingValue =
    LabelledValue<AugmentedPaving, std::pair<RealSpace, RealSpace>>;

/** @brief Attach ordered state and auxiliary coordinates to an augmented paving.
 *  @details Preserve the existing state paving backend (tree or ROBDD).
 *  Validate both dimensions rather than silently accepting a mislabelled
 *  auxiliary mapping. This factory belongs to Reasoning, not Labelled.
 */
inline LabelledAugmentedPavingValue make_labelled_augmented_paving(
    AugmentedPaving paving, RealSpace state_space, RealSpace auxiliary_space) {
    ARIADNE_PRECONDITION(paving.dimension() == state_space.dimension());
    ARIADNE_PRECONDITION(paving.auxiliary_mapping().argument_size() == state_space.dimension());
    ARIADNE_PRECONDITION(paving.auxiliary_mapping().result_size() == auxiliary_space.dimension());
    return LabelledAugmentedPavingValue(
        std::move(paving), std::make_pair(std::move(state_space), std::move(auxiliary_space)));
}

/** @brief Access the labelled augmented paving's state coordinates. */
inline const RealSpace& state_space(const LabelledAugmentedPavingValue& paving) {
    return paving.labels().first;
}

/** @brief Access the labelled augmented paving's auxiliary coordinates. */
inline const RealSpace& auxiliary_space(const LabelledAugmentedPavingValue& paving) {
    return paving.labels().second;
}

/** @brief Return the coordinates in state-then-auxiliary order. */
inline RealSpace state_auxiliary_space(const LabelledAugmentedPavingValue& paving) {
    return join(state_space(paving), auxiliary_space(paving));
}

// Match the legacy labelled operations: compatibility is checked against the
// ordered state space; the underlying paving implements geometric operations.
// Requiring equal auxiliary spaces here would alter the old set semantics.
inline Void adjoin(LabelledAugmentedPavingValue& left,
                   const LabelledAugmentedPavingValue& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    left.value().adjoin(right.value());
}

inline Void restrict(LabelledAugmentedPavingValue& left,
                     const LabelledAugmentedPavingValue& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    left.value().restrict(right.value());
}

inline Void remove(LabelledAugmentedPavingValue& left,
                   const LabelledAugmentedPavingValue& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    left.value().remove(right.value());
}

inline Bool subset(const LabelledAugmentedPavingValue& left,
                   const LabelledAugmentedPavingValue& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    return subset(left.value(), right.value());
}

inline Bool intersect(const LabelledAugmentedPavingValue& left,
                      const LabelledAugmentedPavingValue& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    return intersect(left.value(), right.value());
}

} // namespace Ariadne

#endif // ARIADNE_REASONING_LABELLED_PAVING_VALUE_HPP
