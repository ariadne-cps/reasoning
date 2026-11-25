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
#include <utility>

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
inline Hybrid<Paving> make_hybrid_pavings(
    HybridSpace space, Hybrid<Grid> grids,
    std::function<Paving(const Grid&)> backend) {
    ARIADNE_PRECONDITION(static_cast<bool>(backend));
    return Hybrid<Paving>(std::move(space),
        [grids = std::move(grids), backend = std::move(backend)](
            const DiscreteLocation& location, const RealSpace& real_space) mutable {
            ARIADNE_PRECONDITION(grids.has_location(location));
            ARIADNE_PRECONDITION(grids.space()[location] == real_space);
            const Grid& grid = grids[location];
            Paving result = backend(grid);
            ARIADNE_PRECONDITION(result.grid() == grid);
            return result;
        });
}

/** @brief Union materialized slices, providing missing left-hand slices only
 *  for locations materialized on the right.
 */
inline Void adjoin(Hybrid<Paving>& left, const Hybrid<Paving>& right) {
    for (auto iter = right.begin(); iter != right.end(); ++iter) {
        const PavingInterface& rhs = static_cast<const PavingInterface&>(iter->second);
        left[iter->first].adjoin(rhs);
    }
}

/** @brief Subtract right-hand materialized slices.
 *  @details Like legacy HybridPaving::remove, provide a missing left slice
 *  before subtracting, retaining its empty paving as materialized storage.
 */
inline Void remove(Hybrid<Paving>& left, const Hybrid<Paving>& right) {
    for (auto iter = right.begin(); iter != right.end(); ++iter) {
        const PavingInterface& rhs = static_cast<const PavingInterface&>(iter->second);
        left[iter->first].remove(rhs);
    }
}

/** @brief Restrict already-materialized left slices.
 *  @details Missing right-hand slices are empty, so clear the corresponding
 *  left slice without erasing it. Right-only slices never materialize left.
 */
inline Void restrict(Hybrid<Paving>& left, const Hybrid<Paving>& right) {
    for (auto iter = left.begin(); iter != left.end(); ++iter) {
        if (not right.has_value(iter->first)) {
            iter->second.clear();
        } else {
            const PavingInterface& rhs =
                static_cast<const PavingInterface&>(right[iter->first]);
            iter->second.restrict(rhs);
        }
    }
}

/** @brief Check inclusion without materializing either hybrid family.
 *  @details An absent right-hand slice is empty; an explicitly empty
 *  left-hand slice does not contribute to inclusion.
 */
inline Bool subset(const Hybrid<Paving>& left, const Hybrid<Paving>& right) {
    for (auto iter = left.begin(); iter != left.end(); ++iter) {
        if (iter->second.is_empty())
            continue;
        if (not right.has_value(iter->first))
            return false;
        const PavingInterface& rhs = static_cast<const PavingInterface&>(right[iter->first]);
        if (not iter->second.subset(rhs))
            return false;
    }
    return true;
}

/** @brief Check set intersection without materializing either family. */
inline Bool intersect(const Hybrid<Paving>& left, const Hybrid<Paving>& right) {
    for (auto iter = left.begin(); iter != left.end(); ++iter) {
        if (iter->second.is_empty() || not right.has_value(iter->first))
            continue;
        const PavingInterface& rhs = static_cast<const PavingInterface&>(right[iter->first]);
        if (iter->second.intersects(rhs))
            return true;
    }
    return false;
}

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

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_PAVING_VALUE_HPP
