/***************************************************************************
 *            src/labelled_paving_value.cpp
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

/** @file src/labelled_paving_value.cpp
 *  @brief Compose and manipulate labelled paving values.
 */

#include "reasoning/labelled_paving_value.hpp"

#include "paving/binary_tree_paving.hpp"

namespace Ariadne {

LabelledValue<Grid> make_labelled_grid(const Grid& grid, const RealSpace& space) {
    return LabelledValue<Grid>(grid, space);
}

LabelledValue<Paving> make_labelled_paving(Paving paving, const RealSpace& space) {
    return LabelledValue<Paving>(std::move(paving), space);
}

LabelledAugmentedPaving make_labelled_augmented_paving(
    AugmentedPaving paving, RealSpace state_space, RealSpace auxiliary_space) {
    ARIADNE_PRECONDITION(paving.dimension() == state_space.dimension());
    ARIADNE_PRECONDITION(paving.auxiliary_mapping().argument_size() == state_space.dimension());
    ARIADNE_PRECONDITION(paving.auxiliary_mapping().result_size() == auxiliary_space.dimension());
    return LabelledAugmentedPaving(
        std::move(paving), std::make_pair(std::move(state_space), std::move(auxiliary_space)));
}

const RealSpace& state_space(const LabelledAugmentedPaving& paving) {
    return paving.labels().first;
}

const RealSpace& auxiliary_space(const LabelledAugmentedPaving& paving) {
    return paving.labels().second;
}

RealSpace state_auxiliary_space(const LabelledAugmentedPaving& paving) {
    return join(state_space(paving), auxiliary_space(paving));
}

Void adjoin(LabelledAugmentedPaving& left,
            const LabelledAugmentedPaving& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    left.value().adjoin(right.value());
}

Void restrict(LabelledAugmentedPaving& left,
              const LabelledAugmentedPaving& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    left.value().restrict(right.value());
}

Void remove(LabelledAugmentedPaving& left,
            const LabelledAugmentedPaving& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    left.value().remove(right.value());
}

Bool subset(const LabelledAugmentedPaving& left,
            const LabelledAugmentedPaving& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    return subset(left.value(), right.value());
}

Bool intersect(const LabelledAugmentedPaving& left,
               const LabelledAugmentedPaving& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    return intersect(left.value(), right.value());
}

Void adjoin_outer_approximation(LabelledAugmentedPaving& pavings,
                                const EffectiveEuclideanCompactSetInterface& set, Nat fineness) {
    pavings.value().adjoin_outer_approximation(set, fineness);
}

Void adjoin_outer_approximation(LabelledAugmentedPaving& pavings,
                                const ValidatedEuclideanCompactSetInterface& set, Nat fineness) {
    pavings.value().adjoin_outer_approximation(set, fineness);
}

Void adjoin_outer_approximation(LabelledAugmentedPaving& pavings,
                                const ExactBoxType& box, Nat fineness) {
    pavings.value().adjoin_outer_approximation(box, fineness);
}

Void adjoin_lower_approximation(LabelledAugmentedPaving& pavings,
                                const EffectiveEuclideanOvertSetInterface& set, Nat extent, Nat fineness) {
    pavings.value().adjoin_lower_approximation(set, extent, fineness);
}

Void adjoin_inner_approximation(LabelledAugmentedPaving& pavings,
                                const EffectiveEuclideanSetInterface& set, Nat fineness) {
    pavings.value().adjoin_inner_approximation(set, fineness);
}

Void restrict_to_extent(LabelledAugmentedPaving& pavings, Nat extent) {
    pavings.value().restrict_to_extent(extent);
}

Void mince(LabelledAugmentedPaving& pavings, Nat fineness) {
    pavings.value().mince(fineness);
}

Void recombine(LabelledAugmentedPaving& pavings) {
    pavings.value().recombine();
}

Bool is_empty(const LabelledAugmentedPaving& pavings) {
    return pavings.value().is_empty();
}

Paving inner_approximation(const EffectiveEuclideanSetInterface& set,
                           const LabelledValue<Grid>& grid, Nat fineness) {
    Paving paving(new BinaryTreePaving(grid.value()));
    paving.adjoin_inner_approximation(set, fineness);
    return paving;
}

} // namespace Ariadne
