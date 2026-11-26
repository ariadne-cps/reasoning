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

#include "labelled/labelled_augmented_paving.hpp"

namespace Ariadne {

LabelledValue<Grid> make_labelled_grid(const Grid& grid, const RealSpace& space) {
    return LabelledValue<Grid>(grid, space);
}

LabelledValue<Paving> make_labelled_paving(Paving paving, const RealSpace& space) {
    return LabelledValue<Paving>(std::move(paving), space);
}

LabelledAugmentedPavingValue make_labelled_augmented_paving(
    AugmentedPaving paving, RealSpace state_space, RealSpace auxiliary_space) {
    ARIADNE_PRECONDITION(paving.dimension() == state_space.dimension());
    ARIADNE_PRECONDITION(paving.auxiliary_mapping().argument_size() == state_space.dimension());
    ARIADNE_PRECONDITION(paving.auxiliary_mapping().result_size() == auxiliary_space.dimension());
    return LabelledAugmentedPavingValue(
        std::move(paving), std::make_pair(std::move(state_space), std::move(auxiliary_space)));
}

LabelledAugmentedPavingValue make_labelled_augmented_paving_value(
    const LabelledAugmentedPaving& legacy) {
    return make_labelled_augmented_paving(
        static_cast<const AugmentedPaving&>(legacy),
        legacy.state_space(), legacy.auxiliary_space());
}

const RealSpace& state_space(const LabelledAugmentedPavingValue& paving) {
    return paving.labels().first;
}

const RealSpace& auxiliary_space(const LabelledAugmentedPavingValue& paving) {
    return paving.labels().second;
}

RealSpace state_auxiliary_space(const LabelledAugmentedPavingValue& paving) {
    return join(state_space(paving), auxiliary_space(paving));
}

Void adjoin(LabelledAugmentedPavingValue& left,
            const LabelledAugmentedPavingValue& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    left.value().adjoin(right.value());
}

Void restrict(LabelledAugmentedPavingValue& left,
              const LabelledAugmentedPavingValue& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    left.value().restrict(right.value());
}

Void remove(LabelledAugmentedPavingValue& left,
            const LabelledAugmentedPavingValue& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    left.value().remove(right.value());
}

Bool subset(const LabelledAugmentedPavingValue& left,
            const LabelledAugmentedPavingValue& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    return subset(left.value(), right.value());
}

Bool intersect(const LabelledAugmentedPavingValue& left,
               const LabelledAugmentedPavingValue& right) {
    ARIADNE_PRECONDITION(state_space(left) == state_space(right));
    return intersect(left.value(), right.value());
}

} // namespace Ariadne
