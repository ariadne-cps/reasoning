/***************************************************************************
 *            reasoning/hybrid_augmented_paving_value.hpp
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

/** @file reasoning/hybrid_augmented_paving_value.hpp
 *  @brief Lazy augmented pavings stored in generic location-indexed values.
 */

#ifndef ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_VALUE_HPP
#define ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_VALUE_HPP

#include <functional>

#include "hybrid/hybrid_augmented_paving.hpp"
#include "hybrid/hybrid_set_interface.hpp"
#include "hybrid/value.hpp"
#include "paving/augmented_paving.hpp"

namespace Ariadne {

class HybridAutomatonInterface;
class HybridAuxiliaryMapping;

/** @brief Adapt an automaton into a lazy, location-indexed auxiliary mapping. */
HybridAuxiliaryMapping make_hybrid_auxiliary_mapping(const HybridAutomatonInterface& system);

/** @brief Owning location-indexed family of augmented pavings. */
using HybridAugmentedPavingValue = Hybrid<AugmentedPaving>;

/** @brief Make an owning generic snapshot of legacy hybrid augmented storage.
 *  @details Every materialised location keeps its original tree or ROBDD
 *  state backend and its location-specific auxiliary mapping. The snapshot's
 *  lazy factory materialises empty slices with the same backend, so set
 *  operations never convert the authoritative representation.
 */
HybridAugmentedPavingValue make_hybrid_augmented_paving_value(const HybridAugmentedPaving& legacy);

/** @brief Make an empty hybrid augmented paving value on a generic grid family,
 *  using the automaton's auxiliary mapping and a binary-tree backend.
 */
HybridAugmentedPavingValue make_hybrid_augmented_paving_value(const Hybrid<Grid>& grids,
                                                              const HybridAutomatonInterface& system);

/** @brief Make an empty hybrid augmented paving value on a generic grid family,
 *  using the automaton's auxiliary mapping and the given backend factory.
 */
HybridAugmentedPavingValue make_hybrid_augmented_paving_value(const Hybrid<Grid>& grids,
                                                              const HybridAutomatonInterface& system,
                                                              std::function<Paving(const Grid&)> backend);

/** @brief Lift a generic hybrid state paving to augmented values with the given auxiliary mapping. */
HybridAugmentedPavingValue make_hybrid_augmented_paving_value(const Hybrid<Paving>& state,
                                                              const HybridAuxiliaryMapping& mapping);

/** @brief Union materialized slices, providing missing left-hand slices only
 *  for locations materialized on the right.
 */
Void adjoin(HybridAugmentedPavingValue& left, const HybridAugmentedPavingValue& right);

/** @brief Subtract right-hand materialized slices.
 *  @details Like legacy HybridAugmentedPaving::remove, provide a missing left
 *  slice before subtracting, retaining its empty augmented paving as
 *  materialized storage.
 */
Void remove(HybridAugmentedPavingValue& left, const HybridAugmentedPavingValue& right);

/** @brief Restrict already-materialized left slices.
 *  @details Missing right-hand slices are empty, so clear the corresponding
 *  left slice without erasing it. Right-only slices never materialize left.
 */
Void restrict(HybridAugmentedPavingValue& left, const HybridAugmentedPavingValue& right);

/** @brief Check inclusion without materializing either family.
 *  @details An absent right-hand slice is empty; an explicitly empty
 *  left-hand slice does not contribute to inclusion.
 */
Bool subset(const HybridAugmentedPavingValue& left, const HybridAugmentedPavingValue& right);

/** @brief Check set intersection without materializing either family. */
Bool intersect(const HybridAugmentedPavingValue& left, const HybridAugmentedPavingValue& right);

/** @brief Adjoin the per-location lower approximation of a hybrid overt set
 *  to each slice's state paving, as in legacy HybridAugmentedPaving.
 */
Void adjoin_lower_approximation(HybridAugmentedPavingValue& pavings,
                                const EffectiveHybridOvertSetInterface& set, Nat extent, Nat fineness);

/** @brief Adjoin the per-location outer approximation of a hybrid compact set. */
Void adjoin_outer_approximation(HybridAugmentedPavingValue& pavings,
                                const EffectiveHybridCompactSetInterface& set, Nat fineness);

/** @brief Adjoin the per-location outer approximation of hybrid exact boxes. */
Void adjoin_outer_approximation(HybridAugmentedPavingValue& pavings,
                                const HybridExactBoxes& boxes, Nat fineness);

/** @brief Restrict every materialized state paving to the given grid extent. */
Void restrict_to_extent(HybridAugmentedPavingValue& pavings, Nat extent);

/** @brief Mince every materialized slice to the given fineness. */
Void mince(HybridAugmentedPavingValue& pavings, Nat fineness);

/** @brief Recombine every materialized slice. */
Void recombine(HybridAugmentedPavingValue& pavings);

/** @brief Check whether every materialized slice is empty. */
Bool is_empty(const HybridAugmentedPavingValue& pavings);

/** @brief Adjoin the per-location inner approximation of a hybrid set. */
Void adjoin_inner_approximation(HybridAugmentedPavingValue& pavings,
                                const EffectiveHybridSetInterface& set, Nat fineness);

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_VALUE_HPP
