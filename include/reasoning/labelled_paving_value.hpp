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

#include "geometry/box.hpp"
#include "geometry/set_interface.hpp"
#include "labelled/value.hpp"
#include "paving/augmented_paving.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/grid.hpp"
#include "paving/paving_interface.hpp"

namespace Ariadne {

// Keep grid-specific composition in Reasoning, while LabelledValue<T>
// remains a geometry-independent value/metadata wrapper.
LabelledValue<Grid> make_labelled_grid(const Grid& grid, const RealSpace& space);

// Keep backend-specific Paving creation in Reasoning; the wrapper records
// coordinate labels but never assumes tree rather than ROBDD storage.
LabelledValue<Paving> make_labelled_paving(Paving paving, const RealSpace& space);

// The first space names state coordinates; the second names the auxiliary
// mapping's outputs. No paving-specific metadata is added to LabelledValue.
using LabelledAugmentedPavingValue =
    LabelledValue<AugmentedPaving, std::pair<RealSpace, RealSpace>>;

/** @brief Attach ordered state and auxiliary coordinates to an augmented paving.
 *  @details Preserve the existing state paving backend (tree or ROBDD).
 *  Validate both dimensions rather than silently accepting a mislabelled
 *  auxiliary mapping. This factory belongs to Reasoning, not Labelled.
 */
LabelledAugmentedPavingValue make_labelled_augmented_paving(
    AugmentedPaving paving, RealSpace state_space, RealSpace auxiliary_space);

/** @brief Construct an empty labelled augmented paving value on a labelled grid. */
template<class SYS>
LabelledAugmentedPavingValue make_labelled_augmented_paving(const LabelledValue<Grid>& grid, const SYS& system) {
    ARIADNE_PRECONDITION(grid.space() == system.state_space());
    return make_labelled_augmented_paving(Paving(new BinaryTreePaving(grid.value())), system);
}

/** @brief Attach a system's state/auxiliary coordinates and auxiliary mapping to a state paving. */
template<class SYS>
LabelledAugmentedPavingValue make_labelled_augmented_paving(Paving paving, const SYS& system) {
    return make_labelled_augmented_paving(
        AugmentedPaving(std::move(paving), system.auxiliary_mapping()),
        system.state_space(), system.auxiliary_space());
}

/** @brief Attach a system's state/auxiliary coordinates to an augmented paving. */
template<class SYS>
LabelledAugmentedPavingValue make_labelled_augmented_paving(AugmentedPaving paving, const SYS& system) {
    return make_labelled_augmented_paving(std::move(paving), system.state_space(), system.auxiliary_space());
}

/** @brief Access the labelled augmented paving's state coordinates. */
const RealSpace& state_space(const LabelledAugmentedPavingValue& paving);

/** @brief Access the labelled augmented paving's auxiliary coordinates. */
const RealSpace& auxiliary_space(const LabelledAugmentedPavingValue& paving);

/** @brief Return the coordinates in state-then-auxiliary order. */
RealSpace state_auxiliary_space(const LabelledAugmentedPavingValue& paving);

// Match the legacy labelled operations: compatibility is checked against the
// ordered state space; the underlying paving implements geometric operations.
// Requiring equal auxiliary spaces here would alter the old set semantics.
Void adjoin(LabelledAugmentedPavingValue& left,
            const LabelledAugmentedPavingValue& right);

Void restrict(LabelledAugmentedPavingValue& left,
              const LabelledAugmentedPavingValue& right);

Void remove(LabelledAugmentedPavingValue& left,
            const LabelledAugmentedPavingValue& right);

Bool subset(const LabelledAugmentedPavingValue& left,
            const LabelledAugmentedPavingValue& right);

Bool intersect(const LabelledAugmentedPavingValue& left,
               const LabelledAugmentedPavingValue& right);

Void adjoin_outer_approximation(LabelledAugmentedPavingValue& pavings,
                                const EffectiveEuclideanCompactSetInterface& set, Nat fineness);
Void adjoin_outer_approximation(LabelledAugmentedPavingValue& pavings,
                                const ValidatedEuclideanCompactSetInterface& set, Nat fineness);
Void adjoin_outer_approximation(LabelledAugmentedPavingValue& pavings,
                                const ExactBoxType& box, Nat fineness);
Void adjoin_lower_approximation(LabelledAugmentedPavingValue& pavings,
                                const EffectiveEuclideanOvertSetInterface& set, Nat extent, Nat fineness);
Void adjoin_inner_approximation(LabelledAugmentedPavingValue& pavings,
                                const EffectiveEuclideanSetInterface& set, Nat fineness);
Void restrict_to_extent(LabelledAugmentedPavingValue& pavings, Nat extent);
Void mince(LabelledAugmentedPavingValue& pavings, Nat fineness);
Void recombine(LabelledAugmentedPavingValue& pavings);
Bool is_empty(const LabelledAugmentedPavingValue& pavings);

Paving inner_approximation(const EffectiveEuclideanSetInterface& set,
                           const LabelledValue<Grid>& grid, Nat fineness);

} // namespace Ariadne

#endif // ARIADNE_REASONING_LABELLED_PAVING_VALUE_HPP
