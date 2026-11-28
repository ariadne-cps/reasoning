/***************************************************************************
 *            reasoning/hybrid_paving_value.hpp
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

/** @file reasoning/hybrid_paving_value.hpp
 *  @brief Lazy paving backends stored in generic location-indexed values.
 */

#ifndef ARIADNE_REASONING_HYBRID_PAVING_VALUE_HPP
#define ARIADNE_REASONING_HYBRID_PAVING_VALUE_HPP

#include <functional>

#include "hybrid/hybrid_set_interface.hpp"
#include "hybrid/value.hpp"
#include "paving/paving_interface.hpp"
#include "reasoning/hybrid_grid_value.hpp"

namespace Ariadne {

/** @brief Construct a lazy family of pavings with a caller-selected backend.
 *  @details Grids and paving backends are materialized on first mutable
 *  access to a valid location. The backend factory may capture a shared
 *  RobddManager; this capture is retained by the lazy provider. The chosen
 *  backend must preserve the input grid.
 *
 *  @pre The grid family describes the same location spaces as space.
 *  @pre backend is callable.
 */
Hybrid<Paving> make_hybrid_pavings(
    HybridSpace space, Hybrid<Grid> grids,
    std::function<Paving(const Grid&)> backend);

/** @brief Union materialized slices, providing missing left-hand slices only
 *  for locations materialized on the right.
 */
Void adjoin(Hybrid<Paving>& left, const Hybrid<Paving>& right);

/** @brief Subtract right-hand materialized slices.
 *  @details Like legacy HybridPaving::remove, provide a missing left slice
 *  before subtracting, retaining its empty paving as materialized storage.
 */
Void remove(Hybrid<Paving>& left, const Hybrid<Paving>& right);

/** @brief Restrict already-materialized left slices.
 *  @details Missing right-hand slices are empty, so clear the corresponding
 *  left slice without erasing it. Right-only slices never materialize left.
 */
Void restrict(Hybrid<Paving>& left, const Hybrid<Paving>& right);

/** @brief Check inclusion without materializing either hybrid family.
 *  @details An absent right-hand slice is empty; an explicitly empty
 *  left-hand slice does not contribute to inclusion.
 */
Bool subset(const Hybrid<Paving>& left, const Hybrid<Paving>& right);

/** @brief Check set intersection without materializing either family. */
Bool intersect(const Hybrid<Paving>& left, const Hybrid<Paving>& right);

class HybridAutomatonInterface;

/** @brief Extend the materialized hybrid state pavings with auxiliary variables.
 *  @details The generic storage does not own a scaling policy or a paving
 *  backend factory: both are supplied explicitly. Preserve the selected
 *  tree/ROBDD backend and share its manager through the backend capture.
 *  Missing state locations stay unmaterialized in the result. The input grid
 *  at every materialized location must match the supplied scaling policy.
 */
Hybrid<Paving> extend_auxiliary(
    const Hybrid<Paving>& state_pavings,
    Hybrid<Scalings> scalings,
    std::function<Paving(const Grid&)> backend,
    const HybridAutomatonInterface& automaton);

/** @brief Adjoin the per-location lower approximation of a hybrid overt set.
 *  @details Mirrors legacy HybridPaving::adjoin_lower_approximation: locations
 *  of the set are materialized through the family's lazy factory.
 */
Void adjoin_lower_approximation(Hybrid<Paving>& pavings,
                                const EffectiveHybridOvertSetInterface& set, Nat extent, Nat fineness);

/** @brief Adjoin the per-location outer approximation of a hybrid compact set. */
Void adjoin_outer_approximation(Hybrid<Paving>& pavings,
                                const EffectiveHybridCompactSetInterface& set, Nat fineness);

/** @brief Adjoin the per-location outer approximation of hybrid exact boxes. */
Void adjoin_outer_approximation(Hybrid<Paving>& pavings,
                                const HybridExactBoxes& boxes, Nat fineness);

/** @brief Restrict every materialized slice to the given grid extent. */
Void restrict_to_extent(Hybrid<Paving>& pavings, Nat extent);

/** @brief Mince every materialized slice to the given fineness. */
Void mince(Hybrid<Paving>& pavings, Nat fineness);

/** @brief Recombine every materialized slice. */
Void recombine(Hybrid<Paving>& pavings);

/** @brief Check whether every materialized slice is empty. */
Bool is_empty(const Hybrid<Paving>& pavings);

/** @brief Adjoin the per-location inner approximation of a hybrid set. */
Void adjoin_inner_approximation(Hybrid<Paving>& pavings,
                                const EffectiveHybridSetInterface& set, Nat fineness);

/** @brief Inner approximation of a hybrid set on a generic grid family,
 *  with an explicit backend factory for the new slices.
 */
Hybrid<Paving> inner_approximation(const EffectiveHybridSetInterface& set,
                                   const Hybrid<Grid>& grids,
                                   std::function<Paving(const Grid&)> backend,
                                   Nat fineness);

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_PAVING_VALUE_HPP
