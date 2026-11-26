/***************************************************************************
 *            reasoning/hybrid_paving.hpp
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

/** @file reasoning/hybrid_paving.hpp
 *  @brief Auxiliary-variable extension and projected views for hybrid pavings.
 */

#ifndef ARIADNE_REASONING_HYBRID_PAVING_HPP
#define ARIADNE_REASONING_HYBRID_PAVING_HPP

#include "hybrid/hybrid_augmented_paving.hpp"
#include "hybrid/hybrid_paving.hpp"
#include "labelled/figure.hpp"
#include "paving/paving_interface.hpp"
#include "reasoning/labelled_paving_value.hpp"

namespace Ariadne {

/** @brief HybridAutomatonInterface for auxiliary-variable extensions for hybrid pavings. */
class HybridAutomatonInterface;

//! Extend a hybrid state paving with the auxiliary variables of a system.
HybridPaving extend_auxiliary(const HybridPaving& hgtp,
                              const HybridAutomatonInterface& ha);

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

// Flatten location-indexed storage into ordinary continuous sets before any
// rendering. No hybrid drawing object, backend conversion or new dependency
// in the Drawing module is needed. An empty location filter selects all
// materialised locations, as with the legacy HybridPaving::draw.
template<class Visitor>
Void for_each_flattened_paving(const HybridPaving& storage,
                               const Set<DiscreteLocation>& locations,
                               const Variables2d& axes, Visitor&& visitor) {
    for (auto it = storage.locations_begin(); it != storage.locations_end(); ++it) {
        if (locations.empty() || locations.contains(it->first)) {
            const RealSpace space = storage.space(it->first);
            const Projection2d projection(
                space.dimension(), space.index(axes.x()), space.index(axes.y()));
            visitor(projection, static_cast<const SubPavingInterface&>(it->second));
        }
    }
}

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_PAVING_HPP
