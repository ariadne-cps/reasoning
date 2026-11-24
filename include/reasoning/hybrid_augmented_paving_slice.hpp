/***************************************************************************
 *            reasoning/hybrid_augmented_paving_slice.hpp
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

/** @file reasoning/hybrid_augmented_paving_slice.hpp
 *  @brief Read-only snapshots of hybrid augmented pavings as generic labelled values.
 */

#ifndef ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_SLICE_HPP
#define ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_SLICE_HPP

#include "hybrid/hybrid_augmented_paving.hpp"
#include "reasoning/labelled_augmented_paving_adapters.hpp"

namespace Ariadne {

/** @brief Snapshot one hybrid location without the legacy labelled-paving conversion.
 *  @details Retains the original tree or ROBDD state backend. The state and
 *  auxiliary coordinate spaces come from the location-specific mapping.
 *  This is an owned read-only snapshot, not the mutable hybrid element proxy.
 */
inline LabelledAugmentedPavingValue make_labelled_augmented_slice(
    const HybridAugmentedPaving& hybrid, const DiscreteLocation& location) {
    const LabelledMapping labelled_mapping = hybrid.auxiliary_mapping(location);
    return make_labelled_augmented_paving(
        AugmentedPaving(hybrid.state_paving()[location], labelled_mapping.mapping()),
        labelled_mapping.argument_space(), labelled_mapping.result_space());
}

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_SLICE_HPP
